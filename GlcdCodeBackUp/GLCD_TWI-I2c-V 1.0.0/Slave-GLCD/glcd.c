/*
 * ATmega8 + JHD12864E / KS0108
 *
 * TEXT + IMAGE TEST
 *
 * ATmega8 @ 16 MHz
 *
 * GLCD CONNECTION:
 *
 * PC0 -> RS
 * PC1 -> R/W
 * PC2 -> E
 *
 * PB0 -> CS1
 * PB1 -> CS2
 *
 * PORTD:
 * PD0 -> DB0
 * PD1 -> DB1
 * PD2 -> DB2
 * PD3 -> DB3
 * PD4 -> DB4
 * PD5 -> DB5
 * PD6 -> DB6
 * PD7 -> DB7
 */
//GLCD_DisplayScreen(0);   // Display SCREEN at index 0

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include "i2c.h"
#include "glcd.h"
/* =========================================================
   CONTROL PINS
   ========================================================= */

#define RS   PC2
#define RW   PC1
#define EN   PC0

#define CS1  PB1
#define CS2  PB0

#define GLCD_rst  PB2
/* =========================================================
   PORT DEFINITIONS
   ========================================================= */

#define CTRL_PORT_C PORTC
#define CTRL_DDR_C  DDRC

#define CTRL_PORT_B PORTB
#define CTRL_DDR_B  DDRB

#define DATA_PORT   PORTD
#define DATA_DDR    DDRD

const uint8_t icons16[][32] PROGMEM =
{
	{
		// battery 0
	/* Top 8 */
	0x00,0x00,0xFC,0x04,
	0x04,0x04,0x26,0x76,
	0x26,0x06,0x04,0x04,
	0x04,0xFC,0x00,0x00,

	/* Bottom 8 */
	0x00,0x00,0x7F,0x7C,
	0x7C,0x7C,0x7C,0x7C,
	0x7C,0x7C,0x7C,0x7C,
	0x7C,0x7F,0x00,0x00
  },
  
  //Heart 1
  {
	 /* Top 8 */
	 0x00,0x70,0x8C,0x02,
	 0x02,0x02,0x04,0x18,
	 0x18,0x04,0x02,0x02,
	 0x02,0x8C,0x70,0x00,

	 /* Bottom 8 */
	 0x00,0x00,0x03,0x04,
	 0x08,0x10,0x20,0x40,
	 0x40,0x20,0x10,0x08,
	 0x04,0x03,0x00,0x00
  },
  
  // power Button 2
  {
	  // flipped CW - upper half
	  0x00,0xF8,0xFC,0x1E,
	  0x0E,0x07,0x03,0xE3,
	  0xE3,0x07,0x0F,0x1E,
	  0x1E,0xFC,0xE8,0x00,

	  // flipped CW - lower half
	  0x00,0x0F,0x1F,0x3C,
	  0x38,0x70,0x60,0x63,
	  0x63,0x70,0x78,0x3C,
	  0x3C,0x1F,0x0B,0x00,
  },
  
  // face 3
  {
	  // flipped CW - upper half
	  0x00,0xE0,0x10,0x48,
	  0xA4,0x42,0x02,0x02,
	  0x02,0x02,0x42,0xA4,
	  0x48,0x10,0xE0,0x00,

	  // flipped CW - lower half
	  0x00,0x07,0x08,0x10,
	  0x20,0x44,0x4C,0x55,
	  0x55,0x4C,0x44,0x20,
	  0x10,0x08,0x07,0x00
  },
  //-------------- right arrow 4
  {
	  // flipped CW - upper half. 16 pix by 16 pix icon
	  0x00,0x00,0xF0,0x10,
	  0x10,0x10,0x10,0x10,
	  0x1E,0x04,0x08,0x10,
	  0x20,0x40,0x80,0x00,

	  // flipped CW - lower half
	  0x00,0x00,0x07,0x04,
	  0x04,0x04,0x04,0x04,
	  0x3C,0x10,0x08,0x04,
	  0x02,0x01,0x00,0x00
  },
  
  //- radio 5
  {
	  /* Top 8 */
	  0x00,0xC0,0x20,0x22,
	  0xA4,0xA8,0xB0,0xA0,
	  0xB0,0x28,0x24,0xA2,
	  0xA0,0x20,0xC0,0x00,

	  /* Bottom 8 */
	  0x00,0x3F,0x40,0x4F,
	  0x5F,0x5F,0x5F,0x5F,
	  0x5F,0x4F,0x40,0x55,
	  0x55,0x40,0x3F,0x00
  },
  // msg 6
  {
	  /* Top 8 */
	  0x00,0xF8,0x04,0x0A,
	  0x12,0x22,0x22,0x22,
	  0x22,0x22,0x22,0x12,
	  0x0A,0x04,0xF8,0x00,

	  /* Bottom 8 */
	  0x00,0x00,0x79,0x0A,
	  0x32,0x0A,0x7A,0x02,
	  0xBA,0xAA,0xEA,0x02,
	  0x7A,0x49,0x68,0x00
  }
  
};


