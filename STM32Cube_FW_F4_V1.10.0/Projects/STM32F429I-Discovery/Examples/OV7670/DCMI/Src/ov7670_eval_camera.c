#include "ov7670_eval_camera.h"
#include "sccb.h"
#include "lcd.h"


/*
 * The default register settings, as obtained from OmniVision.  There
 * is really no making sense of most of these - lots of "reserved" values
 * and such.
 *
 * These settings give VGA YUYV.
 */

struct regval_list ov7670_default_regs[] = {
	{ REG_COM7, COM7_RESET },
/*
 * Clock scale: 3 = 15fps
 *              2 = 20fps
 *              1 = 30fps
 */
	{ REG_CLKRC, 0x1 },	/* OV: clock scale (30 fps) */
	{ REG_TSLB,  0x04 },	/* OV */
	{ REG_COM7, 0 },	/* VGA */
	/*
	 * Set the hardware window.  These values from OV don't entirely
	 * make sense - hstop is less than hstart.  But they work...
	 */
	{ REG_HSTART, 0x13 },	{ REG_HSTOP, 0x01 },
	{ REG_HREF, 0xb6 },	{ REG_VSTART, 0x02 },
	{ REG_VSTOP, 0x7a },	{ REG_VREF, 0x0a },

	{ REG_COM3, 0 },	{ REG_COM14, 0 },
	/* Mystery scaling numbers */
	{ 0x70, 0x3a },		{ 0x71, 0x35 },
	{ 0x72, 0x11 },		{ 0x73, 0xf0 },
	{ 0xa2, 0x02 },		{ REG_COM10, 0x0 },

	/* Gamma curve values */
	{ 0x7a, 0x20 },		{ 0x7b, 0x10 },
	{ 0x7c, 0x1e },		{ 0x7d, 0x35 },
	{ 0x7e, 0x5a },		{ 0x7f, 0x69 },
	{ 0x80, 0x76 },		{ 0x81, 0x80 },
	{ 0x82, 0x88 },		{ 0x83, 0x8f },
	{ 0x84, 0x96 },		{ 0x85, 0xa3 },
	{ 0x86, 0xaf },		{ 0x87, 0xc4 },
	{ 0x88, 0xd7 },		{ 0x89, 0xe8 },

	/* AGC and AEC parameters.  Note we start by disabling those features,
	   then turn them only after tweaking the values. */
	{ REG_COM8, COM8_FASTAEC | COM8_AECSTEP | COM8_BFILT },
	{ REG_GAIN, 0 },	{ REG_AECH, 0 },
	{ REG_COM4, 0x40 }, /* magic reserved bit */
	{ REG_COM9, 0x18 }, /* 4x gain + magic rsvd bit */
	{ REG_BD50MAX, 0x05 },	{ REG_BD60MAX, 0x07 },
	{ REG_AEW, 0x95 },	{ REG_AEB, 0x33 },
	{ REG_VPT, 0xe3 },	{ REG_HAECC1, 0x78 },
	{ REG_HAECC2, 0x68 },	{ 0xa1, 0x03 }, /* magic */
	{ REG_HAECC3, 0xd8 },	{ REG_HAECC4, 0xd8 },
	{ REG_HAECC5, 0xf0 },	{ REG_HAECC6, 0x90 },
	{ REG_HAECC7, 0x94 },
	{ REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC },

	/* Almost all of these are magic "reserved" values.  */
	{ REG_COM5, 0x61 },	{ REG_COM6, 0x4b },
	{ 0x16, 0x02 },		{ REG_MVFP, 0x07 },
	{ 0x21, 0x02 },		{ 0x22, 0x91 },
	{ 0x29, 0x07 },		{ 0x33, 0x0b },
	{ 0x35, 0x0b },		{ 0x37, 0x1d },
	{ 0x38, 0x71 },		{ 0x39, 0x2a },
	{ REG_COM12, 0x78 },	{ 0x4d, 0x40 },
	{ 0x4e, 0x20 },		{ REG_GFIX, 0 },
	{ 0x6b, 0x4a },		{ 0x74, 0x10 },
	{ 0x8d, 0x4f },		{ 0x8e, 0 },
	{ 0x8f, 0 },		{ 0x90, 0 },
	{ 0x91, 0 },		{ 0x96, 0 },
	{ 0x9a, 0 },		{ 0xb0, 0x84 },
	{ 0xb1, 0x0c },		{ 0xb2, 0x0e },
	{ 0xb3, 0x82 },		{ 0xb8, 0x0a },

