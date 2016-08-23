#include "PCD8544.h"
#include "global.h"
#include "common.h"


#ifdef enablePartialUpdate
static uint8_t xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;
#endif

static unsigned char  font[] = {
                0x00, 0x00, 0x00, 0x00, 0x00,
                0x3E, 0x5B, 0x4F, 0x5B, 0x3E,
                0x3E, 0x6B, 0x4F, 0x6B, 0x3E,
                0x1C, 0x3E, 0x7C, 0x3E, 0x1C,
                0x18, 0x3C, 0x7E, 0x3C, 0x18,
                0x1C, 0x57, 0x7D, 0x57, 0x1C,
                0x1C, 0x5E, 0x7F, 0x5E, 0x1C,
                0x00, 0x18, 0x3C, 0x18, 0x00,
                0xFF, 0xE7, 0xC3, 0xE7, 0xFF,
                0x00, 0x18, 0x24, 0x18, 0x00,
                0xFF, 0xE7, 0xDB, 0xE7, 0xFF,
                0x30, 0x48, 0x3A, 0x06, 0x0E,
                0x26, 0x29, 0x79, 0x29, 0x26,
                0x40, 0x7F, 0x05, 0x05, 0x07,
                0x40, 0x7F, 0x05, 0x25, 0x3F,
                0x5A, 0x3C, 0xE7, 0x3C, 0x5A,
                0x7F, 0x3E, 0x1C, 0x1C, 0x08,
                0x08, 0x1C, 0x1C, 0x3E, 0x7F,
                0x14, 0x22, 0x7F, 0x22, 0x14,
                0x5F, 0x5F, 0x00, 0x5F, 0x5F,
                0x06, 0x09, 0x7F, 0x01, 0x7F,
                0x00, 0x66, 0x89, 0x95, 0x6A,
                0x60, 0x60, 0x60, 0x60, 0x60,
                0x94, 0xA2, 0xFF, 0xA2, 0x94,
                0x08, 0x04, 0x7E, 0x04, 0x08,
                0x10, 0x20, 0x7E, 0x20, 0x10,
                0x08, 0x08, 0x2A, 0x1C, 0x08,
                0x08, 0x1C, 0x2A, 0x08, 0x08,
                0x1E, 0x10, 0x10, 0x10, 0x10,
                0x0C, 0x1E, 0x0C, 0x1E, 0x0C,
                0x30, 0x38, 0x3E, 0x38, 0x30,
                0x06, 0x0E, 0x3E, 0x0E, 0x06,
                0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x5F, 0x00, 0x00,
                0x00, 0x07, 0x00, 0x07, 0x00,
                0x14, 0x7F, 0x14, 0x7F, 0x14,
                0x24, 0x2A, 0x7F, 0x2A, 0x12,
                0x23, 0x13, 0x08, 0x64, 0x62,
                0x36, 0x49, 0x56, 0x20, 0x50,
                0x00, 0x08, 0x07, 0x03, 0x00,
                0x00, 0x1C, 0x22, 0x41, 0x00,
                0x00, 0x41, 0x22, 0x1C, 0x00,
                0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
                0x08, 0x08, 0x3E, 0x08, 0x08,
                0x00, 0x80, 0x70, 0x30, 0x00,
                0x08, 0x08, 0x08, 0x08, 0x08,
                0x00, 0x00, 0x60, 0x60, 0x00,
                0x20, 0x10, 0x08, 0x04, 0x02,
                0x3E, 0x51, 0x49, 0x45, 0x3E,
                0x00, 0x42, 0x7F, 0x40, 0x00,
                0x72, 0x49, 0x49, 0x49, 0x46,
                0x21, 0x41, 0x49, 0x4D, 0x33,
                0x18, 0x14, 0x12, 0x7F, 0x10,
                0x27, 0x45, 0x45, 0x45, 0x39,
                0x3C, 0x4A, 0x49, 0x49, 0x31,
                0x41, 0x21, 0x11, 0x09, 0x07,
                0x36, 0x49, 0x49, 0x49, 0x36,
                0x46, 0x49, 0x49, 0x29, 0x1E,
                0x00, 0x00, 0x14, 0x00, 0x00,
                0x00, 0x40, 0x34, 0x00, 0x00,
                0x00, 0x08, 0x14, 0x22, 0x41,
                0x14, 0x14, 0x14, 0x14, 0x14,
                0x00, 0x41, 0x22, 0x14, 0x08,
                0x02, 0x01, 0x59, 0x09, 0x06,
                0x3E, 0x41, 0x5D, 0x59, 0x4E,
                0x7C, 0x12, 0x11, 0x12, 0x7C,
                0x7F, 0x49, 0x49, 0x49, 0x36,
                0x3E, 0x41, 0x41, 0x41, 0x22,
                0x7F, 0x41, 0x41, 0x41, 0x3E,
                0x7F, 0x49, 0x49, 0x49, 0x41,
                0x7F, 0x09, 0x09, 0x09, 0x01,
                0x3E, 0x41, 0x41, 0x51, 0x73,
                0x7F, 0x08, 0x08, 0x08, 0x7F,
                0x00, 0x41, 0x7F, 0x41, 0x00,
                0x20, 0x40, 0x41, 0x3F, 0x01,
                0x7F, 0x08, 0x14, 0x22, 0x41,
                0x7F, 0x40, 0x40, 0x40, 0x40,
                0x7F, 0x02, 0x1C, 0x02, 0x7F,
                0x7F, 0x04, 0x08, 0x10, 0x7F,
                0x3E, 0x41, 0x41, 0x41, 0x3E,
                0x7F, 0x09, 0x09, 0x09, 0x06,
                0x3E, 0x41, 0x51, 0x21, 0x5E,
                0x7F, 0x09, 0x19, 0x29, 0x46,
                0x26, 0x49, 0x49, 0x49, 0x32,
                0x03, 0x01, 0x7F, 0x01, 0x03,
                0x3F, 0x40, 0x40, 0x40, 0x3F,
                0x1F, 0x20, 0x40, 0x20, 0x1F,
                0x3F, 0x40, 0x38, 0x40, 0x3F,
                0x63, 0x14, 0x08, 0x14, 0x63,
                0x03, 0x04, 0x78, 0x04, 0x03,
                0x61, 0x59, 0x49, 0x4D, 0x43,
                0x00, 0x7F, 0x41, 0x41, 0x41,
                0x02, 0x04, 0x08, 0x10, 0x20,
                0x00, 0x41, 0x41, 0x41, 0x7F,
                0x04, 0x02, 0x01, 0x02, 0x04,
                0x40, 0x40, 0x40, 0x40, 0x40,
                0x00, 0x03, 0x07, 0x08, 0x00,
                0x20, 0x54, 0x54, 0x78, 0x40,
                0x7F, 0x28, 0x44, 0x44, 0x38,
                0x38, 0x44, 0x44, 0x44, 0x28,
                0x38, 0x44, 0x44, 0x28, 0x7F,
                0x38, 0x54, 0x54, 0x54, 0x18,
                0x00, 0x08, 0x7E, 0x09, 0x02,
                0x18, 0xA4, 0xA4, 0x9C, 0x78,
                0x7F, 0x08, 0x04, 0x04, 0x78,
                0x00, 0x44, 0x7D, 0x40, 0x00,
                0x20, 0x40, 0x40, 0x3D, 0x00,
                0x7F, 0x10, 0x28, 0x44, 0x00,
                0x00, 0x41, 0x7F, 0x40, 0x00,
                0x7C, 0x04, 0x78, 0x04, 0x78,
                0x7C, 0x08, 0x04, 0x04, 0x78,
                0x38, 0x44, 0x44, 0x44, 0x38,
                0xFC, 0x18, 0x24, 0x24, 0x18,
                0x18, 0x24, 0x24, 0x18, 0xFC,
                0x7C, 0x08, 0x04, 0x04, 0x08,
                0x48, 0x54, 0x54, 0x54, 0x24,
                0x04, 0x04, 0x3F, 0x44, 0x24,
                0x3C, 0x40, 0x40, 0x20, 0x7C,
                0x1C, 0x20, 0x40, 0x20, 0x1C,
                0x3C, 0x40, 0x30, 0x40, 0x3C,
                0x44, 0x28, 0x10, 0x28, 0x44,
                0x4C, 0x90, 0x90, 0x90, 0x7C,
                0x44, 0x64, 0x54, 0x4C, 0x44,
                0x00, 0x08, 0x36, 0x41, 0x00,
                0x00, 0x00, 0x77, 0x00, 0x00,
                0x00, 0x41, 0x36, 0x08, 0x00,
                0x02, 0x01, 0x02, 0x04, 0x02,
                0x3C, 0x26, 0x23, 0x26, 0x3C,
                0x1E, 0xA1, 0xA1, 0x61, 0x12,
                0x3A, 0x40, 0x40, 0x20, 0x7A,
                0x38, 0x54, 0x54, 0x55, 0x59,
                0x21, 0x55, 0x55, 0x79, 0x41,
                0x21, 0x54, 0x54, 0x78, 0x41,
                0x21, 0x55, 0x54, 0x78, 0x40,
                0x20, 0x54, 0x55, 0x79, 0x40,
                0x0C, 0x1E, 0x52, 0x72, 0x12,
                0x39, 0x55, 0x55, 0x55, 0x59,
                0x39, 0x54, 0x54, 0x54, 0x59,
                0x39, 0x55, 0x54, 0x54, 0x58,
                0x00, 0x00, 0x45, 0x7C, 0x41,
                0x00, 0x02, 0x45, 0x7D, 0x42,
                0x00, 0x01, 0x45, 0x7C, 0x40,
                0xF0, 0x29, 0x24, 0x29, 0xF0,
                0xF0, 0x28, 0x25, 0x28, 0xF0,
                0x7C, 0x54, 0x55, 0x45, 0x00,
                0x20, 0x54, 0x54, 0x7C, 0x54,
                0x7C, 0x0A, 0x09, 0x7F, 0x49,
                0x32, 0x49, 0x49, 0x49, 0x32,
                0x32, 0x48, 0x48, 0x48, 0x32,
                0x32, 0x4A, 0x48, 0x48, 0x30,
                0x3A, 0x41, 0x41, 0x21, 0x7A,
                0x3A, 0x42, 0x40, 0x20, 0x78,
                0x00, 0x9D, 0xA0, 0xA0, 0x7D,
                0x39, 0x44, 0x44, 0x44, 0x39,
                0x3D, 0x40, 0x40, 0x40, 0x3D,
                0x3C, 0x24, 0xFF, 0x24, 0x24,
                0x48, 0x7E, 0x49, 0x43, 0x66,
                0x2B, 0x2F, 0xFC, 0x2F, 0x2B,
                0xFF, 0x09, 0x29, 0xF6, 0x20,
                0xC0, 0x88, 0x7E, 0x09, 0x03,
                0x20, 0x54, 0x54, 0x79, 0x41,
                0x00, 0x00, 0x44, 0x7D, 0x41,
                0x30, 0x48, 0x48, 0x4A, 0x32,
                0x38, 0x40, 0x40, 0x22, 0x7A,
                0x00, 0x7A, 0x0A, 0x0A, 0x72,
                0x7D, 0x0D, 0x19, 0x31, 0x7D,
                0x26, 0x29, 0x29, 0x2F, 0x28,
                0x26, 0x29, 0x29, 0x29, 0x26,
                0x30, 0x48, 0x4D, 0x40, 0x20,
                0x38, 0x08, 0x08, 0x08, 0x08,
                0x08, 0x08, 0x08, 0x08, 0x38,
                0x2F, 0x10, 0xC8, 0xAC, 0xBA,
                0x2F, 0x10, 0x28, 0x34, 0xFA,
                0x00, 0x00, 0x7B, 0x00, 0x00,
                0x08, 0x14, 0x2A, 0x14, 0x22,
                0x22, 0x14, 0x2A, 0x14, 0x08,
                0xAA, 0x00, 0x55, 0x00, 0xAA,
                0xAA, 0x55, 0xAA, 0x55, 0xAA,
                0x00, 0x00, 0x00, 0xFF, 0x00,
                0x10, 0x10, 0x10, 0xFF, 0x00,
                0x14, 0x14, 0x14, 0xFF, 0x00,
                0x10, 0x10, 0xFF, 0x00, 0xFF,
                0x10, 0x10, 0xF0, 0x10, 0xF0,
                0x14, 0x14, 0x14, 0xFC, 0x00,
                0x14, 0x14, 0xF7, 0x00, 0xFF,
                0x00, 0x00, 0xFF, 0x00, 0xFF,
                0x14, 0x14, 0xF4, 0x04, 0xFC,
                0x14, 0x14, 0x17, 0x10, 0x1F,
                0x10, 0x10, 0x1F, 0x10, 0x1F,
                0x14, 0x14, 0x14, 0x1F, 0x00,
                0x10, 0x10, 0x10, 0xF0, 0x00,
                0x00, 0x00, 0x00, 0x1F, 0x10,
                0x10, 0x10, 0x10, 0x1F, 0x10,
                0x10, 0x10, 0x10, 0xF0, 0x10,
                0x00, 0x00, 0x00, 0xFF, 0x10,
                0x10, 0x10, 0x10, 0x10, 0x10,
                0x10, 0x10, 0x10, 0xFF, 0x10,
                0x00, 0x00, 0x00, 0xFF, 0x14,
                0x00, 0x00, 0xFF, 0x00, 0xFF,
                0x00, 0x00, 0x1F, 0x10, 0x17,
                0x00, 0x00, 0xFC, 0x04, 0xF4,
                0x14, 0x14, 0x17, 0x10, 0x17,
                0x14, 0x14, 0xF4, 0x04, 0xF4,
                0x00, 0x00, 0xFF, 0x00, 0xF7,
                0x14, 0x14, 0x14, 0x14, 0x14,
                0x14, 0x14, 0xF7, 0x00, 0xF7,
                0x14, 0x14, 0x14, 0x17, 0x14,
                0x10, 0x10, 0x1F, 0x10, 0x1F,
                0x14, 0x14, 0x14, 0xF4, 0x14,
                0x10, 0x10, 0xF0, 0x10, 0xF0,
                0x00, 0x00, 0x1F, 0x10, 0x1F,
                0x00, 0x00, 0x00, 0x1F, 0x14,
                0x00, 0x00, 0x00, 0xFC, 0x14,
                0x00, 0x00, 0xF0, 0x10, 0xF0,
                0x10, 0x10, 0xFF, 0x10, 0xFF,
                0x14, 0x14, 0x14, 0xFF, 0x14,
                0x10, 0x10, 0x10, 0x1F, 0x00,
                0x00, 0x00, 0x00, 0xF0, 0x10,
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
                0xFF, 0xFF, 0xFF, 0x00, 0x00,
                0x00, 0x00, 0x00, 0xFF, 0xFF,
                0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
                0x38, 0x44, 0x44, 0x38, 0x44,
                0x7C, 0x2A, 0x2A, 0x3E, 0x14,
                0x7E, 0x02, 0x02, 0x06, 0x06,
                0x02, 0x7E, 0x02, 0x7E, 0x02,
                0x63, 0x55, 0x49, 0x41, 0x63,
                0x38, 0x44, 0x44, 0x3C, 0x04,
                0x40, 0x7E, 0x20, 0x1E, 0x20,
                0x06, 0x02, 0x7E, 0x02, 0x02,
                0x99, 0xA5, 0xE7, 0xA5, 0x99,
                0x1C, 0x2A, 0x49, 0x2A, 0x1C,
                0x4C, 0x72, 0x01, 0x72, 0x4C,
                0x30, 0x4A, 0x4D, 0x4D, 0x30,
                0x30, 0x48, 0x78, 0x48, 0x30,
                0xBC, 0x62, 0x5A, 0x46, 0x3D,
                0x3E, 0x49, 0x49, 0x49, 0x00,
                0x7E, 0x01, 0x01, 0x01, 0x7E,
                0x2A, 0x2A, 0x2A, 0x2A, 0x2A,
                0x44, 0x44, 0x5F, 0x44, 0x44,
                0x40, 0x51, 0x4A, 0x44, 0x40,
                0x40, 0x44, 0x4A, 0x51, 0x40,
                0x00, 0x00, 0xFF, 0x01, 0x03,
                0xE0, 0x80, 0xFF, 0x00, 0x00,
                0x08, 0x08, 0x6B, 0x6B, 0x08,
                0x36, 0x12, 0x36, 0x24, 0x36,
                0x06, 0x0F, 0x09, 0x0F, 0x06,
                0x00, 0x00, 0x18, 0x18, 0x00,
                0x00, 0x00, 0x10, 0x10, 0x00,
                0x30, 0x40, 0xFF, 0x01, 0x01,
                0x00, 0x1F, 0x01, 0x01, 0x1E,
                0x00, 0x19, 0x1D, 0x17, 0x12,
                0x00, 0x3C, 0x3C, 0x3C, 0x3C,
                0x00, 0x00, 0x00, 0x00, 0x00,
};