void GLCD_Icon16x16(
    uint8_t row,
    uint8_t column,
    uint8_t icon
)
{
    uint8_t x;
    uint8_t top;
    uint8_t bottom;

    /*
     * 16 pixels high = 2 GLCD pages.
     */
    if (row > 6)
        return;

    /*
     * 16 pixels wide.
     */
    if (column >= 128)
        return;

    for (x = 0; x < 16; x++)
    {
        /*
         * First 16 bytes = top page.
         */
        top = pgm_read_byte(
            &icons16[icon][x]
        );

        /*
         * Next 16 bytes = bottom page.
         */
        bottom = pgm_read_byte(
            &icons16[icon][16 + x]
        );

        /*
         * Top half.
         */
        if ((uint16_t)column + x < 128)
        {
            GLCD_WriteByteAt(
                row,
                column + x,
                top
            );
        }

        /*
         * Bottom half.
         */
        if ((uint16_t)column + x < 128)
        {
            GLCD_WriteByteAt(
                row + 1,
                column + x,
                bottom
            );
        }
		
    }
}




/* =========================================================
   FULL PRINTABLE ASCII 5x7 FONT
   =========================================================
   
   Supported characters:

   ASCII 0x20 = SPACE
   ASCII 0x21 = !
   ...
   ASCII 0x7E = ~

   95 characters total.

   Each character:
       5 columns
       + 1 blank column spacing

   Font format:
       bit 0 = top pixel
       bit 6 = bottom pixel
	   
	lsb is drawn from the top pixel
*/


