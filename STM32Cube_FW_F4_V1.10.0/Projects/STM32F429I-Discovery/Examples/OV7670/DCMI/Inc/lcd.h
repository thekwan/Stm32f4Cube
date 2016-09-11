#ifndef _LCD_H_
#define _LCD_H_

/*
#define SCCB_SIO_C_PIN                          GPIO_PIN_0
#define SCCB_SIO_D_PIN                          GPIO_PIN_1
#define SCCB_GPIO_PORT                          GPIOB
#define SCCB_GPIO_CLK_ENABLE()                  __GPIOB_CLK_ENABLE()  
#define SCCB_GPIO_CLK_DISABLE()                 __GPIOB_CLK_DISABLE()  

#define SCCB_ADDR_WRITE   0x42
#define SCCB_ADDR_READ    0x43

#define UART_DEBUG_MSG	1
*/

void lcd_initialize( void );
void lcd_init_Gram( uint8_t r, uint8_t g, uint8_t b );
void lcd_copy_RGB565_frameBuffer( uint32_t *buffer );

#endif
