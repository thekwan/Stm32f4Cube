#include "stm32f4xx.h"
#include "main.h"

#ifndef _UART_H_
#define _UART_H_

#define UART_MSG_BUFF_SIZE	64

/* UART Hanlder declaration */
extern UART_HandleTypeDef UartHandle;
extern __IO ITStatus UartReady;

typedef struct _uart_buffer {
	uint8_t  queue[ UART_MSG_BUFF_SIZE ];
	uint16_t start_pointer;    /* pointer to bucket to be filled */
	uint16_t end_pointer;      /* pointer to last input element  */
	uint16_t count;            /* total number of input elements */
} UartMsgBuffer;

typedef enum _uart_buffer_state {
	FULL=0,
	EMPTY=1,
	FILLED=2
} UartBuffStat;

typedef enum _new_line_flag {
	NEWLINE=0,
	NONE=1
} NewLineFlag;

void UartBufferClearAll( void );
void SendMessage( char *msg, NewLineFlag new_line_flag );
void ReceiveMessage( char *msg, int size );	// not implemented yet.
void UART4_IRQHandler_wrap( void );

#endif