const uint8_t font5x7[][5] PROGMEM =
{

    /* 0x20 SPACE */
    {0x00,0x00,0x00,0x00,0x00},

    /* 0x21 ! */
    {0x00,0x00,0x5F,0x00,0x00},

    /* 0x22 " */
    {0x00,0x07,0x00,0x07,0x00},

    /* 0x23 # */
    {0x14,0x7F,0x14,0x7F,0x14},

    /* 0x24 $ */
    {0x24,0x2A,0x7F,0x2A,0x12},

    /* 0x25 % */
    {0x23,0x13,0x08,0x64,0x62},

    /* 0x26 & */
    {0x36,0x49,0x55,0x22,0x50},

    /* 0x27 ' */
    {0x00,0x05,0x03,0x00,0x00},

    /* 0x28 ( */
    {0x00,0x1C,0x22,0x41,0x00},

    /* 0x29 ) */
    {0x00,0x41,0x22,0x1C,0x00},

    /* 0x2A * */
    {0x14,0x08,0x3E,0x08,0x14},

    /* 0x2B + */
    {0x08,0x08,0x3E,0x08,0x08},

    /* 0x2C , */
    {0x00,0x50,0x30,0x00,0x00},

    /* 0x2D - */
    {0x08,0x08,0x08,0x08,0x08},

    /* 0x2E . */
    {0x00,0x60,0x60,0x00,0x00},

    /* 0x2F / */
    {0x20,0x10,0x08,0x04,0x02},


    /* 0x30 0 */
    {0x3E,0x51,0x49,0x45,0x3E},

    /* 0x31 1 */
    {0x00,0x42,0x7F,0x40,0x00},

    /* 0x32 2 */
    {0x42,0x61,0x51,0x49,0x46},

    /* 0x33 3 */
    {0x21,0x41,0x45,0x4B,0x31},

    /* 0x34 4 */
    {0x18,0x14,0x12,0x7F,0x10},

    /* 0x35 5 */
    {0x27,0x45,0x45,0x45,0x39},

    /* 0x36 6 */
    {0x3C,0x4A,0x49,0x49,0x30},

    /* 0x37 7 */
    {0x01,0x71,0x09,0x05,0x03},

    /* 0x38 8 */
    {0x36,0x49,0x49,0x49,0x36},

    /* 0x39 9 */
    {0x06,0x49,0x49,0x29,0x1E},


    /* 0x3A : */
    {0x00,0x36,0x36,0x00,0x00},

    /* 0x3B ; */
    {0x00,0x56,0x36,0x00,0x00},

    /* 0x3C < */
    {0x08,0x14,0x22,0x41,0x00},

    /* 0x3D = */
    {0x14,0x14,0x14,0x14,0x14},

    /* 0x3E > */
    {0x00,0x41,0x22,0x14,0x08},

    /* 0x3F ? */
    {0x02,0x01,0x51,0x09,0x06},

    /* 0x40 @ */
    {0x3E,0x41,0x5D,0x55,0x1E},


    /* 0x41 A */
    {0x7E,0x11,0x11,0x11,0x7E},

    /* 0x42 B */
    {0x7F,0x49,0x49,0x49,0x36},

    /* 0x43 C */
    {0x3E,0x41,0x41,0x41,0x22},

    /* 0x44 D */
    {0x7F,0x41,0x41,0x22,0x1C},

    /* 0x45 E */
    {0x7F,0x49,0x49,0x49,0x41},

    /* 0x46 F */
    {0x7F,0x09,0x09,0x09,0x01},

    /* 0x47 G */
    {0x3E,0x41,0x49,0x49,0x7A},

    /* 0x48 H */
    {0x7F,0x08,0x08,0x08,0x7F},

    /* 0x49 I */
    {0x00,0x41,0x7F,0x41,0x00},

    /* 0x4A J */
    {0x20,0x40,0x41,0x3F,0x01},

    /* 0x4B K */
    {0x7F,0x08,0x14,0x22,0x41},

    /* 0x4C L */
    {0x7F,0x40,0x40,0x40,0x40},

    /* 0x4D M */
    {0x7F,0x02,0x0C,0x02,0x7F},

    /* 0x4E N */
    {0x7F,0x04,0x08,0x10,0x7F},

    /* 0x4F O */
    {0x3E,0x41,0x41,0x41,0x3E},

    /* 0x50 P */
    {0x7F,0x09,0x09,0x09,0x06},

    /* 0x51 Q */
    {0x3E,0x41,0x51,0x21,0x5E},

    /* 0x52 R */
    {0x7F,0x09,0x19,0x29,0x46},

    /* 0x53 S */
    {0x46,0x49,0x49,0x49,0x31},

    /* 0x54 T */
    {0x01,0x01,0x7F,0x01,0x01},

    /* 0x55 U */
    {0x3F,0x40,0x40,0x40,0x3F},

    /* 0x56 V */
    {0x1F,0x20,0x40,0x20,0x1F},

    /* 0x57 W */
    {0x3F,0x40,0x38,0x40,0x3F},

    /* 0x58 X */
    {0x63,0x14,0x08,0x14,0x63},

    /* 0x59 Y */
    {0x07,0x08,0x70,0x08,0x07},

    /* 0x5A Z */
    {0x61,0x51,0x49,0x45,0x43},


    /* 0x5B [ */
    {0x00,0x7F,0x41,0x41,0x00},

    /* 0x5C \ */
    {0x02,0x04,0x08,0x10,0x20},

    /* 0x5D ] */
    {0x00,0x41,0x41,0x7F,0x00},

    /* 0x5E ^ */
    {0x04,0x02,0x01,0x02,0x04},

    /* 0x5F _ */
    {0x40,0x40,0x40,0x40,0x40},

    /* 0x60 ` */
    {0x00,0x01,0x02,0x04,0x00},


    /* 0x61 a */
    {0x20,0x54,0x54,0x54,0x78},

    /* 0x62 b */
    {0x7F,0x48,0x44,0x44,0x38},

    /* 0x63 c */
    {0x38,0x44,0x44,0x44,0x20},

    /* 0x64 d */
    {0x38,0x44,0x44,0x48,0x7F},

    /* 0x65 e */
    {0x38,0x54,0x54,0x54,0x18},

    /* 0x66 f */
    {0x08,0x7E,0x09,0x01,0x02},

    /* 0x67 g */
    {0x0C,0x52,0x52,0x52,0x3E},

    /* 0x68 h */
    {0x7F,0x08,0x04,0x04,0x78},

    /* 0x69 i */
    {0x00,0x44,0x7D,0x40,0x00},

    /* 0x6A j */
    {0x20,0x40,0x44,0x3D,0x00},

    /* 0x6B k */
    {0x7F,0x10,0x28,0x44,0x00},

    /* 0x6C l */
    {0x00,0x41,0x7F,0x40,0x00},

    /* 0x6D m */
    {0x7C,0x04,0x18,0x04,0x78},

    /* 0x6E n */
    {0x7C,0x08,0x04,0x04,0x78},

    /* 0x6F o */
    {0x38,0x44,0x44,0x44,0x38},

    /* 0x70 p */
    {0x7C,0x14,0x14,0x14,0x08},

    /* 0x71 q */
    {0x08,0x14,0x14,0x18,0x7C},

    /* 0x72 r */
    {0x7C,0x08,0x04,0x04,0x08},

    /* 0x73 s */
    {0x48,0x54,0x54,0x54,0x20},

    /* 0x74 t */
    {0x04,0x3F,0x44,0x40,0x20},

    /* 0x75 u */
    {0x3C,0x40,0x40,0x20,0x7C},

    /* 0x76 v */
    {0x1C,0x20,0x40,0x20,0x1C},

    /* 0x77 w */
    {0x3C,0x40,0x30,0x40,0x3C},

    /* 0x78 x */
    {0x44,0x28,0x10,0x28,0x44},

    /* 0x79 y */
    {0x0C,0x50,0x50,0x50,0x3C},

    /* 0x7A z */
    {0x44,0x64,0x54,0x4C,0x44},


    /* 0x7B { */
    {0x00,0x08,0x36,0x41,0x00},

    /* 0x7C | */
    {0x00,0x00,0x7F,0x00,0x00},

    /* 0x7D } */
    {0x00,0x41,0x36,0x08,0x00},

    /* 0x7E ~ */
    {0x08,0x04,0x08,0x10,0x08}
};

