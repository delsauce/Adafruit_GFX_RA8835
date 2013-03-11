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

#include "Adafruit_GFX_RA8835.h"
#include <Parallel.h>

// Bus/memory addresses for the RA8835
#define DATA_ADDR   0
#define CMD_ADDR    1

// Memory addresses for the graphics and text layers inside the RA8835
#define TEXT_LAYER_ADDR_L     0x00
#define TEXT_LAYER_ADDR_H     0x00
#define GRAPHICS_LAYER_ADDR_L 0x60
#define GRAPHICS_LAYER_ADDR_H 0x09

typedef enum 
{
    REG_SYSTEM_SET = 0x40,
    MEM_WRITE = 0x42,
    MEM_READ = 0x43,
    SCROLL = 0x44,
    SET_CURSOR_ADDR = 0x46,
    READ_CURSOR_ADDR = 0x47,
    CSR_DIR_RIGHT = 0x4C,
    CSR_DIR_LEFT = 0x4D,
    CSR_DIR_UP = 0x4E,
    CSR_DIR_DOWN = 0x4F,
    POWER_SAVE = 0x53,
    DISP_OFF = 0x58,
    DISP_ON = 0x59,
    HDOT_SCR = 0x5A,
    OVLAY = 0x5B,
    CG_RAM_ADR = 0x5C,
    CSR_FORM = 0x5D,
    GRAYSCALE = 0x60,
} LCDReg_t;


// constructor
Adafruit_GFX_RA8835::Adafruit_GFX_RA8835(int8_t reset) 
{
  // Save reset pin
  _reset = reset;
	
  // call base class constructor
  constructor(LCDWIDTH, LCDHEIGHT);
}

void Adafruit_GFX_RA8835::begin()
{
  // configure the parallel port of the Due
  Parallel.begin(PARALLEL_BUS_WIDTH_8, PARALLEL_CS_1, 1, 1, 1);
  Parallel.setAddressSetupTiming(5,1,5,1);
  Parallel.setPulseTiming(20,30,20,30);
  Parallel.setCycleTiming(55,55);

  // yank the HW reset line
  pinMode(_reset, OUTPUT);
  digitalWrite(_reset, 1);
  delay(3);
  digitalWrite(_reset, 0);
  delay(3);
  digitalWrite(_reset, 1);
  delay(10);

  // initialize the display
  // system setup
  Parallel.write(CMD_ADDR, REG_SYSTEM_SET);
  Parallel.write(DATA_ADDR, 0x30);	// no origin comp, single panel, 
  					//  8pix char height, internal CGROM
  Parallel.write(DATA_ADDR, 0x87);	// 8 pix char width
  Parallel.write(DATA_ADDR, 0x07);	// 8 pix char height (why twice?)
  Parallel.write(DATA_ADDR, 0x27);	// 40 chars wide
  Parallel.write(DATA_ADDR, 0x39);	
  Parallel.write(DATA_ADDR, 0xEF);	// 240 lines high
  Parallel.write(DATA_ADDR, 0x28);	// horizontal address range 40 (?)
  Parallel.write(DATA_ADDR, 0x00);

  Parallel.write(CMD_ADDR, SCROLL);	// Screen block start addresses
  Parallel.write(DATA_ADDR, TEXT_LAYER_ADDR_L);	// Block 1 @ 0x0
  Parallel.write(DATA_ADDR, TEXT_LAYER_ADDR_H);
  Parallel.write(DATA_ADDR, 0xEF);	// 240 lines
  Parallel.write(DATA_ADDR, GRAPHICS_LAYER_ADDR_L);	// Block 2 start address
  Parallel.write(DATA_ADDR, GRAPHICS_LAYER_ADDR_H);	// 0x0960
  Parallel.write(DATA_ADDR, 0xEF);	// 240 lines
  Parallel.write(DATA_ADDR, 0x00);	// Block 3 start address (not used)
  Parallel.write(DATA_ADDR, 0x00);
  Parallel.write(DATA_ADDR, 0x00);	// Block 4 start address (not used) 
  Parallel.write(DATA_ADDR, 0x00);

  Parallel.write(CMD_ADDR, HDOT_SCR);	// Horizontal Scroll
  Parallel.write(DATA_ADDR, 0x00);	  // 0 bits
	
  Parallel.write(CMD_ADDR, OVLAY);	// OVRLY
  Parallel.write(DATA_ADDR, 0x01);	// three layers, layer 3 text, 
  					                        //  layer 1 text, XOR
	
  Parallel.write(CMD_ADDR, DISP_OFF);	// DISP_OFF
  Parallel.write(DATA_ADDR, 0x16);	// No Flashing, just graphics

  // clear screen
  memset(&gbuf, 0x00, sizeof(gbuf));
  update();

  // clear text layer
  clearTextLayer();
  
  // Cursor direction	
  Parallel.write(CMD_ADDR, CSR_DIR_RIGHT);
  
  Parallel.write(CMD_ADDR, CSR_FORM);	// cursor mode
  Parallel.write(DATA_ADDR, 0x07);	// 8 bits wide
  Parallel.write(DATA_ADDR, 0x87);	// Block style, 8 bits high
  	
  // Turn display on	
  Parallel.write(CMD_ADDR, DISP_ON);	// DISP_ON
  Parallel.write(DATA_ADDR, 0x16);	// No Flashing, graphics and text
                                    // Note: text layer not in use yet.
}

// draw a single pixel
// Note that this does not update the display -- only the local buffer
// make sure to call update() to dump the buffer to the LCD
void Adafruit_GFX_RA8835::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  uint16_t offset = y*(320/8) + x/8;
  uint8_t bit = x%8;

  if (color > 0)
  {
	  gbuf[offset] = (gbuf[offset])|(0x80>>bit);
  }
  else
  {
	  gbuf[offset] = (gbuf[offset])&(~(0x80>>bit));
  }
}

// dump the graphics buffer to the screen
void Adafruit_GFX_RA8835::update()
{
  Parallel.write(CMD_ADDR, SET_CURSOR_ADDR);
  Parallel.write(DATA_ADDR, GRAPHICS_LAYER_ADDR_L);
  Parallel.write(DATA_ADDR, GRAPHICS_LAYER_ADDR_H);
	
  Parallel.write(CMD_ADDR, MEM_WRITE);
  for (int i=0; i < sizeof(gbuf); i++)	
  {
	Parallel.write(DATA_ADDR, gbuf[i]);
  }
}

void Adafruit_GFX_RA8835::clearTextLayer()
{
  Parallel.write(CMD_ADDR, SET_CURSOR_ADDR);
  Parallel.write(DATA_ADDR, TEXT_LAYER_ADDR_L);
  Parallel.write(DATA_ADDR, TEXT_LAYER_ADDR_H);
	
  Parallel.write(CMD_ADDR, MEM_WRITE);
  for (int i=0; i < ((LCDWIDTH/8)*(LCDHEIGHT*8)); i++)	
  {
	Parallel.write(DATA_ADDR, ' ');
  }
}

