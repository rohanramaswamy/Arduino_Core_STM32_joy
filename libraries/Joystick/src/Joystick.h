
#ifndef JOYSTICK_h
#define JOYSTICK_h

#include <Arduino.h>

#if !defined(USBCON) || !defined(USBD_USE_HID_COMPOSITE_JOYSTICK)

#error "USB HID Joystick not enabled! Select 'HID Joystick' in the 'Tools->USB interface' menu."

#else

void usb_joystick_configure(void);
int usb_joystick_send(void);


class Joystick_
{
private:
  bool manual_mode;
  void usb_joystick_send(void);

public:
  Joystick_(void);
  void begin(void);
  void end(void);
  void button(uint8_t button, bool val);
  void X(unsigned int val);
  void use_manual_send(bool mode);
	void send_now(void);
};
extern Joystick_ Joystick;

#endif
#endif
