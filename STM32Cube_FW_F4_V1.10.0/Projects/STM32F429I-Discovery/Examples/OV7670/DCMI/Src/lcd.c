#include "main.h"
#include "ili9341.h"
#include "lcd.h"

//static void function_name( void );

void lcd_initialize( void ) {
	ili9341_Init( );
}

void lcd_copy_RGB565_frameBuffer( uint32_t *buffer )
{
	int i, j;
	uint8_t *p_byte = (uint8_t*) buffer;

	for(i=0; i<240; i++) {
		for(j=0; j<320; j++) {
			ili9341_WriteData(*p_byte++);
			ili9341_WriteData(*p_byte++);
		}
	}

	return;
}

void lcd_init_Gram( uint8_t r, uint8_t g, uint8_t b ) {
	int i=0, j=0;

#ifdef LCD_RGB_565    // 16bits color depth
	uint8_t val1, val2;
	val1  =  (r << 3);
	val1 |= ((g >> 3) & 0x7);

	val2  =  (g << 5);
	val2 |=  (b & 0x1F);

	/* Set WRX to send data */
	//LCD_WRX_HIGH();

	/* Reset LCD control line(/CS) and Send data */  
	//LCD_CS_LOW();

	for(i=0; i<240; i++) {
		for(j=0; j<320; j++) {
			ili9341_WriteData(val1);
			ili9341_WriteData(val2);
			//SPIx_Write((uint16_t) val1);
			//SPIx_Write((uint16_t) val2);
		}
	}

	/* Deselect: Chip Select high */
	//LCD_CS_HIGH();
#else	// 18bits color depth
	for(i=0; i<240; i++) {
		for(j=0; j<320; j++) {
			ili9341_WriteData(r<<2);
			ili9341_WriteData(g<<2);
			ili9341_WriteData(b<<2);
		}
	}
#endif

	return;
}