/* =========================================================
   8x16 FONT

   Character size:
       8 pixels wide
       16 pixels high , but only 12 pixels used rest 4 pixels empty

   Storage:
       16 bytes per character

   First 8 bytes  = upper 8 pixels
   Next  8 bytes  = lower 8 pixels

   This example contains:
       SPACE
       0-9
       A-Z
       a-z

   You can add punctuation later.
   ========================================================= */

const uint8_t font16[][16] PROGMEM =
{
    /* SPACE */
    {
        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,

        0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    },

    /* 0 */
    {
        0x00,0xF8,0xFC,0x06,
        0x02,0x06,0xFC,0xF8,

        0x00,0x07,0x0F,0x0C,
        0x08,0x0C,0x0F,0x07
    },

    /* 1 */
    {
        0x00,0x00,0x0C,0x0E,
        0xFE,0xFE,0x00,0x00,

        0x00,0x00,0x0C,0x0C,
        0x0F,0x0F,0x0C,0x00
    },

/* 2 */
{	

	0b00000000,
	0b00001000,
	0b10001100,
	0b11000110,
	0b01100110,
	0b00111100,
	0b00011000,
	0b00000000,

	0b00001110,
	0b00001111,
	0b00001101,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00001100,
	0b00000000
},


	/* 3 */
	{
		0b00000000,
		0b00001000,
		0b11001100,
		0b11000110,
		0b11000110,
		0b11000110,
		0b11111100,
		0b01111000,

		0b00000000,
		0b00000110,
		0b00001100,
		0b00001100,
		0b00001100,
		0b00001100,
		0b00001111,
		0b00000111
	},


    /* 4 */
    {
        0x00,0xC0,0xF0,0x3C,
        0xFE,0xFE,0x00,0x00,

        0x03,0x03,0x02,0x02,
        0x0F,0x0F,0x02,0x02
    },

    /* 5 */
    {
        0b00000000,
        0b00011110,
        0b00011110,
        0b00010010,
        0b00010010,
        0b11110010,
        0b11100010,
        0b00000000,

        0b00000000,
        0b00000010,
        0b00000110,
        0b00001100,
        0b00001100,
        0b00000111,
        0b00000011,
        0b00000000
    },

    /* 6 */
    {
        0b00000000,
        0b11111000,
        0b11111100,
        0b01000110,
        0b01000110,
        0b11001100,
        0b10001000,
        0b00000000,

        0b00000000,
        0b00000111,
        0b00001111,
        0b00001000,
        0b00001000,
        0b00001111,
        0b00000111,
        0b00000000
    },

    /* 7 */
    {
        0x06,0x06,0x02,0x02,
        0xC2,0xF2,0x3E,0x0E,

        0x00,0x00,0x00,0x0F,
        0x0F,0x00,0x00,0x00
    },

    /* 8 */
    {
        0x7C,0xFE,0x86,0x82,
        0x82,0x86,0xFE,0x7C,

        0x07,0x0F,0x0C,0x08,
        0x08,0x0C,0x0F,0x07
    },

    /* 9 */
    {
        0x7C,0xFE,0x86,0x82,
        0x82,0x86,0xFC,0xF8,

        0x00,0x08,0x08,0x08,
        0x0C,0x0F,0x07,0x03
    },


    /* A */
    {
        0x00,0xF0,0xFC,0x0E,
        0x06,0x0E,0xFC,0xF0,

        0x0F,0x0F,0x01,0x01,
        0x01,0x01,0x0F,0x0F
    },

	/* B */
	{
		0xFE,0xFE,0x82,0x82,
		0x82,0xC6,0x7C,0x38,

		0x0F,0x0F,0x08,0x08,
		0x08,0x0C,0x0F,0x03
	},

    /* C */
    {
        0xF8,0xFC,0x06,0x02,
        0x02,0x06,0x0E,0x1C,

        0x07,0x0F,0x0C,0x08,
        0x08,0x0C,0x0E,0x06
    },

    /* D */
    {
        0xFE,0xFE,0x02,0x02,
        0x06,0x0E,0xFC,0xF8,

        0x0F,0x0F,0x08,0x08,
        0x0C,0x0E,0x0F,0x07
    },

    /* E */
    {
	    0xFE,0xFE,0x82,0x82,
	    0x82,0x82,0x82,0x82,

	    0x0F,0x0F,0x08,0x08,
	    0x08,0x08,0b00001000,0b00001000
    },

    /* F */
    {
        0xFE,0xFE,0x82,0x82,
        0x82,0x82,0x02,0x02,

        0x0F,0x0F,0x00,0x00,
        0x00,0x00,0x00,0x00
    },

    /* G */
    {
        0xF8,0xFC,0x06,0x02,
        0x82,0x86,0x8E,0x9C,

        0x07,0x0F,0x0C,0x08,
        0x09,0x0F,0x0F,0x07
    },

    /* H */
    {
        0xFE,0xFE,0x80,0x80,
        0x80,0x80,0xFE,0xFE,

        0x0F,0x0F,0x00,0x00,
        0x00,0x00,0x0F,0x0F
    },

    /* I */
    {
        0x02,0x02,0x02,0xFE,
        0xFE,0x02,0x02,0x02,

        0x08,0x08,0x08,0x0F,
        0x0F,0x08,0x08,0x08
    },

    /* J */
    {
        0x00,0x00,0x02,0x02,
        0x02,0x02,0xFE,0xFC,

        0x06,0x0E,0x0C,0x08,
        0x08,0x0C,0x0F,0x07
    },

    /* K */
    {
        0xFE,0xFE,0x80,0xC0,
        0xE0,0x70,0x1E,0x0E,

        0x0F,0x0F,0x00,0x01,
        0x03,0x07,0x0E,0x0C
    },

    /* L */
    {
        0xFE,0xFE,0x00,0x00,
        0x00,0x00,0x00,0x00,

        0x0F,0x0F,0x08,0x08,
        0x08,0x08,0x08,0x08
    },

    /* M */
    {
        0xFE,0xFE,0x1C,0xF0,
        0x1C,0xFE,0xFE,0x00,

        0x0F,0x0F,0x00,0x03,
        0x00,0x0F,0x0F,0x00
    },

    /* N */
    {
        0xFE,0xFE,0x38,0x70,
        0xE0,0xC0,0xFE,0xFE,

        0x0F,0x0F,0x00,0x00,
        0x00,0x01,0x0F,0x0F
    },

    /* O */
    {
        0xF8,0xFC,0x06,0x02,
        0x02,0x06,0xFC,0xF8,

        0x07,0x0F,0x0C,0x08,
        0x08,0x0C,0x0F,0x07
    },

    /* P */
    {
        0xFE,0xFE,0x82,0x82,
        0x82,0x86,0xFC,0x78,

        0x0F,0x0F,0x00,0x00,
        0x00,0x00,0x00,0x00
    },

    /* Q */
    {
        0xF8,0xFC,0x06,0x02,
        0x02,0x06,0xFC,0xF8,

        0x07,0x0F,0x0C,0x08,
        0x0E,0x0F,0x07,0x0F
    },

    /* R */
    {
        0xFE,0xFE,0x82,0x82,
        0x82,0xC6,0xFC,0x78,

        0x0F,0x0F,0x00,0x01,
        0x03,0x06,0x0C,0x08
    },

    /* S */
    {
	    0x7C,0xFE,0x86,0x82,
	    0x82,0x86,0x0E,0x0C,

	    0x06,0x0C,0x0C,0x08,
	    0x08,0x0C,0x0F,0x07
    },

    /* T */
    {
        0x02,0x02,0x02,0xFE,
        0xFE,0x02,0x02,0x02,

        0x00,0x00,0x00,0x0F,
        0x0F,0x00,0x00,0x00
    },

    /* U */
    {
        0xFE,0xFE,0x00,0x00,
        0x00,0x00,0xFE,0xFE,

        0x07,0x0F,0x0C,0x08,
        0x08,0x0C,0x0F,0x07
    },

    /* V */
    {
        0x1E,0x7E,0xE0,0x80,
        0x80,0xE0,0x7E,0x1E,

        0x00,0x00,0x03,0x0F,
        0x0F,0x03,0x00,0x00
    },

    /* W */
    {
        0xFE,0xFE,0x00,0xF0,
        0xF0,0x00,0xFE,0xFE,

        0x01,0x07,0x0F,0x03,
        0x03,0x0F,0x07,0x01
    },

    /* X */
    {
        0x0E,0x3E,0xF0,0xC0,
        0xC0,0xF0,0x3E,0x0E,

        0x0C,0x0F,0x03,0x00,
        0x00,0x03,0x0F,0x0C
    },

    /* Y */
    {
        0x06,0x1E,0xF8,0xE0,
        0xE0,0xF8,0x1E,0x06,

        0x00,0x00,0x00,0x0F,
        0x0F,0x00,0x00,0x00
    },

    /* Z */
    {
        0x06,0x02,0x02,0x82,
        0xC2,0xE2,0x3E,0x1E,

        0x0E,0x0F,0x0D,0x0C,
        0x0C,0x0C,0b1100,0b1110
    },
	
	// battery icon
	{
		0x18,0x3C,0x7E,0x42,
		0x42,0x42,0x7E,0x00,

		0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x18
	},

};


