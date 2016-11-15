
// These lines specify what pin the LED strip is on.
// You will either need to attach the LED strip's data line to PC0 or change these
// lines to specify a different pin.
#define LED_STRIP_PORT PORTC
#define LED_STRIP_DDR  DDRC
#define LED_STRIP_PIN  0

#define UART_RX0_BUFFER_SIZE 512
#define UART_TX0_BUFFER_SIZE 1

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart.h"

typedef struct rgb_color
{
	unsigned char red, green, blue;
} rgb_color;

/** led_strip_write sends a series of colors to the LED strip, updating the LEDs.
The colors parameter should point to an array of rgb_color structs that hold the colors to send.
The count parameter is the number of colors to send.

This function takes about 1.1 ms to update 30 LEDs.
Interrupts must be disabled during that time, so any interrupt-based library
can be negatively affected by this function.

Timing details at 20 MHz (the numbers slightly different at 16 MHz and 8MHz):
0 pulse  = 400 ns
1 pulse  = 850 ns
"period" = 1300 ns

SOURCE: https://github.com/pololu/pololu-led-strip-avr/blob/master/led_strip.c
*/
void __attribute__((noinline)) led_strip_write(rgb_color * colors, unsigned int count)
{
	// Set the pin to be an output driving low.
	LED_STRIP_PORT &= ~(1<<LED_STRIP_PIN);
	LED_STRIP_DDR |= (1<<LED_STRIP_PIN);

	cli();   // Disable interrupts temporarily because we don't want our pulse timing to be messed up.
	while(count--)
	{
		// Send a color to the LED strip.
		// The assembly below also increments the 'colors' pointer,
		// it will be pointing to the next color at the end of this loop.
		asm volatile(
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0\n"
		"rcall send_led_strip_byte%=\n"  // Send red component.
		"ld __tmp_reg__, -%a0\n"
		"rcall send_led_strip_byte%=\n"  // Send green component.
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0+\n"
		"ld __tmp_reg__, %a0+\n"
		"rcall send_led_strip_byte%=\n"  // Send blue component.
		"rjmp led_strip_asm_end%=\n"     // Jump past the assembly subroutines.

		// send_led_strip_byte subroutine:  Sends a byte to the LED strip.
		"send_led_strip_byte%=:\n"
		"rcall send_led_strip_bit%=\n"  // Send most-significant bit (bit 7).
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"
		"rcall send_led_strip_bit%=\n"  // Send least-significant bit (bit 0).
		"ret\n"

		// send_led_strip_bit subroutine:  Sends single bit to the LED strip by driving the data line
		// high for some time.  The amount of time the line is high depends on whether the bit is 0 or 1,
		// but this function always takes the same time (2 us).
		"send_led_strip_bit%=:\n"
		#if F_CPU == 8000000
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		#endif
		"sbi %2, %3\n"                           // Drive the line high.

		#if F_CPU != 8000000
		"rol __tmp_reg__\n"                      // Rotate left through carry.
		#endif

		#if F_CPU == 16000000
		"nop\n" "nop\n"
		#elif F_CPU == 20000000
		"nop\n" "nop\n" "nop\n" "nop\n"
		#elif F_CPU != 8000000
		#error "Unsupported F_CPU"
		#endif

		"brcs .+2\n" "cbi %2, %3\n"              // If the bit to send is 0, drive the line low now.

		#if F_CPU == 8000000
		"nop\n" "nop\n"
		#elif F_CPU == 16000000
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		#elif F_CPU == 20000000
		"nop\n" "nop\n" "nop\n" "nop\n" "nop\n"
		"nop\n" "nop\n"
		#endif

		"brcc .+2\n" "cbi %2, %3\n"              // If the bit to send is 1, drive the line low now.

		"ret\n"
		"led_strip_asm_end%=: "
		: "=b" (colors)
		: "0" (colors),         // %a0 points to the next color to display
		"I" (_SFR_IO_ADDR(LED_STRIP_PORT)),   // %2 is the port register (e.g. PORTC)
		"I" (LED_STRIP_PIN)     // %3 is the pin number (0-8)
		);

		// Uncomment the line below to temporarily enable interrupts between each color.
		sei(); asm volatile("nop\n"); cli();
	}
	sei();          // Re-enable interrupts now that we are done.
	_delay_us(50);  // Hold the line low for 15 microseconds to send the reset signal.
}

//baud rate for serial communication with PC
#define BAUD_RATE 9600

//amount of LEDS to control
#define LED_COUNT 60

//state definitions, current state is decided by node application and sent over serial
#define LED_MODE_STATUS 1
#define LED_MODE_VICTORY 2
#define LED_MODE_STUNNED 3

//each color in this array represents a single led, index 0 being the first LED.
rgb_color colors[LED_COUNT];

int main()
{	
	//init UART and make sure buffer is empty
	uart0_init(UART_BAUD_SELECT(BAUD_RATE, F_CPU));
	uart0_flush();
	
	uint8_t timer = 0;
	uint8_t incr = 3;
	uint8_t victory = 0;
	
	DDRD &= ~(1<<PD3);
	DDRD &= ~(1<<PD2);
	
	DDRB &= (1<<PB1);
	
	PORTD |= (1<<PD3)|(1<<PD2);
	
	for(uint8_t i = 0; i < LED_COUNT; i++)
	{
		colors[i] = (rgb_color){0, 0, 0};
	}

	led_strip_write(colors, LED_COUNT);
	_delay_ms(20);
	//uart0_puts("ready");
	
	while(1)
	{		
		PORTD |= (1<<7);
		uint8_t updated = 0;
		while(uart0_available() > 0) {
			uint8_t ledMode = uart0_getc();
			_delay_ms(20);
			switch(ledMode)
			{
				case LED_MODE_STUNNED:
				{
					for(uint8_t i = 0; i < 40; i++)
					{
						colors[i] = (rgb_color){122, 122, 122};
					}
					break;
				}
				case LED_MODE_STATUS:
				{
					for(uint8_t i = 0; i < 2; i++) {
						rgb_color color;
						if(i == 0) {
							color = (rgb_color){0, 25, 0};
							} else if(i == 1) {
							color = (rgb_color){0, 0, 25};
						}
						uint8_t ledAmount = uart0_getc();
						for(uint8_t l = 0 + (i*20); l < ledAmount + (i*20); l++)
						{
							colors[l] = color;
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
			if (timer % 60 == 0)
				incr *= -1;
			led_strip_write(colors, LED_COUNT);
			_delay_ms(20);
		}
		
		if(updated && !victory) {
			led_strip_write(colors, LED_COUNT);
			_delay_ms(20);
			uart0_puts("r");
			updated = 0;
		}
	}
}