/**
  ******************************************************************************
  * @file    usbd_hid_composite.c
  * @author  MCD Application Team
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *
  *          ===================================================================
  *                                HID Class  Description
  *          ===================================================================
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifdef USBCON
#ifdef USBD_USE_HID_COMPOSITE_JOYSTICK

/* Includes ------------------------------------------------------------------*/
#include "usbd_hid_composite_joy.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_HID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_HID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_HID_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_HID_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_HID_Private_FunctionPrototypes
  * @{
  */

static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_COMPOSITE_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
// static uint8_t USBD_HID_MOUSE_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
// static uint8_t USBD_HID_KEYBOARD_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_HID_JOYSTICK_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);

static uint8_t *USBD_HID_GetFSCfgDesc(uint16_t *length);
// static uint8_t *USBD_HID_GetDeviceQualifierDesc(uint16_t *length);
static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);

/**
  * @}
  */

/** @defgroup USBD_HID_Private_Variables
  * @{
  */

USBD_ClassTypeDef  USBD_COMPOSITE_HID = {
  USBD_HID_Init,
  USBD_HID_DeInit,
  USBD_COMPOSITE_HID_Setup,
  NULL,              /* EP0_TxSent */
  NULL,              /* EP0_RxReady */
  USBD_HID_DataIn,   /* DataIn */
  NULL,              /* DataOut */
  NULL,              /* SOF */
  NULL,
  NULL,
  NULL,//USBD_HID_GetHSCfgDesc,
  USBD_HID_GetFSCfgDesc,
  NULL,//USBD_HID_GetOtherSpeedCfgDesc,
  NULL,//USBD_HID_GetDeviceQualifierDesc,
};

/* USB HID device FS Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_HID_CfgFSDesc[USB_COMPOSITE_HID_CONFIG_DESC_SIZ]  __ALIGN_END = {
  0x09,                                               /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                        /* bDescriptorType: Configuration */
  LOBYTE(USB_COMPOSITE_HID_CONFIG_DESC_SIZ),          /* wTotalLength: Bytes returned */
  HIBYTE(USB_COMPOSITE_HID_CONFIG_DESC_SIZ),
  0x01,                                               /* bNumInterfaces: 1 interface */
  0x01,                                               /* bConfigurationValue: Configuration value */
  0x00,                                               /* iConfiguration: Index of string descriptor describing the configuration */
  0x80,                                               /* bmAttributes: Bus Powered according to user configuration */
  USBD_MAX_POWER,                                     /* MaxPower (mA) */

  /************** Descriptor of Joystick interface ****************/
  /* 09 */
  0x09,                                               /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                            /* bDescriptorType: Interface descriptor type */
  HID_JOYSTICK_INTERFACE,                             /* bInterfaceNumber: Number of Interface */
  0x00,                                               /* bAlternateSetting: Alternate setting */
  0x01,                                               /* bNumEndpoints */
  0x03,                                               /* bInterfaceClass: HID */
  0x00,                                               /* bInterfaceSubClass : 1=BOOT, 0=no boot */
  0x00,                                               /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
  0x00,                                               /* iInterface: Index of string descriptor */
  /******************** Descriptor of Joystick HID ********************/
  /* 18 */
  0x09,                                               /* bLength: HID Descriptor size */
  HID_DESCRIPTOR_TYPE,                                /* bDescriptorType: HID */
  0x11,                                               /* bcdHID: HID Class Spec release number */
  0x01,
  0x00,                                               /* bCountryCode: Hardware target country */
  0x01,                                               /* bNumDescriptors: Number of HID class descriptors to follow */
  0x22,                                               /* bDescriptorType */
  HID_JOYSTICK_REPORT_DESC_SIZE,                      /* wItemLength: Total length of Report descriptor */
  0x00,
  /******************** Descriptor of Joystick endpoint ********************/
  /* 27 */
  0x07,                                               /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                             /* bDescriptorType: */
  HID_JOYSTICK_EPIN_ADDR,                                /* bEndpointAddress: Endpoint Address (IN) */
  0x03,                                               /* bmAttributes: Interrupt endpoint */
  HID_JOYSTICK_EPIN_SIZE,                                /* wMaxPacketSize: 64 Bytes max */
  0x00,
  HID_FS_BINTERVAL,                                   /* bInterval: Polling Interval*/
} ;