/* =========================================================
   FONT INDEX
   ========================================================= */

uint8_t Font16Index(char c)
{
    if (c == ' ')
        return 0;

    if (c >= '0' && c <= '9')
        return (c - '0') + 1;

    if (c >= 'A' && c <= 'Z')
        return (c - 'A') + 11;

    /*
     * Lowercase can use uppercase glyph.
     */
    if (c >= 'a' && c <= 'z')
        return (c - 'a') + 11;

    return 0;
}


/* =========================================================
   DRAW 16 PIXEL HIGH CHARACTER
   ========================================================= */

void GLCD_Char16(
    uint8_t page,
    uint8_t column,
    char c
)
{
    uint8_t index;
    uint8_t x;
    uint8_t top;
    uint8_t bottom;

    /*
     * Need two pages:
     *
     * page     = upper 8 pixels
     * page + 1 = lower 8 pixels
     */
    if (page > 6)
        return;

    if (column >= 128)
        return;

    index = Font16Index(c);

    for (x = 0; x < 8; x++)
    {
        if ((uint16_t)column + x >= 128)
            break;

        /*
         * Upper 8 pixels
         */
        top = pgm_read_byte(
            &font16[index][x]
        );

        /*
         * Lower 8 pixels
         */
        bottom = pgm_read_byte(
            &font16[index][8 + x]
        );

        GLCD_WriteByteAt(
            page,
            column + x,
            top
        );

        GLCD_WriteByteAt(
            page + 1,
            column + x,
            bottom
        );
    }

    /*
     * One blank column between characters.
     */
    if ((uint16_t)column + 8 < 128)
    {
        GLCD_WriteByteAt(
            page,
            column + 8,
            0x00
        );

        GLCD_WriteByteAt(
            page + 1,
            column + 8,
            0x00
        );
    }
}


