/*
 * color.h
 *
 * Created: 2-1-2017 23:24:10
 *  Author: oxyge_000
 */ 


#ifndef COLOR_H_
#define COLOR_H_

#include <stdint.h>

typedef struct rgb_color
{
	unsigned char red, green, blue;
} rgb_color;

//Operators
rgb_color add(rgb_color color_1, rgb_color color_2);
rgb_color multiply(rgb_color color, uint8_t multiplier);
rgb_color divide(rgb_color color, uint8_t divider);

rgb_color interpolate(rgb_color color_1, rgb_color color_2, float ratio);

#endif /* COLOR_H_ */