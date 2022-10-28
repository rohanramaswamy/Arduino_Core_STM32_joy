#include <stdint.h>

#ifdef USBD_USE_HID_COMPOSITE_JOYSTICK
#include <Joystick.h>
#include "usbd_hid_composite_joy_if.h"

#define MAX_AXIS_NUM							8						// max 8
#define MAX_BUTTONS_NUM						128					// power of 2, max 128
#define MAX_POVS_NUM							4

uint8_t usb_joystick_data[3];

void Joystick_::begin(void)
{
  HID_Composite_Init(HID_JOYSTICK);
}

void Joystick_::end(void)
{
  HID_Composite_DeInit(HID_JOYSTICK);
}

Joystick_::Joystick_(void)
{
  manual_mode = false;
}

void Joystick_::button(uint8_t button, bool val)
{
  if (--button >= 32)
    return;
  if (val)
    usb_joystick_data[0] |= (1 << button);
  else
    usb_joystick_data[0] &= ~(1 << button);
  if (!manual_mode)
    usb_joystick_send();
}

void Joystick_::X(unsigned int val)
{
  if (val > 1023)
    val = 1023;
  usb_joystick_data[1] = (usb_joystick_data[1] & 0xFFFFC00F) | (val << 4);
  if (!manual_mode)
    usb_joystick_send();
}

void Joystick_::use_manual_send(bool mode)
{
  manual_mode = mode;
}

void Joystick_::send_now(void)
{
  usb_joystick_send();
}

void Joystick_::usb_joystick_send(void)
{
  // 
  uint8_t	report_buf[37];
  uint8_t	pos = 0;

  report_buf[pos++] = 1;
  // uint8_t button_data[MAX_BUTTONS_NUM/8];
  // button_data[0] = 255;
  // memcpy(&report_buf[pos], button_data, MAX_BUTTONS_NUM/8);
	// pos += 17;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;
  report_buf[pos++] = 0b10101010;

  for (uint8_t i=0; i<MAX_AXIS_NUM; i++) {
					
						report_buf[pos++] = (uint8_t) (200 & 0xFF);
						report_buf[pos++] = (uint8_t) (200 >> 8);							
					
	}
			
      for (uint8_t i=0; i<MAX_POVS_NUM; i++)
			{
					
						report_buf[pos++] = 2;
					
			}

      HID_Composite_joystick_sendReport(report_buf, 37);
}

Joystick_ Joystick;

#endif