#include <string.h>
#include "shell.h"
#include "usart.h"
#include "sccb.h"
#include "ov7670_eval_camera.h"

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

static void hex2str_32b(char *str, int data) {
	uint8_t hex;
	int i;

	*str++ = '0';
	*str++ = 'x';

	for( i = 7 ; i >= 0 ; i-- ) {
		hex = (data >> (i*4)) & 0xF;
		if( hex < 0xA )
			*str++ = hex + 0x30;
		else
			*str++ = hex + 55;
	}

	*str = 0x0;

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

static int SCCB_CMD_PROC( const char *args, int max_length ) {
	int ret = 0;	// 0: no error, -1: error
	uint8_t addr, data;
	char str[5];

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

	if( ret == -1 ) {
		SendMessage("Usage: sccb [read/write] [HEX(addr)] [HEX(data) if write]", NEWLINE);
	}

	return ret;
}

static int DCMI_CMD_PROC( const char *args, int max_length ) {
	int ret = 0;	// 0: no error, -1: error
	int data, i;
	char str[32];

	if( strncmpC( args, " snap", 5 ) == 1 ) {
		SendMessage("DCMI snapshot: ", NEWLINE);
		BSP_CAMERA_SnapshotStart( );
	} else if( strncmpC( args, " stop", 5 ) == 1 ) {
		SendMessage("DCMI suspend: ", NEWLINE);
		BSP_CAMERA_Suspend( );
	} else if( strncmpC( args, " resm", 5 ) == 1 ) {
		SendMessage("DCMI resume: ", NEWLINE);
		BSP_CAMERA_Resume( );
	} else if( strncmpC( args, " disp", 5 ) == 1 ) {
		for( i = 0 ; i < 32 ; i++) {
			data = BSP_GET_FRAME_MEM( i );
			hex2str_32b(str, data);
			SendMessage("DCMI frame data: ", NONE);
			SendMessage(str, NEWLINE);
		}
	} else if( strncmpC( args, " test", 5 ) == 1 ) {
		data = 0x12345678;
		for( i = 0 ; i < 10 ; i++ ) {
			BSP_SET_FRAME_MEM( i, data );
			data++;
		}
	} else {
		ret = -1;
	}

	if( ret == -1 ) {
		SendMessage("Usage: dcmi", NEWLINE);
	}

	return ret;
}



void shell_processing( const char *cmd , int cmd_max_length ) {
	volatile int length = strlenC( cmd, cmd_max_length );
	int ret = -1;

	if( strncmpC( cmd , "led" , 3 ) == 1 ) {
		//SendMessage("LED command", NEWLINE);
		ret = LED_CMD_PROC( cmd+3 , cmd_max_length-3 );
	}
	else if( strncmpC( cmd , "sccb" , 4 ) == 1 ) {
		//SendMessage("SCCB command", NEWLINE);
		ret = SCCB_CMD_PROC( cmd+4 , cmd_max_length-4 );
	}
	else if( strncmpC( cmd , "dcmi" , 4 ) == 1 ) {
		//SendMessage("DCMI command", NEWLINE);
		ret = DCMI_CMD_PROC( cmd+4 , cmd_max_length-4 );
	}
	else if( strncmpC( cmd , "\r\n" , 2 ) == 1 ) {
		SendMessage(" ", NEWLINE);
	}

	if( ret == -1 ) {
		SendMessage("Command processing is failed!", NEWLINE);
	}

	return;
}