void digitalWrite(int8_t pin, int8_t val)
 {
   if(val == 1)
    bcm2835_gpio_set(pin);
   else if(val == 0)
    bcm2835_gpio_clr(pin);
 }


void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
        uint8_t i;
        volatile uint32_t j;

        for (i = 0; i < 8; i++)  {
                if (bitOrder == LSBFIRST)
                        digitalWrite(dataPin, !!(val & (1 << i)));
                else
                        digitalWrite(dataPin, !!(val & (1 << (7 - i))));

                digitalWrite(clockPin, HIGH);
                //usleep(1);
                for (j = CLKCONST_3; j > 0; j--); // clock speed, anyone? (LCD Max CLK input: 4MHz)
                digitalWrite(clockPin, LOW);
        }
}

void LCDspiwrite(uint8_t c)
{
        shiftOut(_din, _sclk, MSBFIRST, c);
}

void LCDcommand(uint8_t c)
{
        usleep(2);
        digitalWrite( _dc, LOW);
        LCDspiwrite(c);
}

void LCDdata(uint8_t c)
{
        digitalWrite(_dc, HIGH);
        LCDspiwrite(c);
}

static void updateBoundingBox(uint8_t xmin, uint8_t ymin, uint8_t xmax, uint8_t ymax) {
#ifdef enablePartialUpdate
        if (xmin < xUpdateMin) xUpdateMin = xmin;
        if (xmax > xUpdateMax) xUpdateMax = xmax;
        if (ymin < yUpdateMin) yUpdateMin = ymin;
        if (ymax > yUpdateMax) yUpdateMax = ymax;
#endif
}