/* USB HID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_JOYSTICK_HID_Desc[USB_HID_DESC_SIZ]  __ALIGN_END  = {
  0x09,                                               /* bLength: HID Descriptor size */
  HID_DESCRIPTOR_TYPE,                                /* bDescriptorType: HID */
  0x11,                                               /* bcdHID: HID Class Spec release number */
  0x01,
  0x00,                                               /* bCountryCode: Hardware target country */
  0x01,                                               /* bNumDescriptors: Number of HID class descriptors to follow */
  0x22,                                               /* bDescriptorType */
  HID_JOYSTICK_REPORT_DESC_SIZE,                         /* wItemLength: Total length of Report descriptor */
  0x00
};

__ALIGN_BEGIN static uint8_t HID_JOYSTICK_ReportDesc[HID_JOYSTICK_REPORT_DESC_SIZE] __ALIGN_END =  {                    
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
		0x85, 0x01,				 	           //		REPORT_ID	(1)	
		// buttons data
		0x05, 0x09,                    //   USAGE_PAGE (Button)		
    0x19, 0x01,                    //   USAGE_MINIMUM (Button 1)
    0x29, MAX_BUTTONS_NUM,         //   USAGE_MAXIMUM (Button MAX_BUTTONS_NUM)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, MAX_BUTTONS_NUM,         //   REPORT_COUNT (MAX_BUTTONS_NUM)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)

		// axis data
		0x05, 0x01,                    // 	USAGE_PAGE (Generic Desktop)
		0x09, 0x30,                    //   USAGE (X)
    0x09, 0x31,                    //   USAGE (Y)
    0x09, 0x32,                    //   USAGE (Z)
    0x09, 0x33,                    //   USAGE (Rx)
    0x09, 0x34,                    //   USAGE (Ry)
    0x09, 0x35,                    //   USAGE (Rz)
		0x09, 0x36,                    //   USAGE (Slider)
		0x09, 0x36,                    //  	USAGE (Slider)
    0x16, 0x01, 0x80,              //  	LOGICAL_MINIMUM (-32767)
    0x26, 0xFF, 0x7F,						   //   LOGICAL_MAXIMUM (32767)
    0x75, 0x10,                    //   REPORT_SIZE (16)
    0x95, MAX_AXIS_NUM,       		 //   REPORT_COUNT (MAX_AXIS_NUM)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
		
		// POV data
		0x09, 0x39, 									 //   USAGE (Hat switch)
		0x15, 0x00, 									 //   LOGICAL_MINIMUM (0)
		0x25, 0x07, 									 //   LOGICAL_MAXIMUM (7)
		0x35, 0x00, 									 //   PHYSICAL_MINIMUM (0)
		0x46, 0x3B, 0x01,							 //   PHYSICAL_MAXIMUM (315)
		0x65, 0x12, 									 //   UNIT (SI Rot:Angular Pos) 
		0x75, 0x08, 									 //   REPORT_SIZE (8) 
		0x95, 0x01, 								   //   REPORT_COUNT (1)
		0x81, 0x02, 									 //   INPUT (Data,Var,Abs)
		0x09, 0x39, 									 //   USAGE (Hat switch)
		0x81, 0x02, 									 //   INPUT (Data,Var,Abs)
		0x09, 0x39, 									 //   USAGE (Hat switch)
		0x81, 0x02, 									 //   INPUT (Data,Var,Abs)
		0x09, 0x39, 									 //   USAGE (Hat switch)
		0x81, 0x02, 									 //   INPUT (Data,Var,Abs)

		0xc0,                           // END_COLLECTION
};