	/* More reserved magic, some of which tweaks white balance */
	{ 0x43, 0x0a },		{ 0x44, 0xf0 },
	{ 0x45, 0x34 },		{ 0x46, 0x58 },
	{ 0x47, 0x28 },		{ 0x48, 0x3a },
	{ 0x59, 0x88 },		{ 0x5a, 0x88 },
	{ 0x5b, 0x44 },		{ 0x5c, 0x67 },
	{ 0x5d, 0x49 },		{ 0x5e, 0x0e },
	{ 0x6c, 0x0a },		{ 0x6d, 0x55 },
	{ 0x6e, 0x11 },		{ 0x6f, 0x9f }, /* "9e for advance AWB" */
	{ 0x6a, 0x40 },		{ REG_BLUE, 0x40 },
	{ REG_RED, 0x60 },
	{ REG_COM8, COM8_FASTAEC|COM8_AECSTEP|COM8_BFILT|COM8_AGC|COM8_AEC|COM8_AWB },

	/* Matrix coefficients */
	{ 0x4f, 0x80 },		{ 0x50, 0x80 },
	{ 0x51, 0 },		{ 0x52, 0x22 },
	{ 0x53, 0x5e },		{ 0x54, 0x80 },
	{ 0x58, 0x9e },

	{ REG_COM16, COM16_AWBGAIN },	{ REG_EDGE, 0 },
	{ 0x75, 0x05 },		{ 0x76, 0xe1 },
	{ 0x4c, 0 },		{ 0x77, 0x01 },
	{ REG_COM13, 0xc3 },	{ 0x4b, 0x09 },
	{ 0xc9, 0x60 },		{ REG_COM16, 0x38 },
	{ 0x56, 0x40 },

	{ 0x34, 0x11 },		{ REG_COM11, COM11_EXP|COM11_HZAUTO },
	{ 0xa4, 0x88 },		{ 0x96, 0 },
	{ 0x97, 0x30 },		{ 0x98, 0x20 },
	{ 0x99, 0x30 },		{ 0x9a, 0x84 },
	{ 0x9b, 0x29 },		{ 0x9c, 0x03 },
	{ 0x9d, 0x4c },		{ 0x9e, 0x3f },
	{ 0x78, 0x04 },

	/* Extra-weird stuff.  Some sort of multiplexor register */
	{ 0x79, 0x01 },		{ 0xc8, 0xf0 },
	{ 0x79, 0x0f },		{ 0xc8, 0x00 },
	{ 0x79, 0x10 },		{ 0xc8, 0x7e },
	{ 0x79, 0x0a },		{ 0xc8, 0x80 },
	{ 0x79, 0x0b },		{ 0xc8, 0x01 },
	{ 0x79, 0x0c },		{ 0xc8, 0x0f },
	{ 0x79, 0x0d },		{ 0xc8, 0x20 },
	{ 0x79, 0x09 },		{ 0xc8, 0x80 },
	{ 0x79, 0x02 },		{ 0xc8, 0xc0 },
	{ 0x79, 0x03 },		{ 0xc8, 0x40 },
	{ 0x79, 0x05 },		{ 0xc8, 0x30 },
	{ 0x79, 0x26 },

	{ 0xff, 0xff },	/* END MARKER */
};

/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 *
 * RGB656 and YUV422 come from OV; RGB444 is homebrewed.
 *
 * IMPORTANT RULE: the first entry must be for COM7, see ov7670_s_fmt for why.
 */


