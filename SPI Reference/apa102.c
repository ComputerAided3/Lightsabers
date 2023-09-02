#include <string.h>

#include "sam.h"
#include "apa102.h"
#include "spi.h"

void apa102_set_color(CRGB *led, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
	led->r = r;
	led->g = g;
	led->b = b;
	led->brightness = 0xE0 | brightness;
}

void apa102_set_strip_color(CRGB *led, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, uint16_t len) {
	for (uint16_t i = 0; i < len; ++i) {
		apa102_set_color(led+i, r, g, b, brightness);
	}
}

void apa102_set_color_crgb(CRGB *led, CRGB color) {
	*led = color;
}

void apa102_set_strip_color_crgb(CRGB* led, CRGB color, uint16_t len) {
	for (uint16_t i = 0; i < len; ++i) {
		apa102_set_color_crgb(led+i, color);
	}
}

void apa102_copy_strip(CRGB *dest, CRGB *src, uint16_t len) {
	memcpy(dest, src, len * sizeof(CRGB));
}

void apa102_write_strip(CRGB *led, uint16_t len) {
	uint8_t frame[] = {0, 0, 0, 0}; // Start/end frame
	
	spi_write_bytes(frame, 4);
	spi_write_bytes((uint8_t*) led, len * sizeof(CRGB));
	spi_write_bytes(frame, 4);
}