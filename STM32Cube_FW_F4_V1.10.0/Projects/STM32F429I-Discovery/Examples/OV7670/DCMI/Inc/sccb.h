#ifndef _SCCH_H_
#define _SCCH_H_

#define SCCB_SIO_C_PIN                          GPIO_PIN_0
#define SCCB_SIO_D_PIN                          GPIO_PIN_1
#define SCCB_GPIO_PORT                          GPIOB
#define SCCB_GPIO_CLK_ENABLE()                  __GPIOB_CLK_ENABLE()  
#define SCCB_GPIO_CLK_DISABLE()                 __GPIOB_CLK_DISABLE()  

#define SCCB_ADDR_WRITE   0x42
#define SCCB_ADDR_READ    0x43

#define UART_DEBUG_MSG	1

void sccb_hw_initialize  ( void );
void sccb_hw_deinitialize( void );
void sccb_write_reg( uint8_t reg, uint8_t data );
void sccb_read_reg ( uint8_t reg, uint8_t*data );

#endif