struct regval_list ov7670_fmt_yuv422[] = {
	{ REG_COM7, 0x0 },  /* Selects YUV mode */
	{ REG_RGB444, 0 },	/* No RGB444 please */
	{ REG_COM1, 0 },	/* CCIR601 */
	{ REG_COM15, COM15_R00FF },
	{ REG_COM9, 0x48 }, /* 32x gain ceiling; 0x8 is reserved bit */
	{ 0x4f, 0x80 }, 	/* "matrix coefficient 1" */
	{ 0x50, 0x80 }, 	/* "matrix coefficient 2" */
	{ 0x51, 0    },		/* vb */
	{ 0x52, 0x22 }, 	/* "matrix coefficient 4" */
	{ 0x53, 0x5e }, 	/* "matrix coefficient 5" */
	{ 0x54, 0x80 }, 	/* "matrix coefficient 6" */
	{ REG_COM13, COM13_GAMMA|COM13_UVSAT },
	{ 0xff, 0xff },
};

struct regval_list ov7670_fmt_rgb565[] = {
	//{ REG_COM7, COM7_RGB },	/* Selects RGB mode */ original code
	{ REG_COM7, (COM7_FMT_QVGA | COM7_RGB) },	/* Selects RGB mode */
	{ REG_RGB444, 0 },	/* No RGB444 please */
	{ REG_COM1, 0x0 },	/* CCIR601 */
	{ REG_COM15, COM15_RGB565 },
	{ REG_COM9, 0x38 }, 	/* 16x gain ceiling; 0x8 is reserved bit */
	{ 0x4f, 0xb3 }, 	/* "matrix coefficient 1" */
	{ 0x50, 0xb3 }, 	/* "matrix coefficient 2" */
	{ 0x51, 0    },		/* vb */
	{ 0x52, 0x3d }, 	/* "matrix coefficient 4" */
	{ 0x53, 0xa7 }, 	/* "matrix coefficient 5" */
	{ 0x54, 0xe4 }, 	/* "matrix coefficient 6" */
	{ REG_COM13, COM13_GAMMA|COM13_UVSAT },
	{ 0xff, 0xff },
};

struct regval_list ov7670_fmt_rgb444[] = {
	{ REG_COM7, COM7_RGB },	/* Selects RGB mode */
	{ REG_RGB444, R444_ENABLE },	/* Enable xxxxrrrr ggggbbbb */
	{ REG_COM1, 0x0 },	/* CCIR601 */
	{ REG_COM15, COM15_R01FE|COM15_RGB565 }, /* Data range needed? */
	{ REG_COM9, 0x38 }, 	/* 16x gain ceiling; 0x8 is reserved bit */
	{ 0x4f, 0xb3 }, 	/* "matrix coefficient 1" */
	{ 0x50, 0xb3 }, 	/* "matrix coefficient 2" */
	{ 0x51, 0    },		/* vb */
	{ 0x52, 0x3d }, 	/* "matrix coefficient 4" */
	{ 0x53, 0xa7 }, 	/* "matrix coefficient 5" */
	{ 0x54, 0xe4 }, 	/* "matrix coefficient 6" */
	{ REG_COM13, COM13_GAMMA|COM13_UVSAT|0x2 },  /* Magic rsvd bit */
	{ 0xff, 0xff },
};

struct regval_list ov7670_fmt_raw[] = {
	{ REG_COM7, COM7_BAYER },
	{ REG_COM13, 0x08 }, /* No gamma, magic rsvd bit */
	{ REG_COM16, 0x3d }, /* Edge enhancement, denoise */
	{ REG_REG76, 0xe1 }, /* Pix correction, magic rsvd */
	{ 0xff, 0xff },
};


/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

/*
 * QCIF mode is done (by OV) in a very strange way - it actually looks like
 * VGA with weird scaling options - they do *not* use the canned QCIF mode
 * which is allegedly provided by the sensor.  So here's the weird register
 * settings.
 */
