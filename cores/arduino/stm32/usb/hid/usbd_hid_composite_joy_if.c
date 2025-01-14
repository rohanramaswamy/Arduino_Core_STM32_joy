/**
  ******************************************************************************
  * @file    usbd_hid_composite_if.c
  * @brief   Provide the USB HID composite interface
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics
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
#include <stdbool.h>
#include "usbd_desc.h"
#include "usbd_hid_composite_joy_if.h"
#include "usbd_hid_composite_joy.h"

#ifdef __cplusplus
extern "C" {
#endif

/* USB Device Core HID composite handle declaration */
USBD_HandleTypeDef hUSBD_Device_HID;

// static bool HID_keyboard_initialized = false;
// static bool HID_mouse_initialized = false;
static bool HID_joystick_initialized = false;

/**
  * @brief  Initialize USB devices
  * @param  HID_Interface device type: HID_KEYBOARD or HID_MOUSE
  * @retval none
  */
void HID_Composite_Init(HID_Interface device)
{
  if (IS_HID_INTERFACE(device) && !HID_joystick_initialized) {
    /* Init Device Library */
    if (USBD_Init(&hUSBD_Device_HID, &USBD_Desc, 0) == USBD_OK) {
      /* Add Supported Class */
      if (USBD_RegisterClass(&hUSBD_Device_HID, USBD_COMPOSITE_HID_CLASS) == USBD_OK) {
        /* Start Device Process */
        USBD_Start(&hUSBD_Device_HID);
        HID_joystick_initialized = true;
      }
    }
  }
}

/**
  * @brief  DeInitialize USB devices
  * @param  HID_Interface device type: HID_KEYBOARD or HID_MOUSE
  * @retval none
  */
void HID_Composite_DeInit(HID_Interface device)
{
  if (IS_HID_INTERFACE(device) && HID_joystick_initialized ) {
    /* Stop Device Process */
    USBD_Stop(&hUSBD_Device_HID);
    /* DeInit Device Library */
    USBD_DeInit(&hUSBD_Device_HID);
  }
}

/**
  * @brief  Send HID keyboard Report
  * @param  report pointer to report
  * @param  len report length
  * @retval none
  */
void HID_Composite_joystick_sendReport(uint8_t *report, uint16_t len)
{
  USBD_HID_JOYSTICK_SendReport(&hUSBD_Device_HID, report, len);
}

#ifdef __cplusplus
}
#endif
#endif /* USBD_USE_HID_COMPOSITE_JOYSTICK */
#endif /* USBCON */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