/* =========================================================
   DRAW 16 PIXEL HIGH STRING
   ========================================================= */

void GLCD_String16(
    uint8_t page,
    uint8_t column,
    const char *str
)
{
    while (*str)
    {
        if (page > 6)
            break;

        if (column >= 128)
            break;

        GLCD_Char16(
            page,
            column,
            *str
        );

        /*
         * 8 pixel character
         * + 1 pixel spacing
         */
        column += 9;

        str++;
    }
}



/* =========================================================
   GET FONT INDEX
   ========================================================= */

uint8_t FontIndex(char c)
{
    /*
     * Printable ASCII range:
     *
     * 0x20 = SPACE
     * 0x7E = ~
     *
     * Font table is arranged in exactly this order.
     */

    if ((uint8_t)c >= 0x20 && (uint8_t)c <= 0x7E)
        return (uint8_t)c - 0x20;

    /*
     * Unsupported / non-printable character.
     * Show as SPACE.
     */
    return 0;
}


/* =========================================================
   CHIP SELECT
   ========================================================= */

void GLCD_SelectChip(uint8_t chip)
{
    /*
     * Disable both.
     */
    CTRL_PORT_B &= ~((1 << CS1) | (1 << CS2));

    /*
     * Select requested chip.
     */
    if (chip == 1)
        CTRL_PORT_B |= (1 << CS1);
    else
        CTRL_PORT_B |= (1 << CS2);

    _delay_us(2);
}


/* =========================================================
   ENABLE PULSE
   ========================================================= */

void GLCD_Enable_Pulse(void)
{
    CTRL_PORT_C |= (1 << EN);

    _delay_us(7);

    CTRL_PORT_C &= ~(1 << EN);

    _delay_us(7);
}


/* =========================================================
   COMMAND
   ========================================================= */