static void my_setpixel(uint8_t x, uint8_t y, uint8_t color)
{
        if ((x >= LCDWIDTH) || (y >= LCDHEIGHT))
                return;
        // x is which column
        if (color)
                pcd8544_buffer[x+ (y/8)*LCDWIDTH] |= _BV(y%8);
        else
                pcd8544_buffer[x+ (y/8)*LCDWIDTH] &= ~_BV(y%8);
}


void LCDdrawchar(uint8_t x, uint8_t y, char c, uint8_t inverted)
{
        if (y >= LCDHEIGHT) return;
        if ((x+5) >= LCDWIDTH) return;
        uint8_t i,j;
        for ( i =0; i<5; i++ )
        {
                uint8_t d = *(font+(c*5)+i);

                uint8_t j;

                for (j = 0; j<8; j++)
                {
                    if(inverted)
                     {
                        if (!(d & _BV(j)))
                        {
                                my_setpixel(x+i, y+j, textcolor);
                        }
                        else
                        {
                                my_setpixel(x+i, y+j, !textcolor);
                        }
                     }
                    else
                     {
                        if ((d & _BV(j)))
                        {
                                my_setpixel(x+i, y+j, textcolor);
                        }
                        else
                        {
                                my_setpixel(x+i, y+j, !textcolor);
                        }
                     }
                }
        }

       if(!inverted)
        {
         for ( j = 0; j<8; j++)
          {
                my_setpixel(x+5, y+j, !textcolor);
          }
         for( i = 0; i<6; i++)
          {
                my_setpixel(x+i,y-1, !textcolor);
          }
        }
       else
        {
         for ( j = 0; j<8; j++)
          {
                my_setpixel(x+5, y+j, textcolor);
          }
         for( i = 0; i<6; i++)
          {
                my_setpixel(x+i,y-1, textcolor);
          }
        }

        updateBoundingBox(x, y, x+5, y + 8);
}

