#include "ov7670_eval_camera.h"
#include "sccb.h"

uint32_t dcmi_frame_buff[DCMI_FRAME_BUFF_MAX_SIZE]	__attribute__ ((section(".bss")));

static DCMI_HandleTypeDef hdcmi_eval;
static DMA_HandleTypeDef  hdma_eval;
static CameraResolution current_resolution = 0;

static uint32_t GetSize(CameraResolution resolution);

int dcmi_hw_initialize( void )
{
	int ret = 0;

	/*** Configures DCMI peripheral ***/
	GPIO_InitTypeDef    GPIO_Init_Structure;
	DCMI_HandleTypeDef *phdcmi;

	/* Enable GPIO clocks */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOE_CLK_ENABLE();

	/* Configure DCMI GPIO as alternate function */
	GPIO_Init_Structure.Pin       = GPIO_PIN_4 | GPIO_PIN_6 | GPIO_PIN_9 | GPIO_PIN_10;
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_OD;
	GPIO_Init_Structure.Pull      = GPIO_PULLUP;
	GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
	GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;  
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);

	GPIO_Init_Structure.Pin       = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9; 
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_OD;
	GPIO_Init_Structure.Pull      = GPIO_PULLUP;
	GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
	GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;   
	HAL_GPIO_Init(GPIOB, &GPIO_Init_Structure);

	GPIO_Init_Structure.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_OD;
	GPIO_Init_Structure.Pull      = GPIO_PULLUP;
	GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
	GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;   
	HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

	GPIO_Init_Structure.Pin       = GPIO_PIN_4;
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_OD;
	GPIO_Init_Structure.Pull      = GPIO_PULLUP;
	GPIO_Init_Structure.Speed     = GPIO_SPEED_HIGH;
	GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;   
	HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

	/* Enable DCMI clock */
	__DCMI_CLK_ENABLE();

	/* Get the DCMI handle structure */
	phdcmi = &hdcmi_eval;

	/*** Configures the DCMI to interface with the camera module ***/
	/* DCMI configuration */
	phdcmi->Init.CaptureRate      = DCMI_CR_ALL_FRAME;  
	phdcmi->Init.HSPolarity       = DCMI_HSPOLARITY_LOW;
	phdcmi->Init.SynchroMode      = DCMI_SYNCHRO_HARDWARE;
	phdcmi->Init.VSPolarity       = DCMI_VSPOLARITY_HIGH;
	phdcmi->Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	phdcmi->Init.PCKPolarity      = DCMI_PCKPOLARITY_RISING;
	//phdcmi->Init.PCKPolarity      = DCMI_PCKPOLARITY_FALLING;
	phdcmi->Instance              = DCMI;
	
	/*** Configures DMA and NVIC peripherals ***/
	/* Enable DMA2 clock */
	__DMA2_CLK_ENABLE(); 

	/*** Configure the DMA ***/
	/* Set the parameters to be configured */
	hdma_eval.Init.Channel             = DMA_CHANNEL_1;
	hdma_eval.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_eval.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_eval.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_eval.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_eval.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
	hdma_eval.Init.Mode                = DMA_CIRCULAR;
	hdma_eval.Init.Priority            = DMA_PRIORITY_HIGH;
	hdma_eval.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;         
	hdma_eval.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma_eval.Init.MemBurst            = DMA_MBURST_SINGLE;
	hdma_eval.Init.PeriphBurst         = DMA_PBURST_SINGLE; 

	hdma_eval.Instance = DMA2_Stream1;

	/* Associate the initialized DMA handle to the DCMI handle */
	__HAL_LINKDMA(phdcmi, DMA_Handle, hdma_eval);

	/*** Configure the NVIC for DCMI and DMA ***/
	/* NVIC configuration for DCMI transfer complete interrupt */
	HAL_NVIC_SetPriority(DCMI_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DCMI_IRQn);  

	/* NVIC configuration for DMA2D transfer complete interrupt */
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

	/* Configure the DMA stream */
	if( HAL_DMA_Init(phdcmi->DMA_Handle) != HAL_OK ) {
		ret = -1;
	}

	if( HAL_DCMI_Init(phdcmi) != HAL_OK ) {
		ret = -1;
	}

