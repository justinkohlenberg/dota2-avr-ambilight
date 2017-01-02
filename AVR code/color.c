/*
 * color.c
 *
 * Created: 2-1-2017 23:23:57
 *  Author: Justin
 */ 

#include "color.h"

//Add color 1 to Color 2 by adding each component and then taking the average of the result, returning the calculated color
rgb_color add(rgb_color color_1, rgb_color color_2) {
	return (rgb_color) {(color_1.red + color_2.red) / 2, (color_1.green + color_2.green) / 2, (color_1.blue + color_2.blue) / 2};
}

rgb_color multiply(rgb_color color, uint8_t multiplier) {
	return (rgb_color) {(color.red%255), (color.green%255), (color.blue%255)};
}

rgb_color divide(rgb_color color, uint8_t divider) {
	return (rgb_color) {(color.red/divider), (color.green/divider), (color.blue/divider)};
}

// (color2 - color1) * fraction + color1
rgb_color interpolate(rgb_color color_1, rgb_color color_2, float ratio) {
	ratio = ratio < 1.0f ? ratio : 1.0f;
	
	unsigned char r = (color_2.red - color_1.red) * ratio + color_1.red;
	unsigned char g = (color_2.green - color_1.green) * ratio + color_1.green;
	unsigned char b = (color_2.blue - color_1.blue) * ratio + color_1.blue;
	
	return (rgb_color) {r, g, b};
}