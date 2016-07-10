#include <string.h>
#include "shell.h"
#include "usart.h"


static int strncmpC(const char *str1, const char *str2, int length) {
	while( length-- > 0 ) {
		if( *str1++ != *str2++ )
			return 0;
	}

	return 1;
}

static int strlenC(const char *str, int max_length ) {
	int i;

	for( i = 0 ; i < max_length ; i++ ) {
		if( *str == 0x0 )
			break;
	}

	return i;
}

static int LED_CMD_PROC( const char *args, int max_length ) {

	if( strncmpC( args, " on", 3 ) == 1 ) {
		if( (strncmpC( args+3, " 3", 2 ) == 1) && (args[5] == 0x0) ) {
			BSP_LED_On(LED3);
		}
		else if( strncmpC( args+3, " 4", 2 ) == 1 && (args[5] == 0x0)  ) {
			BSP_LED_On(LED4);
		}
		else {
			return -1;
		}
	}
	else if( strncmpC( args, " off", 4 ) == 1 ) {
		if( strncmpC( args+4, " 3", 2 ) == 1 && (args[6] == 0x0)  ) {
			BSP_LED_Off(LED3);
		}
		else if( strncmpC( args+4, " 4", 2 ) == 1 && (args[6] == 0x0)  ) {
			BSP_LED_Off(LED4);
		}
		else {
			return -1;
		}
	}

	return 1;
}

void shell_processing( const char *cmd , int cmd_max_length ) {
	volatile int length = strlenC( cmd, cmd_max_length );
	int ret = -1;

	if( strncmpC( cmd , "led" , 3 ) == 1 ) {
		SendMessage("LED command", NEWLINE);
		ret = LED_CMD_PROC( cmd+3 , cmd_max_length-3 );
	}
	else if( strncmpC( cmd , "sccb" , 4 ) == 1 ) {
		SendMessage("SCCB command", NEWLINE);
		//SCCB_CMD_PROC( cmd+4 );
	}
	else if( strncmpC( cmd , "\r\n" , 2 ) == 1 ) {
		SendMessage(" ", NEWLINE);
	}

	if( ret == -1 ) {
		SendMessage("Command processing is failed!", NEWLINE);
	}

	return;
}


