/*
 * ws2812b.h
 *
 * Created: 16-11-2016 10:59:02
 *  Author: Justin
 */ 

#ifndef WS2812B_H_
#define WS2812B_H_

#include "color.h"

#ifndef LED_STRIP_PORT
	#define LED_STRIP_PORT PORTC /*Which port is the LED strip connected to, defaults to PORTC*/
#endif
#ifndef LED_STRIP_DDR
	#define LED_STRIP_DDR DDRC /*DDR register of the port specified in LED_STRIP_PORT, defaults to DDRC*/
#endif
#ifndef LED_STRIP_PIN
	#define LED_STRIP_PIN 0 /*Which pin is the LED strip connected to, defaults to PC0*/
#endif

void __attribute__((noinline)) led_strip_write(rgb_color * colors, unsigned int count);

#endif /* WS2812B_H_ */