// __ALIGN_BEGIN static uint8_t HID_JOYSTICK_ReportDesc[] __ALIGN_END = {
//   0x05, 0x01,                     // Usage Page (Generic Desktop)
//   0x09, 0x04,                     // Usage (Joystick)
//   0xA1, 0x01,                     // Collection (Application)
// 	0x15, 0x00,			// Logical Minimum (0)
// 	0x25, 0x01,			// Logical Maximum (1)
// 	0x75, 0x01,			// Report Size (1)
// 	0x95, 0x08,//0x20,			// Report Count (32)
// 	0x05, 0x09,			// Usage Page (Button)
// 	0x19, 0x01,			// Usage Minimum (Button #1)
// 	0x29, 0x08,//0x20,			// Usage Maximum (Button #32)
// 	0x81, 0x02,			// Input (variable,absolute)
// 	0x15, 0x00,			// Logical Minimum (0)
// 	0x25, 0x07,			// Logical Maximum (7)
// 	0x35, 0x00,			// Physical Minimum (0)
// 	0x46, 0x3B, 0x01,		// Physical Maximum (315)
// 	0x75, 0x04,			// Report Size (4)
// 	0x95, 0x01,			// Report Count (1)
// 	0x65, 0x14,			// Unit (20)
//   0x05, 0x01,                     // Usage Page (Generic Desktop)
// 	0x09, 0x39,			// Usage (Hat switch)
// 	0x81, 0x42,			// Input (variable,absolute,null_state)
//   0x05, 0x01,                     // Usage Page (Generic Desktop)
// 	0x09, 0x01,			// Usage (Pointer)
//   0xA1, 0x00,                     // Collection ()
// 	0x15, 0x00,			//   Logical Minimum (0)
// 	0x26, 0xFF, 0x03,		//   Logical Maximum (1023)
// 	0x75, 0x0A,			//   Report Size (10)
// 	0x95, 0x04,			//   Report Count (4)
// 	0x09, 0x30,			//   Usage (X)
// 	0x09, 0x31,			//   Usage (Y)
// 	0x09, 0x32,			//   Usage (Z)
// 	0x09, 0x35,			//   Usage (Rz)
// 	0x81, 0x02,			//   Input (variable,absolute)
//         0xC0,                           // End Collection
// 	0x15, 0x00,			// Logical Minimum (0)
// 	0x26, 0xFF, 0x03,		// Logical Maximum (1023)
// 	0x75, 0x0A,			// Report Size (10)
// 	0x95, 0x02,			// Report Count (2)
// 	0x09, 0x36,			// Usage (Slider)
// 	0x09, 0x36,			// Usage (Slider)
// 	0x81, 0x02,			// Input (variable,absolute)
//         0xC0                            // End Collection
// };

/**
  * @}
  */