struct regval_list ov7670_qcif_regs[] = {
	{ REG_COM3, COM3_SCALEEN|COM3_DCWEN },
	{ REG_COM3, COM3_DCWEN },
	{ REG_COM14, COM14_DCWEN | 0x01},
	{ 0x73, 0xf1 },
	{ 0xa2, 0x52 },
	{ 0x7b, 0x1c },
	{ 0x7c, 0x28 },
	{ 0x7d, 0x3c },
	{ 0x7f, 0x69 },
	{ REG_COM9, 0x38 },
	{ 0xa1, 0x0b },
	{ 0x74, 0x19 },
	{ 0x9a, 0x80 },
	{ 0x43, 0x14 },
	{ REG_COM13, 0xc0 },
	{ 0xff, 0xff },
};

struct ov7670_win_size {
	int	width;
	int	height;
	unsigned char com7_bit;
	int	hstart;		/* Start/stop values for the camera.  Note */
	int	hstop;		/* that they do not always make complete */
	int	vstart;		/* sense to humans, but evidently the sensor */
	int	vstop;		/* will do the right thing... */
	struct regval_list *regs; /* Regs to tweak */
};

struct ov7670_win_size ov7670_win_sizes[] = {
	/* VGA */
	{
		.width		= VGA_WIDTH,
		.height		= VGA_HEIGHT,
		.com7_bit	= COM7_FMT_VGA,
		.hstart		= 158,	/* These values from */
		.hstop		=  14,	/* Omnivision */
		.vstart		=  10,
		.vstop		= 490,
		.regs		= NULL,
	},
	/* CIF */
	{
		.width		= CIF_WIDTH,
		.height		= CIF_HEIGHT,
		.com7_bit	= COM7_FMT_CIF,
		.hstart		= 170,	/* Empirically determined */
		.hstop		=  90,
		.vstart		=  14,
		.vstop		= 494,
		.regs		= NULL,
	},
	/* QVGA */
	{
		.width		= QVGA_WIDTH,
		.height		= QVGA_HEIGHT,
		.com7_bit	= COM7_FMT_QVGA,
		.hstart		= 168,	/* Empirically determined */
		.hstop		=  24,
		.vstart		=  12,
		.vstop		= 492,
		.regs		= NULL,
	},
	/* QCIF */
	{
		.width		= QCIF_WIDTH,
		.height		= QCIF_HEIGHT,
		.com7_bit	= COM7_FMT_VGA, /* see comment above */
		.hstart		= 456,	/* Empirically determined */
		.hstop		=  24,
		.vstart		=  14,
		.vstop		= 494,
		.regs		= ov7670_qcif_regs,
	}
};



uint32_t dcmi_frame_buff[DCMI_FRAME_BUFF_MAX_SIZE];//	__attribute__ ((section(".bss")));

#ifdef DCMI_DEBUG
uint32_t dcmi_line_cnt = 0;
uint32_t dcmi_vsync_cnt = 0;
uint32_t dcmi_frame_cnt = 0;
uint32_t dcmi_reg_ndr   = 0;
uint32_t dcmi_error_code= 0;
#endif

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
	//hdma_eval.Init.Mode                = DMA_NORMAL;
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
#endif
  BSP_OV7670_Init( );

  current_resolution = CAMERA_R320x240;

#ifdef DCMI_DEBUG
  /* Initialize debug variables */
  dcmi_line_cnt = 0;
  dcmi_vsync_cnt = 0;
  dcmi_frame_cnt = 0;
  dcmi_reg_ndr = 0;
#endif
  
  return ret;
}

void BSP_OV7670_DISPLAY_FRAME( )
{
	lcd_copy_RGB565_frameBuffer( dcmi_frame_buff );
	return;
}

//Quick hack, approximately 1ms delay
static void ms_delay(int us)
{
   while (us-- > 0) {
      volatile int x=5971;	// this is for milli sec
      //volatile int x=597;		// this is for micro sec
      //volatile int x=30;		// this is for micro sec
      while (x-- > 0)
         __asm("nop");
   }
}

/*
 * Store a set of start/stop values into the camera.
 */
