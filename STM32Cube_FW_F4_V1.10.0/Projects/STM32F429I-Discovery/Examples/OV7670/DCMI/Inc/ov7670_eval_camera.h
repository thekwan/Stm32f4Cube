#ifndef _OV7670_EVAL_CAMERA_H_
#define _OV7670_EVAL_CAMERA_H_

#include "main.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_dcmi.h"

#define FRAME_BUFF_ROW		240
#define FRAME_BUFF_COL		320
#define PIXEL_PER_4BYTES	2
#define DCMI_FRAME_BUFF_MAX_SIZE	((FRAME_BUFF_ROW * FRAME_BUFF_COL) / PIXEL_PER_4BYTES)

typedef enum {
	CAMERA_R176x140,	// QCIF
	CAMERA_R352x288,	// CIF
	CAMERA_R320x240,	// QVGA
	CAMERA_R640x480 	// VGA
}CameraResolution ;

int   dcmi_hw_initialize( void );
int   BSP_GET_FRAME_MEM ( int addr );
void  BSP_SET_FRAME_MEM ( int addr, int data );
void  BSP_CAMERA_ContinuousStart( void );
void  BSP_CAMERA_SnapshotStart  ( void );
void  BSP_CAMERA_Suspend        ( void );
void  BSP_CAMERA_Resume         ( void );
//void  BSP_CAMERA_Stop           ( void );
void  BSP_DCMI_IRQHandler       ( void );
void  BSP_DCMI_DMA_IRQHandler   ( void );
void  BSP_OV7670_Init           ( void );

#endif