#if 0
	if(ov2640_ReadID(CAMERA_I2C_ADDRESS) == OV2640_ID) { 
		/* Initialize the camera driver structure */ 
		camera_drv = &ov2640_drv;     
		/* Camera Init */   
		camera_drv->Init(CAMERA_I2C_ADDRESS, Resolution); 
		/* Return CAMERA_OK status */
		ret = CAMERA_OK;
	} 
  
  current_resolution = Resolution;
#else
	BSP_OV7670_Init( );
#endif

  current_resolution = CAMERA_R320x240;
  
  return ret;
}

void BSP_OV7670_Init( )
{
	sccb_write_reg( 0x12, 0x80 );	// COM7[7]=1: Resets all registers to default values
	sccb_write_reg( 0x12, 0x10 );	// COM7[4]=1: Output format QVGA selection
}

int BSP_GET_FRAME_MEM( int addr )
{
	return dcmi_frame_buff[addr];
}

void BSP_SET_FRAME_MEM( int addr , int data )
{
	dcmi_frame_buff[addr] = data;
}

/**
  * @brief  Starts the camera capture in continuous mode.
  * @param  buff: pointer to the camera output buffer
  * @retval None
  */
void BSP_CAMERA_ContinuousStart( void )
{ 
	/* Start the camera capture */
	HAL_DCMI_Start_DMA(&hdcmi_eval, DCMI_MODE_CONTINUOUS, (uint32_t)dcmi_frame_buff, GetSize(current_resolution));  
}

/**
  * @brief  Starts the camera capture in snapshot mode.
  * @param  buff: pointer to the camera output buffer
  * @retval None
  */
void BSP_CAMERA_SnapshotStart( void )
{ 
	/* Start the camera capture */
	HAL_DCMI_Start_DMA(&hdcmi_eval, DCMI_MODE_SNAPSHOT, (uint32_t)dcmi_frame_buff, GetSize(current_resolution));  
}

/**
  * @brief Suspend the CAMERA capture 
  * @param  None
  * @retval None
  */
void BSP_CAMERA_Suspend(void) 
{
	/* Disable the DMA */
	__HAL_DMA_DISABLE(hdcmi_eval.DMA_Handle);
	/* Disable the DCMI */
	__HAL_DCMI_DISABLE(&hdcmi_eval);
}

/**
  * @brief Resume the CAMERA capture 
  * @param  None
  * @retval None
  */
void BSP_CAMERA_Resume(void) 
{
	/* Enable the DCMI */
	__HAL_DCMI_ENABLE(&hdcmi_eval);
	/* Enable the DMA */
	__HAL_DMA_ENABLE(hdcmi_eval.DMA_Handle);
}

#if 0
/**
  * @brief  Stop the CAMERA capture 
  * @param  None
  * @retval Camera status
  */
uint8_t BSP_CAMERA_Stop(void) 
{
	DCMI_HandleTypeDef *phdcmi;

	uint8_t ret = CAMERA_ERROR;
	
	/* Get the DCMI handle structure */
	phdcmi = &hdcmi_eval;
	
	if(HAL_DCMI_Stop(phdcmi) == HAL_OK)
		ret = CAMERA_OK;
  
	/* Initialize IO */
	BSP_IO_Init();
  
	/* Reset the camera STANDBY pin */
	BSP_IO_ConfigPin(XSDN_PIN, IO_MODE_OUTPUT);
	BSP_IO_WritePin(XSDN_PIN, RESET);  
  
	return ret;
}

/**
  * @brief  Configures the camera contrast and brightness.
  * @param  contrast_level: Contrast level
  *          This parameter can be one of the following values:
  *            @arg  CAMERA_CONTRAST_LEVEL4: for contrast +2
  *            @arg  CAMERA_CONTRAST_LEVEL3: for contrast +1
  *            @arg  CAMERA_CONTRAST_LEVEL2: for contrast  0
  *            @arg  CAMERA_CONTRAST_LEVEL1: for contrast -1
  *            @arg  CAMERA_CONTRAST_LEVEL0: for contrast -2
  * @param  brightness_level: Contrast level
  *          This parameter can be one of the following values:
  *            @arg  CAMERA_BRIGHTNESS_LEVEL4: for brightness +2
  *            @arg  CAMERA_BRIGHTNESS_LEVEL3: for brightness +1
  *            @arg  CAMERA_BRIGHTNESS_LEVEL2: for brightness  0
  *            @arg  CAMERA_BRIGHTNESS_LEVEL1: for brightness -1
  *            @arg  CAMERA_BRIGHTNESS_LEVEL0: for brightness -2    
  * @retval None
  */
