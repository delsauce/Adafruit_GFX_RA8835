/******************************************************************
 This is the core graphics library for all our displays, providing
 basic graphics primitives (points, lines, circles, etc.). It needs
 to be paired with a hardware-specific library for each display
 device we carry (handling the lower-level functions).
 
 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source hardware
 by purchasing products from Adafruit!
 
 Written by Limor Fried/Ladyada for Adafruit Industries.
 BSD license, check license.txt for more information.
 All text above must be included in any redistribution.
 ******************************************************************/

#ifndef _ADAFRUIT_GFX_RA8835_H
#define _ADAFRUIT_GFX_RA8835_H

#include <Adafruit_GFX.h>

#define LCDWIDTH	320
#define LCDHEIGHT	240
#define BPP		1	// Bits per pixel - greyscale

class Adafruit_GFX_RA8835 : public Adafruit_GFX {
 public:
	Adafruit_GFX_RA8835(int8_t reset);
	virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
	void begin();
	void update(void);
	//void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
  
 private:
	uint8_t gbuf[(LCDHEIGHT*LCDWIDTH)/(8/BPP)];
	int8_t _reset;


	void clearTextLayer();
	
};

#endif