void LCDwrite(uint8_t c, uint8_t is_inverted)
{
        if (c == '\n')
        {
                cursor_y += textsize*8;
                cursor_x = 0;
        }
        else if (c == '\r')
        {
                // skip em
        }
        else
        {
                LCDdrawchar(cursor_x, cursor_y, c, is_inverted);
                cursor_x += textsize*6;
                if (cursor_x >= (LCDWIDTH-5))
                {
                        cursor_x = 0;
                        cursor_y+=8;
                }
                if (cursor_y >= LCDHEIGHT)
                        cursor_y = 0;
        }
}


void LCDdrawstring(uint8_t x, uint8_t y, char *c, uint8_t is_inverted)
{
        cursor_x = x;
        cursor_y = y;
        while (*c)
        {
                LCDwrite(*c++, is_inverted);
        }
}

void LCDdisplay(void)
{
        uint8_t col, maxcol, p;

        for(p = 0; p < 6; p++)
        {
#ifdef enablePartialUpdate
                // check if this page is part of update
                if ( yUpdateMin >= ((p+1)*8) )
                {
                        continue;   // nope, skip it!
                }
                if (yUpdateMax < p*8)
                {
                        break;
                }
#endif

                LCDcommand(PCD8544_SETYADDR | p);


#ifdef enablePartialUpdate
                col = xUpdateMin;
                maxcol = xUpdateMax;
#else
                // start at the beginning of the row
                col = 0;
                maxcol = LCDWIDTH-1;
#endif

                LCDcommand(PCD8544_SETXADDR | col);

                for(; col <= maxcol; col++) {
                        //uart_putw_dec(col);
                        //uart_putchar(' ');
                        LCDdata(pcd8544_buffer[(LCDWIDTH*p)+col]);
                }
        }

        LCDcommand(PCD8544_SETYADDR );  // no idea why this is necessary but it is to finish the last byte?
#ifdef enablePartialUpdate
        xUpdateMin = LCDWIDTH - 1;
        xUpdateMax = 0;
        yUpdateMin = LCDHEIGHT-1;
        yUpdateMax = 0;
#endif

}