void BSP_CAMERA_ContrastBrightnessConfig(uint32_t contrast_level, uint32_t brightness_level)
{
	if(camera_drv->Config != NULL)
  {
    camera_drv->Config(CAMERA_I2C_ADDRESS, CAMERA_CONTRAST_BRIGHTNESS, contrast_level, brightness_level);
  }  
}

/**
  * @brief  Configures the camera white balance.
  * @param  Mode: black_white mode
  *          This parameter can be one of the following values:
  *            @arg  CAMERA_BLACK_WHITE_BW
  *            @arg  CAMERA_BLACK_WHITE_NEGATIVE
  *            @arg  CAMERA_BLACK_WHITE_BW_NEGATIVE
  *            @arg  CAMERA_BLACK_WHITE_NORMAL       
  * @retval None
  */
void BSP_CAMERA_BlackWhiteConfig(uint32_t Mode)
{
  if(camera_drv->Config != NULL)
  {
    camera_drv->Config(CAMERA_I2C_ADDRESS, CAMERA_BLACK_WHITE, Mode, 0);
  }  
}

/**
  * @brief  Configures the camera color effect.
  * @param  Effect: Color effect
  *          This parameter can be one of the following values:
  *            @arg  CAMERA_COLOR_EFFECT_ANTIQUE               
  *            @arg  CAMERA_COLOR_EFFECT_BLUE        
  *            @arg  CAMERA_COLOR_EFFECT_GREEN    
  *            @arg  CAMERA_COLOR_EFFECT_RED        
  * @retval None
  */
void BSP_CAMERA_ColorEffectConfig(uint32_t Effect)
{
  if(camera_drv->Config != NULL)
  {
    camera_drv->Config(CAMERA_I2C_ADDRESS, CAMERA_COLOR_EFFECT, Effect, 0);
  }  
}
#endif

/**
  * @brief  Handles DCMI interrupt request.
  * @param  None
  * @retval None
  */
void BSP_DCMI_IRQHandler(void) 
{
	HAL_DCMI_IRQHandler(&hdcmi_eval);
}

/**
  * @brief  Handles DMA interrupt request.
  * @param  None
  * @retval None
  */
void BSP_DCMI_DMA_IRQHandler(void) 
{
  HAL_DMA_IRQHandler(hdcmi_eval.DMA_Handle);
}

/**
  * @brief  Get the capture size.
  * @param  current_resolution: the current resolution.
  * @retval capture size.
  */
static uint32_t GetSize(CameraResolution resolution)
{ 
	uint32_t size = 0;

	/* Get capture size */
	switch (resolution)
	{
		case CAMERA_R176x140:
			size =  (176 * 140) / PIXEL_PER_4BYTES;
			break;    
		case CAMERA_R352x288:
			size =  (352 * 288) / PIXEL_PER_4BYTES;
			break;    
		case CAMERA_R320x240:
			size =  (320 * 240) / PIXEL_PER_4BYTES;
			break;
		case CAMERA_R640x480:
			size =  (640 * 480) / PIXEL_PER_4BYTES;
			break;
		default:
			break;
	}
	return size;
}

/**
  * @brief  Initializes the DCMI MSP.
  * @param  None
  * @retval None
  */
void HAL_DCMI_MspInit(DCMI_HandleTypeDef* hdcmi)
{
 
}

/**
  * @brief  Line event callback
  * @param  hdcmi: pointer to the DCMI handle  
  * @retval None
  */
void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
	//BSP_LED_Toggle(LED4);
  //BSP_CAMERA_LineEventCallback();
}

/**
  * @brief  VSYNC event callback
  * @param  hdcmi: pointer to the DCMI handle  
  * @retval None
  */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
  //BSP_CAMERA_VsyncEventCallback();
}

/**
  * @brief  Frame event callback
  * @param  hdcmi: pointer to the DCMI handle  
  * @retval None
  */
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
  //BSP_CAMERA_FrameEventCallback();
	BSP_LED_Toggle(LED4);
}

/**
  * @brief  Error callback
  * @param  hdcmi: pointer to the DCMI handle  
  * @retval None
  */
void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{        
	BSP_LED_On(LED3);
  //BSP_CAMERA_ErrorCallback();
}

/**
  * @}
  */  
  
/**
  * @}
  */
  
/**
  * @}
  */
  
/**
  * @}
  */      

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/




