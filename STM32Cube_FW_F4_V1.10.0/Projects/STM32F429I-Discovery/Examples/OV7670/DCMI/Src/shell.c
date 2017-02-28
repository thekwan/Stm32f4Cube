#include "shell.h"
#include "string.h"
#include "usart.h"
#include "sccb.h"
#include "lcd.h"
#include "ov7670_eval_camera.h"

static void func_A( ) {
	return;
}

static struct _command command_list[] = {
	{ "command_name A", &func_A },
	{ "command_name B", &func_A },
};

static int LED_CMD_PROC( const char *args, int max_length ) {

	if( embed_strncmp( args, " on", 3 ) == 1 ) {
		if( (embed_strncmp( args+3, " 3", 2 ) == 1) && (args[5] == 0x0) ) {
			BSP_LED_On(LED3);
		}
		else if( embed_strncmp( args+3, " 4", 2 ) == 1 && (args[5] == 0x0)  ) {
			BSP_LED_On(LED4);
		}
		else {
			return -1;
		}
	}
	else if( embed_strncmp( args, " off", 4 ) == 1 ) {
		if( embed_strncmp( args+4, " 3", 2 ) == 1 && (args[6] == 0x0)  ) {
			BSP_LED_Off(LED3);
		}
		else if( embed_strncmp( args+4, " 4", 2 ) == 1 && (args[6] == 0x0)  ) {
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

	if( embed_strncmp( args, " read", 5 ) == 1 ) {
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
	} else if( embed_strncmp( args, " write", 6 ) == 1 ) {
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

	if( embed_strncmp( args, " snap", 5 ) == 1 ) {
		SendMessage("DCMI snapshot: ", NEWLINE);
		BSP_CAMERA_SnapshotStart( );
	} else if( embed_strncmp( args, " cont", 5 ) == 1 ) {
		SendMessage("DCMI continuous: ", NEWLINE);
		BSP_CAMERA_ContinuousStart( );
	} else if( embed_strncmp( args, " stop", 5 ) == 1 ) {
		SendMessage("DCMI suspend: ", NEWLINE);
		BSP_CAMERA_Suspend( );
	} else if( embed_strncmp( args, " resm", 5 ) == 1 ) {
		SendMessage("DCMI resume: ", NEWLINE);
		BSP_CAMERA_Resume( );
	} else if( embed_strncmp( args, " disp", 5 ) == 1 ) {
		for( i = 0 ; i < 32 ; i++) {
			data = BSP_GET_FRAME_MEM( i );
			hex2str_32b(str, data);
			SendMessage("DCMI frame data: ", NONE);
			SendMessage(str, NEWLINE);
		}
	} else if( embed_strncmp( args, " test", 5 ) == 1 ) {
		data = 0x00f800f8;
		for( i = 0 ; i < DCMI_FRAME_BUFF_MAX_SIZE/2 ; i++ ) {
			BSP_SET_FRAME_MEM( i, data );
		}
		data = 0x1f001f00;
		for( i = (DCMI_FRAME_BUFF_MAX_SIZE/2) ; i < DCMI_FRAME_BUFF_MAX_SIZE ; i++ ) {
			BSP_SET_FRAME_MEM( i, data );
		}

		BSP_OV7670_Init();
#ifdef DCMI_DEBUG
	} else if( embed_strncmp( args, " debugP", 7 ) == 1 ) {
			SendMessage("DCMI Line Cnt : ", NONE);
			hex2str(str, (uint8_t) dcmi_line_cnt);	// print address
			SendMessage(str, NEWLINE);
			SendMessage("DCMI VSYNC Cnt: ", NONE);
			hex2str(str, (uint8_t) dcmi_vsync_cnt);	// print address
			SendMessage(str, NEWLINE);
			SendMessage("DCMI Frame Cnt: ", NONE);
			hex2str(str, (uint8_t) dcmi_frame_cnt);	// print address
			SendMessage(str, NEWLINE);
			SendMessage("DCMI DMA regnd: ", NONE);
			hex2str_32b(str, dcmi_reg_ndr);	// print address
			SendMessage(str, NEWLINE);
			SendMessage("DCMI Errorcode: ", NONE);
			hex2str_32b(str, dcmi_error_code);	// print address
			SendMessage(str, NEWLINE);
	} else if( embed_strncmp( args, " debugR", 7 ) == 1 ) {
			dcmi_line_cnt = 0;
			dcmi_vsync_cnt = 0;
			dcmi_frame_cnt = 0;
			dcmi_error_code= 0;
#endif
	} else {
		ret = -1;
	}

	if( ret == -1 ) {
		SendMessage("Usage: dcmi", NEWLINE);
	}

	return ret;
}

static int LCD_CMD_PROC( const char *args, int max_length ) {
	int ret = 0;	// 0: no error, -1: error
	uint8_t r, g, b;

	if( embed_strncmp( args, " color", 6 ) == 1 ) {
		SendMessage("LCD change color: ", NEWLINE);
		ret = str2hex( args+7 , &r, 2 );	// get red color [hex]
		ret = str2hex( args+10, &g, 2 );	// get green color [hex]
		ret = str2hex( args+13, &b, 2 );	// get blue color [hex]
		lcd_init_Gram( r, g, b );
	} else if( embed_strncmp( args, " dcmi", 5 ) == 1 ) {
		SendMessage("LCD display dcmi:", NEWLINE);
		BSP_OV7670_DISPLAY_FRAME( );
	} else if( embed_strncmp( args, " dcm5", 5 ) == 1 ) {
		SendMessage("LCD display dcmi 20 continuous running:", NEWLINE);
		for( ret = 0 ; ret < 20 ; ret++ ) 
			BSP_OV7670_DISPLAY_FRAME( );
		ret = 0;
	} else {
		ret = -1;
	}

	if( ret == -1 ) {
		SendMessage("Usage: lcd color [R(hex)] [G(hex)] [B(hex)]", NEWLINE);
		SendMessage("Usage: lcd dcmi", NEWLINE);
	}

	return ret;
}



void shell_processing( char *cmd , int cmd_max_length ) {
	//int length = embed_strlen( cmd, cmd_max_length );
	int ret = -1;

	if( embed_strncmp( cmd , "led" , 3 ) == 1 ) {
		//SendMessage("LED command", NEWLINE);
		ret = LED_CMD_PROC( cmd+3 , cmd_max_length-3 );
	}
	else if( embed_strncmp( cmd , "sccb" , 4 ) == 1 ) {
		//SendMessage("SCCB command", NEWLINE);
		ret = SCCB_CMD_PROC( cmd+4 , cmd_max_length-4 );
	}
	else if( embed_strncmp( cmd , "dcmi" , 4 ) == 1 ) {
		//SendMessage("DCMI command", NEWLINE);
		ret = DCMI_CMD_PROC( cmd+4 , cmd_max_length-4 );
	}
	else if( embed_strncmp( cmd , "lcd" , 3 ) == 1 ) {
		//SendMessage("LCD  command", NEWLINE);
		ret = LCD_CMD_PROC( cmd+3 , cmd_max_length-3 );
	}
	else if( embed_strncmp( cmd , "uart" , 4 ) == 1 ) {
		//SendMessage("LCD  command", NEWLINE);
		UartBufferClearAll();
		ret = 0;
	}
	else if( embed_strncmp( cmd , "command" , 7 ) == 1 ) {
		//SendMessage("LCD  command", NEWLINE);
		SendMessage((char *) command_list[0].name, NEWLINE);
		ret = 0;
	}
	else if( embed_strncmp( cmd , "\r\n" , 2 ) == 1 ) {
		SendMessage(" ", NEWLINE);
	}

	if( ret == -1 ) {
		SendMessage("Command processing is failed!", NEWLINE);
	}

	return;
}
