#ifndef APA102_H
#define APA102_H

typedef struct {
	union {
		struct {
			uint8_t brightness;
			union {
				uint8_t blue;
				uint8_t b;
			};
			union {
				uint8_t green;
				uint8_t g;
			};
			union {
				uint8_t red;
				uint8_t r;
			};
		};
		uint32_t raw;
	};
} CRGB;

void apa102_set_color(CRGB *led, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);
void apa102_set_strip_color(CRGB *led, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, uint16_t len);

void apa102_set_color_crgb(CRGB* led, CRGB color);
void apa102_set_strip_color_crgb(CRGB* led, CRGB color, uint16_t len);

void apa102_copy_strip(CRGB *dest, CRGB *src, uint16_t len);

void apa102_write_strip(CRGB *led, uint16_t len);

#endif