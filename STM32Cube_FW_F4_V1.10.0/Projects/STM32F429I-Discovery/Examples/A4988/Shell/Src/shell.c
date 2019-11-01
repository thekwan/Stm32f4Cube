#include <string.h>
#include "shell.h"
#include "usart.h"
#include "lcd.h"

#if 0
static void hex2str(char *str, uint8_t data) {
	uint8_t lsb = data & 0xF;
	uint8_t msb = (data>>4) & 0xF;

	str[0] = '0';
	str[1] = 'x';

	if( msb < 0xA )
		str[2] = msb + 0x30;
	else
		str[2] = msb + 55;

	if( lsb < 0xA )
		str[3] = lsb + 0x30;
	else
		str[3] = lsb + 55;

	str[4] = 0x0;

	return;
}

static int str2hex(const char *str, uint8_t *data, int strMaxLength) {
	int i = 0;
	int ret = 0;

	*data = 0;
	while( strMaxLength-- ) {
		char c = str[i++];
		*data <<= 4;
		if( c >= 0x61 )
			*data += (c - 0x61) + 0xA;
		else if( c >= 0x41 )
			*data += (c - 0x41) + 0xA;
		else if( c >= 0x30 )
			*data += (c - 0x30);
		else
			ret = -1;	// invalid character
	}

	return ret;
}
#endif


static int strncmpC(const char *str1, const char *str2, int length) {
	while( length-- > 0 ) {
		if( *str1++ != *str2++ )
			return 0;
	}

	return 1;
}

#if 0
static int strlenC(const char *str, int max_length ) {
	int i;

	for( i = 0 ; i < max_length ; i++ ) {
		if( *str == 0x0 )
			break;
	}

	return i;
}
#endif

static int LED_CMD_PROC( const char *args, int max_length ) {

	if( strncmpC( args, " on", 3 ) == 1 ) {
		if( (strncmpC( args+3, " 3", 2 ) == 1) && (args[5] == 0x0) ) {
			BSP_LED_On(LED3);
            return 1;
		}
		else if( strncmpC( args+3, " 4", 2 ) == 1 && (args[5] == 0x0)  ) {
			BSP_LED_On(LED4);
            return 1;
		}
	}
	else if( strncmpC( args, " off", 4 ) == 1 ) {
		if( strncmpC( args+4, " 3", 2 ) == 1 && (args[6] == 0x0)  ) {
			BSP_LED_Off(LED3);
            return 1;
		}
		else if( strncmpC( args+4, " 4", 2 ) == 1 && (args[6] == 0x0)  ) {
			BSP_LED_Off(LED4);
            return 1;
		}
	}

	return -1;
}

static int LCD_CMD_PROC( const char *args, int max_length ) {
#if 0
	int ret = 0;	// 0: no error, -1: error
	uint8_t addr, data;
	char str[5];
#endif

	//lcd_init_Gram( 0x3F, 0x3F, 0x3F );
	lcd_init_Gram( 0x3F, 0x00, 0x00 );
	/*
	if( strncmpC( args, " read", 5 ) == 1 ) {
		ret = str2hex( args+6, &addr, 2 );	// get address
		if( (args[8] == 0x0) ) {
			sccb_read_reg( addr, &data );

#if defined(UART_DEBUG_MSG)
			SendMessage("SCCB Read data : ", NONE);
			hex2str(str, addr);
			SendMessage(str, NONE);
			SendMessage(" ", NONE);
			hex2str(str, data);
			SendMessage(str, NEWLINE);
#endif
		}
		else {
			ret = -1;
		}
	} else if( strncmpC( args, " write", 6 ) == 1 ) {
		ret = str2hex( args+7 , &addr, 2 );	// get address
		ret = str2hex( args+10, &data, 2 );	// get value
		if( args[12] == 0x0 ) {
			sccb_write_reg( addr, data);

#if defined(UART_DEBUG_MSG)
			SendMessage("SCCB Write data: ", NONE);
			hex2str(str, addr);	// print address
			SendMessage(str, NONE);
			SendMessage(" ", NONE);
			hex2str(str, data);	// print data
			SendMessage(str, NEWLINE);
#endif
		}
		else {
			ret = -1;
		}
	} else {
		ret = -1;
	}
	*/

#if 0
	if( ret == -1 ) {
		SendMessage("Usage: lcd [read/write] [HEX(addr)] [HEX(data) if write]", NEWLINE);
	}

	return ret;
#endif
    return 1;
}


void shell_processing( const char *cmd , int cmd_max_length ) {
	//volatile int length = strlenC( cmd, cmd_max_length );
	int ret = -1;

	if( strncmpC( cmd , "led" , 3 ) == 1 ) {
		//SendMessage("LED command", NEWLINE);
		ret = LED_CMD_PROC( cmd+3 , cmd_max_length-3 );
	}
	else if( strncmpC( cmd , "lcd" , 3 ) == 1 ) {
		//SendMessage("LCD  command", NEWLINE);
		ret = LCD_CMD_PROC( cmd+3 , cmd_max_length-3 );
	}
	else if( strncmpC( cmd , "\r\n" , 2 ) == 1 ) {
		SendMessage(" ", NEWLINE);
	}

	if( ret == -1 ) {
		SendMessage("'", NONE);
		SendMessage((char*)cmd, NONE);
		SendMessage("'", NONE);
		SendMessage(" command not found", NEWLINE);
	}

	return;
}