void LCDInit(uint8_t contrast)
{
        _din = DIN1;
        _sclk = SCLK1;
        _dc = DC1;
        _rst = RST1;
        _cs = CS1;
        cursor_x = cursor_y = 0;
        textsize = 1;
        textcolor = BLACK;

        //if(!bcm2835_init())
        // {
        //  printf("LIB_i2c_bcm2835_init: fatal: cannot initialize BCM2835 library!");
        //  return;
        // }

        // set pin directions
        bcm2835_gpio_fsel(_din,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(_sclk,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(_dc,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(_rst,BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(_cs,BCM2835_GPIO_FSEL_OUTP);

        // toggle RST low to reset; CS low so it'll listen to us
        if (_cs > 0)
          digitalWrite(_cs, LOW);

        digitalWrite(_rst, LOW);
        usleep(500);
        digitalWrite(_rst, HIGH);

        // get into the EXTENDED mode!
        LCDcommand(PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );

        // LCD bias select (4 is optimal?)
        LCDcommand(PCD8544_SETBIAS | 0x4);

        // set VOP
        if (contrast > 0x7f)
            contrast = 0x7f;

        LCDcommand( PCD8544_SETVOP | contrast); // Experimentally determined

        // normal mode
        LCDcommand(PCD8544_FUNCTIONSET);

        // Set display to Normal
        LCDcommand(PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);

        // set up a bounding box for screen updates
        updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);

}

void LCDclear(void) {
  memset(pcd8544_buffer, 0, LCDWIDTH*LCDHEIGHT/8);
  updateBoundingBox(0, 0, LCDWIDTH-1, LCDHEIGHT-1);
  cursor_y = cursor_x = 0;
  pthread_mutex_lock(&G_display_lock);
  LCDdisplay();
  pthread_mutex_unlock(&G_display_lock);
}

