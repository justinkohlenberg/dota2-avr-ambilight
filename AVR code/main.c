#define UART_RX0_BUFFER_SIZE 512
#define UART_TX0_BUFFER_SIZE 1

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"
#include "ws2812b.h"

//baud rate for serial communication with PC
#define BAUD_RATE 9600

//amount of LEDS to control
#define LED_COUNT 60

//state definitions, current state is decided by node application and sent over serial
#define LED_MODE_CONFIGURE 0
#define LED_MODE_STATUS 1
#define LED_MODE_VICTORY 2
#define LED_MODE_STUNNED 3

//each color in this array represents a single led, index 0 being the first LED.
rgb_color colors[LED_COUNT];

int main()
{	
	//initialize UART and make sure buffer is empty
	uart0_init(UART_BAUD_SELECT(BAUD_RATE, F_CPU));
	uart0_flush();
	
	uint8_t timer = 0;
	uint8_t incr = 3;
	uint8_t victory = 0;
	uint8_t brightness = 25;
	
	rgb_color colorOff = (rgb_color){0, 0, 0};
	
	DDRD &= ~(1<<PD3);
	DDRD &= ~(1<<PD2);
	
	DDRB &= (1<<PB1);
	
	PORTD |= (1<<PD3)|(1<<PD2);
	
	for(uint8_t i = 0; i < LED_COUNT; i++)
	{
		colors[i] = colorOff;
	}

	led_strip_write(colors, LED_COUNT);
	_delay_ms(10);
	//uart0_puts("r");
	
	while(1)
	{		
		PORTD |= (1<<7);
		uint8_t updated = 0;
		while(uart0_available() > 0) {
			uint8_t ledMode = uart0_getc();
			_delay_ms(10);
			switch(ledMode)
			{
				case LED_MODE_CONFIGURE:
				{
					brightness = uart0_getc();
					_delay_ms(10);
				}
				case LED_MODE_STUNNED:
				{
					for(uint8_t i = 0; i < 40; i++)
					{
						colors[i] = (rgb_color){brightness*2, brightness*2, brightness*2};
					}
					break;
				}
				case LED_MODE_STATUS:
				{
				//TODO: change this so it only updates LEDS that have changed instead of refilling all indices of the colors array
					for(uint8_t i = 0; i < 2; i++) {
						rgb_color color;
						rgb_color dimColor;
						if(i == 0)
							color = (rgb_color){0, brightness, 0};
							dimColor = (rgb_color){0, brightness/2, 0}; 
						if(i == 1)
							color = (rgb_color){0, 0, brightness};
							dimColor  = (rgb_color){0, 0, brightness/2};
						uint8_t ledAmount = uart0_getc();
						for(uint8_t l = 0 + (i*20); l < ledAmount + (i*20); l++)
						{
							colors[l] = color;
						}
						if((uint8_t) uart0_getc())
						{
							colors[ledAmount + (i*20) + 1] = dimColor;
						}
						for(uint8_t l = ledAmount + (i*20); l < ledAmount + 20 + (i*20); l++)
						{
							colors[l] = (rgb_color){4, 0, 0};
						}
						_delay_ms(10);
					}
					break;
				}
				case LED_MODE_VICTORY:
				{
					victory = uart0_getc();
					
					break;
				}
				default:
					break;
			}
			
			updated = 1;
		}
		
		if(victory)
		{
			for(uint8_t l = 0; l < 40; l++)
			{
				colors[l] = (rgb_color){0, timer, 0};
			}
			timer += incr;
			if (timer % brightness == 0)
				incr *= -1;
			led_strip_write(colors, LED_COUNT);
			_delay_ms(10);
		}
		
		if(updated && !victory) {
			led_strip_write(colors, LED_COUNT);
			_delay_ms(10);
			uart0_puts("r");
			updated = 0;
		}
	}
}