/** @defgroup USBD_HID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev,
                             uint8_t cfgidx)
{
  UNUSED(cfgidx);

  USBD_HID_HandleTypeDef *hhid;

  hhid = (USBD_HID_HandleTypeDef *)USBD_malloc(sizeof(USBD_HID_HandleTypeDef));

  if (hhid == NULL) {
    pdev->pClassData = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassData = (void *)hhid;

  pdev->ep_in[HID_JOYSTICK_EPIN_ADDR & 0xFU].bInterval = HID_FS_BINTERVAL;  

  /* Open EP IN */
  (void)USBD_LL_OpenEP(pdev, HID_JOYSTICK_EPIN_ADDR, USBD_EP_TYPE_INTR,
                       HID_JOYSTICK_EPIN_SIZE);
  pdev->ep_in[HID_JOYSTICK_EPIN_ADDR & 0xFU].is_used = 1U;

  hhid->Joystickstate = HID_IDLE;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_HID_DeInit
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev,
                               uint8_t cfgidx)
{
  UNUSED(cfgidx);

  /* Close HID EPs */
  (void)USBD_LL_CloseEP(pdev, HID_JOYSTICK_EPIN_ADDR);
  pdev->ep_in[HID_JOYSTICK_EPIN_ADDR & 0xFU].is_used = 0U;
  pdev->ep_in[HID_JOYSTICK_EPIN_ADDR & 0xFU].bInterval = 0U;


  /* Free allocated memory */
  if (pdev->pClassData != NULL) {
    (void)USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_COMPOSITE_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_COMPOSITE_HID_Setup(USBD_HandleTypeDef *pdev,
                                         USBD_SetupReqTypedef *req)
{

    return USBD_HID_JOYSTICK_Setup(pdev, req);
  
}

/**
  * @brief  USBD_HID_KEYBOARD_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_HID_JOYSTICK_Setup(USBD_HandleTypeDef *pdev,
                                        USBD_SetupReqTypedef *req)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *) pdev->pClassData;
  uint16_t len = 0U;
  uint8_t *pbuf = NULL;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS:
      switch (req->bRequest) {


        case HID_REQ_SET_PROTOCOL:
          hhid->Protocol = (uint8_t)(req->wValue);
          break;

        case HID_REQ_GET_PROTOCOL:
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->Protocol, 1U);
          break;

        case HID_REQ_SET_IDLE:
          hhid->IdleState = (uint8_t)(req->wValue >> 8);
          break;

        case HID_REQ_GET_IDLE:
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->IdleState, 1U);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest) {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED) {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          } else {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;
        case USB_REQ_GET_DESCRIPTOR:
          if ((req->wValue >> 8) == HID_REPORT_DESC) {
            len = MIN(HID_JOYSTICK_REPORT_DESC_SIZE, req->wLength);
            pbuf = HID_JOYSTICK_ReportDesc;
          } else {
            if ((req->wValue >> 8) == HID_DESCRIPTOR_TYPE) {
              pbuf = USBD_JOYSTICK_HID_Desc;
              len = MIN(USB_HID_DESC_SIZ, req->wLength);
            } else {
              USBD_CtlError(pdev, req);
              ret = USBD_FAIL;
              break;
            }
          }
          (void)USBD_CtlSendData(pdev, pbuf, len);
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED) {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&hhid->AltSetting, 1U);
          } else {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED) {
            hhid->AltSetting = (uint8_t)(req->wValue);
          } else {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return (uint8_t)ret;
}


/**
  * @brief  USBD_HID_GetPollingInterval
  *         return polling interval from endpoint descriptor
  * @param  pdev: device instance
  * @retval polling interval
  */
uint32_t USBD_HID_GetPollingInterval(USBD_HandleTypeDef *pdev)
{
  uint32_t polling_interval = 0U;

  polling_interval = HID_FS_BINTERVAL;
  

  return ((uint32_t)(polling_interval));
}

/**
  * @brief  USBD_HID_GetCfgFSDesc
  *         return FS configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_HID_GetFSCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_HID_CfgFSDesc);

  return USBD_HID_CfgFSDesc;
}

/**
  * @brief  USBD_HID_SendReport
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_JOYSTICK_SendReport(USBD_HandleTypeDef  *pdev,
                                  uint8_t *report,
                                  uint16_t len)
{
  USBD_HID_HandleTypeDef *hhid = (USBD_HID_HandleTypeDef *)pdev->pClassData;

  if (hhid == NULL) {
    return (uint8_t)USBD_FAIL;
  }

  if (pdev->dev_state == USBD_STATE_CONFIGURED) {
    if (hhid->Joystickstate == HID_IDLE) {
      hhid->Joystickstate = HID_BUSY;
      (void)USBD_LL_Transmit(pdev, HID_JOYSTICK_EPIN_ADDR, report, len);
    } else {
      return (uint8_t)USBD_BUSY;
    }
  }
  return (uint8_t)USBD_OK;
}



/**
  * @brief  USBD_HID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef *pdev,
                               uint8_t epnum)
{

  /* Ensure that the FIFO is empty before a new transfer, this condition could
  be caused by  a new transfer before the end of the previous transfer */
  if (epnum == (HID_JOYSTICK_EPIN_ADDR & 0x7F)) {
    ((USBD_HID_HandleTypeDef *)pdev->pClassData)->Joystickstate = HID_IDLE;
  }
  return (uint8_t)USBD_OK;
}


#endif /* USBD_USE_HID_COMPOSITE_JOY */
#endif /* USBCON */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