void GLCD_Command(uint8_t cmd, uint8_t chip)
{
    GLCD_SelectChip(chip);

    /*
     * RS = 0
     * R/W = 0
     */
    CTRL_PORT_C &= ~(1 << RS);
    CTRL_PORT_C &= ~(1 << RW);

    DATA_PORT = cmd;

    _delay_us(2);

    GLCD_Enable_Pulse();
}


/* =========================================================
   DATA
   ========================================================= */

void GLCD_Data(uint8_t data)
{
    /*
     * RS = 1
     * R/W = 0
     */

    CTRL_PORT_C |= (1 << RS);
    CTRL_PORT_C &= ~(1 << RW);

    DATA_PORT = data;

    _delay_us(2);

    GLCD_Enable_Pulse();
}


/* =========================================================
   SET PAGE/COLUMN
   ========================================================= */

void GLCD_SetXY(uint8_t page, uint8_t column, uint8_t chip)
{
    if (page > 7)
        page = 0;

    if (column > 63)
        column = 0;

    GLCD_Command(0xB8 | page, chip);
    GLCD_Command(0x40 | column, chip);
}


/* =========================================================
   DISPLAY ON
   ========================================================= */

void GLCD_DisplayOn(uint8_t chip)
{
    GLCD_Command(0x3F, chip);
}


/* =========================================================
   CLEAR CHIP
   ========================================================= */

void GLCD_ClearChip(uint8_t chip)
{
    uint8_t page;
    uint8_t col;

    for (page = 0; page < 8; page++)
    {
        GLCD_SetXY(page, 0, chip);

        for (col = 0; col < 64; col++)
        {
            GLCD_Data(0x00);
        }
    }
}


/* =========================================================
   CLEAR DISPLAY
   ========================================================= */

void GLCD_Clear(void)
{
    GLCD_ClearChip(1);
    GLCD_ClearChip(2);
}


/* =========================================================
   GPIO INIT
   ========================================================= */

void GLCD_GPIO_Init(void)
{
    /*
     * DB0-DB7
     */
    DATA_DDR = 0xFF;

    /*
     * RS/RW/E
     */
    CTRL_DDR_C |=
        (1 << RS) |
        (1 << RW) |
        (1 << EN);

    /*
     * CS1/CS2
     */
    CTRL_DDR_B |=
        (1 << CS1) |
        (1 << CS2)|
		(1 << GLCD_rst);

    /*
     * Initial states
     */

    CTRL_PORT_C &= ~(1 << RS);
    CTRL_PORT_C &= ~(1 << RW);
    CTRL_PORT_C &= ~(1 << EN);

    CTRL_PORT_B &= ~((1 << CS1) | (1 << CS2));
	CTRL_PORT_B |= (1<<GLCD_rst);

    DATA_PORT = 0x00;
}


/* =========================================================
   GLCD INIT
   ========================================================= */

void GLCD_Init(void)
{
    GLCD_GPIO_Init();

    _delay_ms(50);

    /* Explicitly synchronize both controllers */
    GLCD_Command(0xC0, 1);
    GLCD_Command(0xC0, 2);

    /* Display ON */
    GLCD_DisplayOn(1);
    GLCD_DisplayOn(2);

    _delay_ms(10);

    GLCD_Clear();

    _delay_ms(10);
}


/* =========================================================
   WRITE ONE BYTE USING GLOBAL 0..127 COLUMN
   ========================================================= */

void GLCD_WriteByteAt(
    uint8_t row,
    uint8_t column,
    uint8_t data
)
{
    uint8_t chip;
    uint8_t localColumn;

    if (row >= 8 || column >= 128)
        return;

    /*
     * Convert global column to controller + local column.
     */

    if (column < 64)
    {
        chip = 1;
        localColumn = column;
    }
    else
    {
        chip = 2;
        localColumn = column - 64;
    }

    /*
     * Select exact position.
     */
    GLCD_SetXY(row, localColumn, chip);

    /*
     * Write data.
     */
    GLCD_Data(data);
}


/* =========================================================
   DRAW ONE CHARACTER
   ========================================================= */

void GLCD_Char(
    uint8_t page,
    uint8_t column,
    char c
)
{
    uint8_t index;
    uint8_t i;
    uint8_t data;

    index = FontIndex(c);

    /*
     * 5 font columns + 1 blank spacing column.
     *
     * Character width = 6 pixels.
     *
     * Global column range = 0..127.
     *
     * The function safely handles:
     *
     *     CS1 column 63
     *             |
     *             V
     *     CS2 column 0
     */

    for (i = 0; i < 5; i++)
    {
        data = pgm_read_byte(&font5x7[index][i]);

        /*
         * Do not write outside display.
         */
        if ((uint16_t)column + i < 128)
        {
            GLCD_WriteByteAt(
                page,
                column + i,
                data
            );
        }
    }

    /*
     * Blank spacing column.
     */
    if ((uint16_t)column + 5 < 128)
    {
        GLCD_WriteByteAt(
            page,
            column + 5,
            0x00
        );
    }
}


