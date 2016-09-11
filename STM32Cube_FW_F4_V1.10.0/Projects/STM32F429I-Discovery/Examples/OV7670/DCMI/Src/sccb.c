#include "main.h"
#include "sccb.h"

static void sccb_us_delay( int us );
static void start_transmission( void );
static void stop_transmission ( void );
static void phase1_id_address ( uint8_t addr );
static void phase2_sub_address( uint8_t reg );
static void phase2_read_data  ( uint8_t*data );
static void phase3_write_data ( uint8_t data );

void sccb_hw_initialize( void ) {
	GPIO_InitTypeDef  GPIO_InitStruct;
  
	/* Enable the GPIO Clock */
	SCCB_GPIO_CLK_ENABLE();

	/* Configure the GPIO_LED pin */
	GPIO_InitStruct.Pin   = SCCB_SIO_C_PIN | SCCB_SIO_D_PIN;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
  
	HAL_GPIO_Init(SCCB_GPIO_PORT, &GPIO_InitStruct);
  
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_D_PIN, GPIO_PIN_SET); 
}

void sccb_hw_deinitialize( void ) {
	/* Disable the GPIO Clock */
	SCCB_GPIO_CLK_DISABLE();
}

void sccb_write_reg( uint8_t reg, uint8_t data ) {
	start_transmission( );

	phase1_id_address( SCCB_ADDR_WRITE );

	phase2_sub_address( reg );

	phase3_write_data( data );

	stop_transmission( );
}

void sccb_read_reg( uint8_t reg, uint8_t *data ) {
	start_transmission( );

	phase1_id_address( SCCB_ADDR_WRITE );

	sccb_us_delay(10);

	phase2_sub_address( reg );

	sccb_us_delay(10);

	stop_transmission( );

	/* time delay btw 2-phase write cycle and 2-phase read cycle */
	sccb_us_delay(50);
	start_transmission( );

	phase1_id_address( SCCB_ADDR_READ );

	sccb_us_delay(10);

	phase2_read_data ( data );

	stop_transmission( );
}

//Quick hack, approximately 1ms delay
static void sccb_us_delay(int us)
{
   while (us-- > 0) {
      //volatile int x=5971;	// this is for milli sec
      //volatile int x=597;		// this is for micro sec
      volatile int x=30;		// this is for micro sec
      while (x-- > 0)
         __asm("nop");
   }
}

static void start_transmission( void ) {
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_D_PIN, GPIO_PIN_SET); 
	sccb_us_delay( 10 );	// delay 10 us(micro sec)
	
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_D_PIN, GPIO_PIN_RESET); 
	sccb_us_delay( 10 );
	
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_RESET); 
	sccb_us_delay( 50 );
}

static void stop_transmission ( void )
{
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_RESET); 
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_D_PIN, GPIO_PIN_RESET); 
	sccb_us_delay( 50 );	// delay 10 us(micro sec)
	
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
	sccb_us_delay( 10 );
	
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_D_PIN, GPIO_PIN_SET); 
	sccb_us_delay( 200 );
}


static void phase1_id_address ( uint8_t addr )
{
	int i;
	uint8_t mask = 0x80;
	GPIO_PinState pin_state;

	for( i = 0 ; i < 8 ; i++ ) {
		pin_state = ((addr & mask) != 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
		HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_D_PIN, pin_state ); 
		mask >>= 1;
		sccb_us_delay( 6 );

		HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
		sccb_us_delay( 10 );
		HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_RESET); 
		sccb_us_delay( 4 );
	}

	sccb_us_delay( 6 );
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
	sccb_us_delay( 10 );
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_RESET); 
	sccb_us_delay( 4 );
}

static void phase2_sub_address( uint8_t reg )
{
	int i;
	uint8_t mask = 0x80;
	GPIO_PinState pin_state;

	for( i = 0 ; i < 8 ; i++ ) {
		pin_state = ((reg & mask) != 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
		HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_D_PIN, pin_state ); 
		mask >>= 1;
		sccb_us_delay( 6 );

		HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
		sccb_us_delay( 10 );
		HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_RESET); 
		sccb_us_delay( 4 );
	}

	sccb_us_delay( 6 );
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
	sccb_us_delay( 10 );
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_RESET); 
	sccb_us_delay( 4 );
}

static void phase2_read_data  ( uint8_t*data )
{
	int i;
	GPIO_InitTypeDef  GPIO_InitStruct;
 	GPIO_PinState pin_state;
  
	/* Change Data pin from output mode to input mode */
	GPIO_InitStruct.Pin   = SCCB_SIO_D_PIN;
	GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull  = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
  
	HAL_GPIO_Init(SCCB_GPIO_PORT, &GPIO_InitStruct);

	*data = 0x0;
	for( i = 0 ; i < 8 ; i++ ) {
		*data = *data << 1;
		sccb_us_delay( 6 );
		pin_state = HAL_GPIO_ReadPin(SCCB_GPIO_PORT, SCCB_SIO_D_PIN); 
		*data = *data | ((pin_state == GPIO_PIN_SET) ? 1 : 0);

		HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
		sccb_us_delay( 10 );
		HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_RESET); 
		sccb_us_delay( 4 );
	}

	sccb_us_delay( 6 );
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_SET); 
	sccb_us_delay( 10 );
	HAL_GPIO_WritePin(SCCB_GPIO_PORT, SCCB_SIO_C_PIN, GPIO_PIN_RESET); 
	sccb_us_delay( 4 );

	/* Change Data pin from input mode to output mode */
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
	HAL_GPIO_Init(SCCB_GPIO_PORT, &GPIO_InitStruct);


}

static void phase3_write_data ( uint8_t data )
{
	phase2_sub_address( data );

	sccb_us_delay(6);
}