int ov7670_set_hw(int hstart, int hstop, int vstart, int vstop)
{
	int ret = 0;
	unsigned char v;
/*
 * Horizontal: 11 bits, top 8 live in hstart and hstop.  Bottom 3 of
 * hstart are in href[2:0], bottom 3 of hstop in href[5:3].  There is
 * a mystery "edge offset" value in the top two bits of href.
 */
	sccb_write_reg(REG_HSTART, (hstart >> 3) & 0xff);
	sccb_write_reg(REG_HSTOP, (hstop >> 3) & 0xff);
	sccb_read_reg(REG_HREF, &v);
	v = (v & 0xc0) | ((hstop & 0x7) << 3) | (hstart & 0x7);
	ms_delay(10);
	sccb_write_reg(REG_HREF, v);
/*
 * Vertical: similar arrangement, but only 10 bits.
 */
	sccb_write_reg(REG_VSTART, (vstart >> 2) & 0xff);
	sccb_write_reg(REG_VSTOP, (vstop >> 2) & 0xff);
	sccb_read_reg(REG_VREF, &v);
	v = (v & 0xf0) | ((vstop & 0x3) << 2) | (vstart & 0x3);
	ms_delay(10);
	sccb_write_reg(REG_VREF, v);
	return ret;
}

/*
 * Write a list of register settings; ff/ff stops the process.
 */
static int ov7670_write_array(struct regval_list *vals)
{
	while (vals->reg_num != 0xff || vals->value != 0xff) {
		sccb_write_reg(vals->reg_num, vals->value);
		vals++;
	}
	return 0;
}

/*
 * Stuff that knows about the sensor.
 */
static int ov7670_reset(void)
{
	sccb_write_reg(REG_COM7, COM7_RESET);
	ms_delay(1000);
	return 0;
}

void BSP_OV7670_Init( )
{
	ov7670_reset();

	ov7670_write_array(ov7670_default_regs);
	ov7670_write_array(ov7670_fmt_rgb565);
	/* Setting window size */
	//ov7670_write_array(ov7670_win_sizes[2]);
	ov7670_set_hw(
		ov7670_win_sizes[2].hstart, 
		ov7670_win_sizes[2].hstop,
		ov7670_win_sizes[2].vstart, 
		ov7670_win_sizes[2].vstop
	);

	ms_delay( 500);
#if 0
	sccb_write_reg( 0x12, 0x80 );   // COM7[7]=1: Resets all registers to default values
	sccb_write_reg( 0x12, 0x14 );   // COM7[2]=1: RGB selection
	                                // COM7[4]=1: Output format QVGA selection


	sccb_write_reg( 0x40, 0xd0 );   // COM7[7:6]=11: Output range 00 ~ FF
	                                // COM7[5:4]=01: RGB 565 format
#endif
#if 0
	sccb_write_reg( 0x17, 0x15 );   // 
	sccb_write_reg( 0x18, 0x03 );   // 
	sccb_write_reg( 0x32, 0x00 );   // 

	sccb_write_reg( 0x19, 0x03 );   // 
	sccb_write_reg( 0x1A, 0x6B );   // 
	sccb_write_reg( 0x03, 0x04 );   // 
#endif
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
#ifdef DCMI_DEBUG
	dcmi_line_cnt++;
#endif
}

/**
  * @brief  VSYNC event callback
  * @param  hdcmi: pointer to the DCMI handle  
  * @retval None
  */
void HAL_DCMI_VsyncEventCallback(DCMI_HandleTypeDef *hdcmi)
{        
  //BSP_CAMERA_VsyncEventCallback();
#ifdef DCMI_DEBUG
	dcmi_vsync_cnt++;
#endif
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
#ifdef DCMI_DEBUG
	dcmi_frame_cnt++;
	dcmi_reg_ndr = hdma_eval.Instance->NDTR;
#endif
}

/**
  * @brief  Error callback
  * @param  hdcmi: pointer to the DCMI handle  
  * @retval None
  */
void HAL_DCMI_ErrorCallback(DCMI_HandleTypeDef *hdcmi)
{        
	//BSP_LED_On(LED3);
	BSP_LED_Toggle(LED3);
  //BSP_CAMERA_ErrorCallback();
	dcmi_error_code = hdcmi->ErrorCode;
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