/* =========================================================
   DRAW STRING
   ========================================================= */

void GLCD_String8(
    uint8_t page,
    uint8_t column,
    const char *str
)
{
    while (*str)
    {
        /*
         * Stop if starting position is outside display.
         */
        if (column >= 128)
            break;

        /*
         * Draw character.
         */
        GLCD_Char(
            page,
            column,
            *str
        );

        /*
         * Character width:
         *
         * 5 pixels font
         * + 1 pixel spacing
         *
         * = 6 pixels
         */
        column += 6;

        str++;
    }
}


/* =========================================================
   SYNCHRONIZE CONTROLLERS
   ========================================================= */

void GLCD_SyncControllers(void)
{
    GLCD_Command(0xC0, 1);
    GLCD_Command(0xC0, 2);

    GLCD_DisplayOn(1);
    GLCD_DisplayOn(2);
}


//---no signal--
    const uint8_t textWidth = 81;
    const uint8_t y = 24;
	int16_t x = 0;
	int8_t dx = 1;

void No_Signal(){
	/*
         * Clear previous position.
         */
        GLCD_Clear();
        /*
         * Draw NO SIGNAL.
         */
        GLCD_String16(
            y / 8,
            x,
            "NO SIGNAL"
        );

        /*
         * Move horizontally.
         */
        x += dx;

        /*
         * Bounce at left edge.
         */
        if (x <= 0)
        {
            x = 0;
            dx = 1;
        }

        /*
         * Bounce at right edge.
         */
        if (x >= (128 - textWidth))
        {
            x = 128 - textWidth;
            dx = -1;
        }

        /*
         * Slower, smoother movement.
         */
        _delay_ms(80);
}
//------------------------------------------------
void GLCD_String_Page(uint8_t page, uint8_t column, PGM_P str)
{
	char c;

	while ((c = pgm_read_byte(str++)) != '\0')
	{
		GLCD_Char(page, column, c);

		column += 6;   // Adjust according to your font width

		if (column >= 128)
		break;
	}
}

/*------information-------------
[2]   ? 2 screens
[8]   ? 8 rows per screen
[100] ? maximum 99 characters + '\0' per row
*/

const char my_text[4][8][100] PROGMEM =
{
	/* ================= SCREEN 0 ================= */
	{
		"GLCD interfacing with",   // row 0
		"   ATMEGA8 MCU.  I2C ",   // row 1
		"    Address - 0x08   ",   // row 2
		" RS  PC2     SCL  PC5",   // row 3
		" RW  PC1     SDA  PC4",   // row 4
		" EN  PC0     Rst  PB2",   // row 5
		" CS1 PB1   Button PC3",   // row 6
		" CS2 PB0  Page 1/4 ->"    // row 7
	},
	/* ================= SCREEN 1 ================= */
	{
		" BackLight Pwm  PB3  ",   // row 0
		" Screen timeOut 10sec",   // row 1
		" (Icons) Battery  0  ",   // row 2
		" Heart 1     Msg  6  ",   // row 3
		" powerButton      2  ",   // row 4
		" Face             3  ",   // row 5
		" Right Arrow      4  ",   // row 6
		" Radio 5  Page 2/4 ->"    // row 7
	},
	/* ================= SCREEN 2 ================= */
	{
		"     - Commands -    ",   // row 0
		" CMD_STRING8     0x01",   // row 1
		" CMD_STRING16    0x02",   // row 2
		" CMD_CLEAR       0x03",   // row 3
		" CMD_16by16icon  0x04",   // row 4
		" CMD_GLCD_reset  0x05",   // row 5
		" GLCD_Backlight  0x06",// row 6
		"          Page 3/4 ->" // row 7
	},
	
	/* ================= SCREEN 3 ================= */
	{
		" Command format :-   ",   // row 0
		" ",   // row 1
		" Master ->   -> Slave",    // row 2
		" ",// row 3
		"msg[] Col Row CMD ->",// row 4
		"Buffer size 25 bytes",// row 5
		"",// row 6
		"  Exit    Page 4/4 ->" // row 7
	}

};

//----------------------------
void GLCD_DisplayScreen(uint8_t screen)
{
	uint8_t row;
	for (row = 0; row < 8; row++)
	{	
		GLCD_SyncControllers();
		GLCD_String_Page(
		row,
		0,
		my_text[screen][row]
		);
	}
}
//------------------------------
void GLCD_reset(){
	CTRL_PORT_B &= ~(1<<GLCD_rst);
	_delay_us(15);
	CTRL_PORT_B |= (1<<GLCD_rst);
	
}
