#include "stm32f4xx.h"
#include "usart.h"

/* UART Hanlder declaration */
UART_HandleTypeDef UartHandle;
__IO ITStatus UartReady = RESET;

UartMsgBuffer UartTxBuffer;
UartMsgBuffer UartRxBuffer;

static void UartBufferClear( UartMsgBuffer *buffer );
static UartBuffStat EnQueue( UartMsgBuffer *buffer, uint8_t msg );
static UartBuffStat DeQueue( UartMsgBuffer *buffer, uint8_t *data );
//static UartBuffStat QueueProc( UartMsgBuffer *buffer, UartBuffOper oper );

static void USART_SendData(UART_HandleTypeDef *huart, uint8_t data)
{
	huart->Instance->DR = data;

	return;
}

static uint8_t USART_ReceiveData(UART_HandleTypeDef *huart)
{
	return (uint8_t) huart->Instance->DR;
}


// The variable below is to indicate whether TXE interrupt is enabled or disabled.
// The TXE interrupt is only enabled whenever there are data to be transmitted.
// (when there is no data, then we disabled this interrupt by software)
int  enabled_IT_TXE = 0;

int  overflow_UartRxBuffer = 0;

void SendMessage( char *msg , NewLineFlag new_line_flag )
{
	while( *msg != 0x0 ) {
		while( EnQueue( &UartTxBuffer , (uint8_t) *msg ) == FULL );
		msg++;
		if( enabled_IT_TXE == 0 ) {
			//USART_ITConfig( USARTx, USART_IT_TXE, ENABLE );
			__HAL_UART_ENABLE_IT( &UartHandle, UART_IT_TXE);
			__HAL_UART_ENABLE_IT( &UartHandle, UART_IT_TC);
			enabled_IT_TXE = 1;
		}
	}

	if( new_line_flag == NEWLINE ) {
		while( EnQueue( &UartTxBuffer , 0xD ) == FULL );
		while( EnQueue( &UartTxBuffer , 0xA ) == FULL );
	}

	return;
}

// The function below is not implemented yet.
void ReceiveMessage( char *msg , int size )
{
	__HAL_UART_ENABLE_IT( &UartHandle, UART_IT_RXNE);

    char *_msg = msg;
	do {
		while( DeQueue( &UartRxBuffer , (uint8_t*) msg ) == EMPTY );

        char data = *msg;
        int  feedback_flag = 1;
        /* Processing special key char (e.g. Back-space)
         */
        if( data == 0x8 ) {
            msg--;
            if( msg < _msg ) {
                msg = _msg;
                feedback_flag = 0;
            }
        }
        else
            msg++;


        /* Send a received char to HOST
         * Process special char key (back-space, enter, etc)
         */
        char fb_msg[4] = {0x0, 0x0, 0x0, 0x0};  // feedback msg
        fb_msg[0] = data;
        if( data == 0xD ) { // 'Enter': send LF(Line Feed) char
            fb_msg[1] =0x0A;
        }
        else if( data == 0x08 ) {   // 'BS': send 'space' key to remove char on HOST terminal
            fb_msg[1] = 0x20;
            fb_msg[2] = 0x08;
        }
        if( feedback_flag == 1 )
		    SendMessage( fb_msg, NONE );

 
	} while( *(msg-1) != 0xD && (--size) > 0 );

	*(msg-1) = 0x0;

	return;
}

void UART4_IRQHandler_wrap( void ) 
{
	uint8_t data;

	uint32_t tmp1 = 0, tmp2 = 0;

	tmp1 = __HAL_UART_GET_FLAG( &UartHandle, UART_FLAG_TXE );
	tmp2 = __HAL_UART_GET_IT_SOURCE( &UartHandle, UART_IT_TXE );
	if( (tmp1 != RESET) && (tmp2 != RESET) ) {	// TXE interrupt processing routine
		if( DeQueue( &UartTxBuffer , &data ) != EMPTY ) {
			// if there is still remaining data to be transmitted, then only send it and wait.
			USART_SendData( &UartHandle, data );
		}
		else {
			// if there is no remaining data to be tx, then disable the interrupt TXE of USART.
			//USART_ITConfig( USARTx, USART_IT_TXE, DISABLE );
			__HAL_UART_DISABLE_IT( &UartHandle, UART_IT_TC);
			__HAL_UART_DISABLE_IT( &UartHandle, UART_IT_TXE);
			enabled_IT_TXE = 0;
		}
	}

	tmp1 = __HAL_UART_GET_FLAG( &UartHandle, UART_FLAG_RXNE );
	tmp2 = __HAL_UART_GET_IT_SOURCE( &UartHandle, UART_IT_RXNE );
	if( (tmp1 != RESET) && (tmp2 != RESET) ) {	// RXNE interrupt processing routine
		uint8_t data = USART_ReceiveData( &UartHandle );

        /* Queuing received characters
         * if queue is overflowed, set indication flag to debug.
         */
        if( EnQueue( &UartRxBuffer , data ) == FULL ) {
            // Rx buffer is full.
            overflow_UartRxBuffer = 1;
        }
	}

	return;
}

static void UartBufferClear( UartMsgBuffer *buffer )
{
	int i;

	for( i = 0 ; i < UART_MSG_BUFF_SIZE ; i++ )
		buffer->queue[i] = 0;

	buffer->start_pointer = 0;
	buffer->end_pointer   = 0;
	buffer->count         = 0;

	return;
}

static UartBuffStat EnQueue( UartMsgBuffer *buffer, uint8_t msg )
{
	if( buffer->count == (UART_MSG_BUFF_SIZE-1) )
		return FULL;

	buffer->count++;
	buffer->queue[ buffer->start_pointer++ ] = msg;

	if( buffer->start_pointer >= UART_MSG_BUFF_SIZE )
		buffer->start_pointer -= UART_MSG_BUFF_SIZE;

	return FILLED;
}

static UartBuffStat DeQueue( UartMsgBuffer *buffer, uint8_t *data )
{
	if( buffer->count == 0 )
		return EMPTY;

	buffer->count--;
	*data = buffer->queue[ buffer->end_pointer++ ];

	if( buffer->end_pointer >= UART_MSG_BUFF_SIZE )
		buffer->end_pointer -= UART_MSG_BUFF_SIZE;

	return FILLED;
}

void UartBufferClearAll( void )
{
	UartBufferClear( &UartTxBuffer );
	UartBufferClear( &UartRxBuffer );

	return;
}
