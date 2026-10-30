
#include "../include/include.h"

u8 Hsync_H_last = 0x00;
u8 Hsync_L_last = 0x00;
u8 Vsync_H_last = 0x00;
u8 Vsync_L_last = 0x00;

u8 Hsync_L, Hsync_H, Vsync_L, Vsync_H;


bool suspend_on = 0;

u8 Tx_HPD=0;

// hfp, hs, hbp, hact, htotal, vfp, vs, vbp, vact, vtotal, hfp, hs, hbp, hact, htotal, vfp, vs, vbp, vact, vtotal, h_polary, v_polary, vic, aspact_ratio, pclk_khz
struct video_timing video_640x480_60Hz     =    {16,  96, 48,  640,   800,   10, 2,  33,  480,  525,  0,0, 1,  AR_4_3, 25000};
struct video_timing video_720x480_60Hz     =    {16,  62, 60,  720,   858,   9,  6,  30,  480,  525,  0,0, 2,  AR_4_3, 27000};
struct video_timing video_720x576_50Hz     =    {12,  64, 68,  720,   864,   5,  5,  39,  576,  625,  0,0,17,  AR_4_3, 27000};
struct video_timing video_800x600_60Hz     =    {48, 128, 88,  800,  1056,   1,  4,  23,  600,  628,  1,1, 0, AR_16_9, 40000};
struct video_timing video_1024x768_60Hz    =    {24, 136, 160, 1024, 1344,   3,  6,  29,  768,  806,  0,0, 0, AR_16_9, 65000};
struct video_timing video_1280x720_60Hz    =    {110, 40, 220, 1280, 1650,   5,  5,  20,  720,  750,  1,1, 4, AR_16_9, 74250};
struct video_timing video_1280x800_60Hz    =    {28,  32, 100, 1280, 1440,   2,  6,  15,  800,  823,  0,0, 0, AR_16_9, 71000};
struct video_timing video_1920x1080_60Hz   =    {88,  44, 148, 1920, 2200,   4,  5,  36, 1080, 1125,  1,1,16, AR_16_9, 148500};

struct video_timing *video;


void DigitalClockEn(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x02,0xf7);
	HDMI_WriteI2C_Byte(0x08,0xff);
	HDMI_WriteI2C_Byte(0x09,0xff);
	HDMI_WriteI2C_Byte(0x0a,0xff);
	HDMI_WriteI2C_Byte(0x0b,0x7c);
	HDMI_WriteI2C_Byte(0x0c,0xff);
}
void TxAnalog(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x31,0xE1);
	HDMI_WriteI2C_Byte(0x32,0xE1);
	HDMI_WriteI2C_Byte(0x33,0x0c); //en/disable hdmid output
	HDMI_WriteI2C_Byte(0x37,0x00);
	HDMI_WriteI2C_Byte(0x38,0x22);
	HDMI_WriteI2C_Byte(0x60,0x82);
}

void CbusAnalog(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x39,0x45);
	HDMI_WriteI2C_Byte(0x3a,0x00);	//20180719
	HDMI_WriteI2C_Byte(0x3b,0x00);
}

void HDMIPllAnalog(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x44,0x31);
	HDMI_WriteI2C_Byte(0x55,0x44);
	HDMI_WriteI2C_Byte(0x57,0x01);
	HDMI_WriteI2C_Byte(0x5a,0x02);
}

void AviInfoframe(struct video_timing *video_format)
{
	u8 VIC = video_format->vic;
	u8 AR = video_format->aspact_ratio;
	u8 pb0,pb2,pb4,Sync_Polarity;
	
 	I2CADR = 0x94;
	HDMI_WriteI2C_Byte(0x3c,0x41); //enable null package
	
	Sync_Polarity = (video_format->h_polarity*0x02) + (video_format->v_polarity*0x01);
	pb2 =  (AR<<4) + 0x08;
	pb4 =  VIC;
	pb0 = (((pb2 + pb4) <= 0x5f)?(0x5f - pb2 - pb4):(0x15f - pb2 - pb4));
	
	I2CADR = 0x90;
    HDMI_WriteI2C_Byte(0xab,Sync_Polarity); //sync polarity
	
	I2CADR = 0x94;
	HDMI_WriteI2C_Byte(0x43,pb0);	  //PB0:check sum
	HDMI_WriteI2C_Byte(0x44,0x10);	//PB1:RGB888
	HDMI_WriteI2C_Byte(0x45,pb2);	  //PB2
	HDMI_WriteI2C_Byte(0x46,0x00);	//PB3
	HDMI_WriteI2C_Byte(0x47,pb4);	  //PB4:vic
}

void MipiAnalog(void)
{
	I2CADR = 0x90;
	#ifdef _pn_swap_
	HDMI_WriteI2C_Byte(0x3e,0xf6); //P/N swap
	#else
	HDMI_WriteI2C_Byte(0x3e,0xd6); //if mipi pin map follow reference design, no need swap P/N.
	#endif

	HDMI_WriteI2C_Byte(0x3f,0xd4);//EQ
	HDMI_WriteI2C_Byte(0x41,0x3c);//EQ
}

void MipiBasicSet(void)
{
	I2CADR = 0x92;
	HDMI_WriteI2C_Byte(0x10,0x01);    // term en
	HDMI_WriteI2C_Byte(0x11,0x10);    // settle
	//HDMI_WriteI2C_Byte(0x12,0x08);  // trail
	HDMI_WriteI2C_Byte(0x13,lane_cnt);  //00 4 lane //01 1 lane //02 2 lane //03 3lane
	HDMI_WriteI2C_Byte(0x14,0x00);  //debug mux

	#ifdef _lane_swap_  //for EVB only, if mipi pin map follow reference design, no need swap lane.
	HDMI_WriteI2C_Byte(0x15,0xa8);  //lane swap:3210
	printk("%s nmipi basic set: lane swap 3210, %d\n",LOG_FLAG,lane_cnt);
	#else
	HDMI_WriteI2C_Byte(0x15,0x00);  //lane swap:0123
	#endif

	HDMI_WriteI2C_Byte(0x1a,0x03);  // hshift 3
	HDMI_WriteI2C_Byte(0x1b,0x03);  // vshift 3
}

void MIPI_Video_Setup(struct video_timing *video_format)
{
	I2CADR = 0x92;
	HDMI_WriteI2C_Byte(0x18,(u8)(video_format->hs%256)); // hwidth
	HDMI_WriteI2C_Byte(0x19,(u8)(video_format->vs%256)); // vwidth
	HDMI_WriteI2C_Byte(0x1c,(u8)(video_format->hact%256)); // H_active[7:0]
	HDMI_WriteI2C_Byte(0x1d,(u8)(video_format->hact/256)); // H_active[15:8]
	HDMI_WriteI2C_Byte(0x2f,0x0c); // fifo_buff_length 12
	HDMI_WriteI2C_Byte(0x34,(u8)(video_format->htotal%256)); // H_total[7:0]
	HDMI_WriteI2C_Byte(0x35,(u8)(video_format->htotal/256)); // H_total[15:8]
	HDMI_WriteI2C_Byte(0x36,(u8)(video_format->vtotal%256)); // V_total[7:0]
	HDMI_WriteI2C_Byte(0x37,(u8)(video_format->vtotal/256)); // V_total[15:8]
	HDMI_WriteI2C_Byte(0x38,(u8)(video_format->vbp%256)); // VBP[7:0]
	HDMI_WriteI2C_Byte(0x39,(u8)(video_format->vbp/256)); // VBP[15:8]
	HDMI_WriteI2C_Byte(0x3a,(u8)(video_format->vfp%256)); // VFP[7:0]
	HDMI_WriteI2C_Byte(0x3b,(u8)(video_format->vfp/256)); // VFP[15:8]
	HDMI_WriteI2C_Byte(0x3c,(u8)(video_format->hbp%256)); // HBP[7:0]
	HDMI_WriteI2C_Byte(0x3d,(u8)(video_format->hbp/256)); // HBP[15:8]
	HDMI_WriteI2C_Byte(0x3e,(u8)(video_format->hfp%256)); // HFP[7:0]
	HDMI_WriteI2C_Byte(0x3f,(u8)(video_format->hfp/256)); // HFP[15:8]
    video = video_format;
}

void MIPIRxLogicRes(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x03,0x7f);	 //mipi rx reset
	msleep(10);
	HDMI_WriteI2C_Byte(0x03,0xff);

	HDMI_WriteI2C_Byte(0x05,0xfb);	 //dds reset
	msleep(10);
	HDMI_WriteI2C_Byte(0x05,0xff);
}

void DDSConfig(void)
{
	I2CADR = 0x92;
	HDMI_WriteI2C_Byte(0x4e,0x93);//strm_sw_freq_word[ 7: 0]
	HDMI_WriteI2C_Byte(0x4f,0x3E);//strm_sw_freq_word[15: 8]
	HDMI_WriteI2C_Byte(0x50,0x29);//strm_sw_freq_word[23:16]
	HDMI_WriteI2C_Byte(0x51,0x80);//[0]=strm_sw_freq_word[24]
	HDMI_WriteI2C_Byte(0x1e,0x4f);
	HDMI_WriteI2C_Byte(0x1f,0x5e);//full_value   464
	HDMI_WriteI2C_Byte(0x20,0x01);
	HDMI_WriteI2C_Byte(0x21,0x2c);//full_value1  416
	HDMI_WriteI2C_Byte(0x22,0x01);
	HDMI_WriteI2C_Byte(0x23,0xfa);//full_value2  400
	HDMI_WriteI2C_Byte(0x24,0x00);
	HDMI_WriteI2C_Byte(0x25,0xc8);//full_value3  384
	HDMI_WriteI2C_Byte(0x26,0x00);
	HDMI_WriteI2C_Byte(0x27,0x5e);//empty_value   464
	HDMI_WriteI2C_Byte(0x28,0x01);
	HDMI_WriteI2C_Byte(0x29,0x2c);//empty_value1  416
	HDMI_WriteI2C_Byte(0x2a,0x01);
	HDMI_WriteI2C_Byte(0x2b,0xfa);//empty_value2  400
	HDMI_WriteI2C_Byte(0x2c,0x00);
	HDMI_WriteI2C_Byte(0x2d,0xc8);//empty_value3  384
	HDMI_WriteI2C_Byte(0x2e,0x00);
	HDMI_WriteI2C_Byte(0x42,0x64);//tmr_set[ 7:0]:100us
	HDMI_WriteI2C_Byte(0x43,0x00);//tmr_set[15:8]:100us
	HDMI_WriteI2C_Byte(0x44,0x04);//timer step
	HDMI_WriteI2C_Byte(0x45,0x00);
	HDMI_WriteI2C_Byte(0x46,0x59);
	HDMI_WriteI2C_Byte(0x47,0x00);
	HDMI_WriteI2C_Byte(0x48,0xf2);
	HDMI_WriteI2C_Byte(0x49,0x06);
	HDMI_WriteI2C_Byte(0x4a,0x00);
	HDMI_WriteI2C_Byte(0x4b,0x72);
	HDMI_WriteI2C_Byte(0x4c,0x45);
	HDMI_WriteI2C_Byte(0x4d,0x00);
	HDMI_WriteI2C_Byte(0x52,0x08);// trend step
	HDMI_WriteI2C_Byte(0x53,0x00);
	HDMI_WriteI2C_Byte(0x54,0xb2);
	HDMI_WriteI2C_Byte(0x55,0x00);
	HDMI_WriteI2C_Byte(0x56,0xe4);
	HDMI_WriteI2C_Byte(0x57,0x0d);
	HDMI_WriteI2C_Byte(0x58,0x00);
	HDMI_WriteI2C_Byte(0x59,0xe4);
	HDMI_WriteI2C_Byte(0x5a,0x8a);
	HDMI_WriteI2C_Byte(0x5b,0x00);
	HDMI_WriteI2C_Byte(0x5c,0x34);
	HDMI_WriteI2C_Byte(0x51,0x00);
}

void AudioIIsEn(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0xB2,0x01);//DVI mode:0x00; HDMI mode:0x01;
	I2CADR = 0x94;
	HDMI_WriteI2C_Byte(0x06,0x08);
	HDMI_WriteI2C_Byte(0x07,0xF0);
	HDMI_WriteI2C_Byte(0x34,0xD2);//0xE2:32FS; 0xD2:64FS
	HDMI_WriteI2C_Byte(0x0F,0x2B);
}

void AudioSpdifEn(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0xB2,0x01);
	I2CADR = 0x94;
	HDMI_WriteI2C_Byte(0x06,0x0e);
	HDMI_WriteI2C_Byte(0x07,0x00);
	HDMI_WriteI2C_Byte(0x34,0xD2);//0xE2:32FS; 0xD2:64FS
}

void Core_Pll_setup(struct panel_parameter *panel)
{
	u8 cpll_m, cpll_k1,cpll_k2; //cpll_k3;
	u32 temp;
	
	temp=(panel->pclk_khz*7)/25;
	cpll_m=temp/1000;
	
	temp=(panel->pclk_khz*7)/25;
	temp=temp%1000;
	//temp=temp*16.384;
    temp=temp*16384/1000;
	
	cpll_k1=temp%256;
	cpll_k2=temp/256;
	
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x50,0x24);//cp=50uA
	HDMI_WriteI2C_Byte(0x51,0x05);//xtal_clk as reference,second order passive LPF PLL
	HDMI_WriteI2C_Byte(0x52,0x14);//use second-order PLL
	HDMI_WriteI2C_Byte(0x69,cpll_m);//CP_PRESET_DIV_RATIO
	HDMI_WriteI2C_Byte(0x69,(cpll_m|0x80));
	HDMI_WriteI2C_Byte(0x6c,(cpll_k2|0x80));//RGD_CP_SOFT_K_EN,RGD_CP_SOFT_K[13:8]
	HDMI_WriteI2C_Byte(0x6b,cpll_k1);

	HDMI_WriteI2C_Byte(0x04,0xfb);//core pll reset
	HDMI_WriteI2C_Byte(0x04,0xff);
}

void Core_Pll_bypass(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x50,0x24); //cp=50uA
	HDMI_WriteI2C_Byte(0x51,0x2d); //Pix_clk as reference,second order passive LPF PLL
	HDMI_WriteI2C_Byte(0x52,0x04); //loopdiv=0;use second-order PLL
	HDMI_WriteI2C_Byte(0x69,0x0e); //CP_PRESET_DIV_RATIO
	HDMI_WriteI2C_Byte(0x69,0x8e);
	HDMI_WriteI2C_Byte(0x6a,0x00);
	HDMI_WriteI2C_Byte(0x6c,0xb8); //RGD_CP_SOFT_K_EN,RGD_CP_SOFT_K[13:8]
	HDMI_WriteI2C_Byte(0x6b,0x51);

	HDMI_WriteI2C_Byte(0x04,0xfb); //core pll reset
	HDMI_WriteI2C_Byte(0x04,0xff);
}

void Lvds_Pll_Reset(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x02,0xf7); //lvds pll reset
	HDMI_WriteI2C_Byte(0x02,0xff);
}


void Scaler_bypass(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x7f,0x00);//disable scaler
	HDMI_WriteI2C_Byte(0xa8,0x13);
}

void Scaler_setup(struct video_timing *input_video,struct panel_parameter *panel)
{
	//for example: 720P to 1280x800
	//These register base on MIPI resolution and LVDS panel resolution.
	u32 h_ratio,v_ratio;
	u8 i;
	u32 htotal;
//	h_ratio = input_video->hact*4096.00/panel->hact;
//	v_ratio = input_video->vact*4096.00/panel->vact;
    h_ratio = input_video->hact*4096/panel->hact;
	v_ratio = input_video->vact*4096/panel->vact;
	
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x80,0x00);
	HDMI_WriteI2C_Byte(0x81,0xff);
	HDMI_WriteI2C_Byte(0x82,0x03);
	HDMI_WriteI2C_Byte(0x83,(u8)(input_video->hact%256));
	HDMI_WriteI2C_Byte(0x84,(u8)(input_video->hact/256));
	HDMI_WriteI2C_Byte(0x85,0x80);
	HDMI_WriteI2C_Byte(0x86,0x10);
	HDMI_WriteI2C_Byte(0x87,(u8)(panel->htotal%256));
	HDMI_WriteI2C_Byte(0x88,(u8)(panel->htotal/256));
	HDMI_WriteI2C_Byte(0x89,(u8)(panel->hs%256));
	HDMI_WriteI2C_Byte(0x8a,(u8)(panel->hbp%256));
	HDMI_WriteI2C_Byte(0x8b,(u8)(panel->vs%256));
	HDMI_WriteI2C_Byte(0x8c,(u8)(panel->hact%256));
	HDMI_WriteI2C_Byte(0x8d,(u8)(panel->vact%256));
	HDMI_WriteI2C_Byte(0x8e,(u8)(panel->vact/256)*16+(panel->hact/256));
	HDMI_WriteI2C_Byte(0x8f,(u8)(h_ratio%256));
	HDMI_WriteI2C_Byte(0x90,(u8)(h_ratio/256));
	HDMI_WriteI2C_Byte(0x91,(u8)(v_ratio%256));
	HDMI_WriteI2C_Byte(0x92,(u8)(v_ratio/256));
	HDMI_WriteI2C_Byte(0x7f,0x96);
	//HDMI_WriteI2C_Byte(0x7f,0xb0);
	//HDMI_WriteI2C_Byte(0xa8,0x3b);
	HDMI_WriteI2C_Byte(0xa8,0x13);

	HDMI_WriteI2C_Byte(0x02,0xf7);//lvds pll reset
	HDMI_WriteI2C_Byte(0x02,0xff);

	HDMI_WriteI2C_Byte(0x03,0xcf);//scaler reset
	HDMI_WriteI2C_Byte(0x03,0xff);

    HDMI_WriteI2C_Byte(0x7f,0xb0);

	for(i = 0; i < 300; i++)
	{
	   if(HDMI_ReadI2C_Byte(0xa7)&0x20)
	   {																		
		 htotal = (HDMI_ReadI2C_Byte(0xa7)&0x0f)*0x100 + HDMI_ReadI2C_Byte(0xa6);
		 printk("%s scaler setup htotal = %d.\n",LOG_FLAG,htotal);

		 break;
		 //please set "htotal" to panel_parameter's htotal.
	   }
	 printk("%s scaler loop = %d\n",LOG_FLAG, i);
	 msleep(100);
	}
}
void LvdsPowerUp(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x44,0x30);
	HDMI_WriteI2C_Byte(0x51,0x05);
}
void LvdsPowerDown(void)
{
	I2CADR = 0x90;
	//HDMI_WriteI2C_Byte(0x44,0x31);
	HDMI_WriteI2C_Byte(0x51,0x15); //
	//HDMI_WriteI2C_Byte(0x03,0xfb);
}

void LvdsBypass(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x50,0x24); //cp=50uA
	HDMI_WriteI2C_Byte(0x51,0x2d); //Pix_clk as reference,second order passive LPF PLL
	HDMI_WriteI2C_Byte(0x52,0x04); //loopdiv=0;use second-order PLL
	HDMI_WriteI2C_Byte(0x69,0x0e); //CP_PRESET_DIV_RATIO
	HDMI_WriteI2C_Byte(0x69,0x8e);
	HDMI_WriteI2C_Byte(0x6a,0x00);
	HDMI_WriteI2C_Byte(0x6c,0xb8); //RGD_CP_SOFT_K_EN,RGD_CP_SOFT_K[13:8]
	HDMI_WriteI2C_Byte(0x6b,0x51);

	HDMI_WriteI2C_Byte(0x04,0xfb); //core pll reset
	HDMI_WriteI2C_Byte(0x04,0xff);

	HDMI_WriteI2C_Byte(0x7f,0x00); //disable scaler
	HDMI_WriteI2C_Byte(0xa8,0x13); //0x13:VSEA ; 0x33:JEIDA;
}

void LvdsOutput(bool on)
{
    if(on)
    {
        I2CADR = 0x90;
        HDMI_WriteI2C_Byte(0x02,0xf7); //lvds pll reset
        HDMI_WriteI2C_Byte(0x02,0xff); //scaler module reset
        HDMI_WriteI2C_Byte(0x03,0xcb); //lvds tx module reset
		HDMI_WriteI2C_Byte(0x03,0xfb);
		HDMI_WriteI2C_Byte(0x03,0xff);
		HDMI_WriteI2C_Byte(0x44,0x30); //enbale lvds output
		printk("%s LT8912_lvds_output_enable!\n",LOG_FLAG);

 	}else{
   
 		I2CADR = 0x90;
 		HDMI_WriteI2C_Byte(0x44,0x31);
 	}
}


void HdmiOutput(bool on)
{
	if(on)
	{
			I2CADR = 0x90;
			HDMI_WriteI2C_Byte(0x33,0x0e); //enable hdmi output
            printk("%s HDMI output Enable.\n",LOG_FLAG);
	}
	else
	{
			I2CADR = 0x90;
			HDMI_WriteI2C_Byte(0x33,0x0c); //disable hdmi output
            printk("%s HDMI output Disable.\n",LOG_FLAG);
	}	
}


void LvdsScalerResult(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x7f,0xb0);
}


void ScalerReset(void)
{
	I2CADR = 0x90;
	HDMI_WriteI2C_Byte(0x03,0xcf);
	HDMI_WriteI2C_Byte(0x03,0xff);
}


void lt8912_check_dds(void)
{
	u8 reg_920c, reg_920d, reg_920e, reg_920f;
	u8 i;
	for(i = 0; i < 10; i++)
	{
		I2CADR = 0x92;
		reg_920c = HDMI_ReadI2C_Byte(0x0c);
		reg_920d = HDMI_ReadI2C_Byte(0x0d);
		reg_920e = HDMI_ReadI2C_Byte(0x0e);
		reg_920f = HDMI_ReadI2C_Byte(0x0f);
		printk("%s 0x0c~0f = %02x, %02x, %02x\n",LOG_FLAG,reg_920c, reg_920d, reg_920e);
//		if((reg_920e == 0xd2)&&(reg_920d < 0xff)&&(reg_920d > 0xd0)) //shall update threshold here base on actual dds result.
//		{
//			printk("%s lvds_check_dds: stable!\n",LOG_FLAG);
//			break;
//		}

		msleep(500);
	}
}

void lvds_output_cfg(void)
{
	LvdsPowerUp();
	#ifdef _lvds_bypass
	LvdsBypass();
	#else
	//Core_Pll_setup(&AA070M0C1);
	Core_Pll_setup(&KD101FM_1_C001A);
	Scaler_setup(&video_1280x720_60Hz,&KD101FM_1_C001A);
	#endif
}

void MIPI_Input_det(void)
{
	I2CADR = 0x90;
	Hsync_L = HDMI_ReadI2C_Byte(0x9c);
	Hsync_H = HDMI_ReadI2C_Byte(0x9d);
	Vsync_L = HDMI_ReadI2C_Byte(0x9e);
	Vsync_H = HDMI_ReadI2C_Byte(0x9f);
	
	if((Hsync_H!=Hsync_H_last)||(Vsync_H!=Vsync_H_last)) /*hiht byte changed*/
	{
		printk("%s 0x9c~9f = %x, %x, %x, %x.\n", LOG_FLAG,Hsync_H, Hsync_L,Vsync_H, Vsync_L);
	
        if(Vsync_H==0x02 && Vsync_L <= 0xef&&Vsync_L >= 0xec)//0x2EE
		{
			video = &video_1280x720_60Hz;
			printk("%s videoformat = VESA_1280x720_60\n",LOG_FLAG);
		}
		else if(Vsync_H == 0x03 && Vsync_L <= 0x3a &&Vsync_L >= 0x34)//0x337
		{
			video = &video_1280x800_60Hz;
			printk("%s videoformat = VESA_1280x800_60\n",LOG_FLAG);
		}	
		else if(Vsync_H == 0x04 && Vsync_L <= 0x67&&Vsync_L >= 0x63)//0x465
		{
			video = &video_1920x1080_60Hz;
			printk("%s videoformat = VESA_1920x1080_60\n",LOG_FLAG);
		}
		else
		{
			printk("%s videoformat = video_none\n",LOG_FLAG);
		}
		
	  Hsync_L_last = Hsync_L;
	  Hsync_H_last = Hsync_H;
	  Vsync_L_last = Vsync_L;
	  Vsync_H_last = Vsync_H;

      MIPI_Video_Setup(video);
      AviInfoframe(video);
      MIPIRxLogicRes();
  }

}


//void dds_clock_debug(void)
//{
//	#ifdef dds_debug
//	u8 reg_920c, reg_920d, reg_920e, reg_920f;
//	
//	while(1)
//	{
//		I2CADR = 0x92;
//		reg_920c = HDMI_ReadI2C_Byte(0x0c);
//		reg_920d = HDMI_ReadI2C_Byte(0x0d);
//		reg_920e = HDMI_ReadI2C_Byte(0x0e);
//		reg_920f = HDMI_ReadI2C_Byte(0x0f);
//	
//		printk("\r\n0x0c~0e = %02x, %02x, %02x",reg_920c, reg_920d, reg_920e);
//		printk("\r\nEnter the dds_clock_debug cycle");
//		
//		msleep(1000);
//	}
//	#endif
//}

void pattern_test(void)
{
    u32 DDS_initial_value;
	u8 pclk_Mhz;
	
//	video = &video_640x480_60Hz;
//	video = &video_720x480_60Hz;
//	video = &video_800x600_60Hz;
//	video = &video_1280x720_60Hz;
//	video = &video_1280x800_60Hz;
	video = &video_1920x1080_60Hz;
	
    DigitalClockEn();
	TxAnalog();
	CbusAnalog();
	HDMIPllAnalog();
	AudioIIsEn();
	AviInfoframe(video);
//	lvds_output_cfg();

	I2CADR = 0x92;
	//////////////////Pattern resolution set////////////////////
	HDMI_WriteI2C_Byte(0x72,0x12);
	HDMI_WriteI2C_Byte(0x73,(u8)((video->hs+video->hbp)%256)); //RGD_PTN_DE_DLY[7:0]
	HDMI_WriteI2C_Byte(0x74,(u8)((video->hs+video->hbp)/256)); //RGD_PTN_DE_DLY[11:8]
	HDMI_WriteI2C_Byte(0x75,(u8)((video->vs+video->vbp)%256)); //RGD_PTN_DE_TOP[6:0]
	HDMI_WriteI2C_Byte(0x76,(u8)(video->hact%256)); //RGD_PTN_DE_CNT[7:0]
	HDMI_WriteI2C_Byte(0x77,(u8)(video->vact%256)); //RGD_PTN_DE_LIN[7:0]
	HDMI_WriteI2C_Byte(0x78,((((u8)(video->vact/256))<<4)+((u8)(video->hact/256)))); //RGD_PTN_DE_LIN[10:8],RGD_PTN_DE_CNT[11:8]
	HDMI_WriteI2C_Byte(0x79,(u8)(video->htotal%256)); //RGD_PTN_H_TOTAL[7:0]
	HDMI_WriteI2C_Byte(0x7a,(u8)(video->vtotal%256)); //RGD_PTN_V_TOTAL[7:0]
	HDMI_WriteI2C_Byte(0x7b,((((u8)(video->vtotal/256))<<4)+((u8)(video->htotal/256)))); //RGD_PTN_V_TOTAL[10:8],RGD_PTN_H_TOTAL[11:8]
	HDMI_WriteI2C_Byte(0x7c,(u8)(video->hs%256)); //RGD_PTN_HWIDTH[7:0]
	HDMI_WriteI2C_Byte(0x7d,((((u8)(video->hs/256))<<6)+((u8)(video->vs%256)))); //RGD_PTN_HWIDTH[9:8],RGD_PTN_VWIDTH[5:0]
	HDMI_WriteI2C_Byte(0x70,0x80); //pattern enable
	HDMI_WriteI2C_Byte(0x71,0x51);
	HDMI_WriteI2C_Byte(0x42,0x12);
	///////////////////////////////////////////////////////////
	
	HDMI_WriteI2C_Byte(0x1e,0x67); //h v d pol hdmi sel pll sel
	
	//////////////////pattern pixl clock set///////////////////
	pclk_Mhz = (u8)(video->pclk_khz/1000);
	DDS_initial_value = (u32)(pclk_Mhz*0x16C16);
	HDMI_WriteI2C_Byte(0x4e,(u8)(DDS_initial_value&0x000000ff)); //strm_sw_freq_word[ 7: 0]
	HDMI_WriteI2C_Byte(0x4f,(u8)((DDS_initial_value&0x0000ff00)>>8)); //strm_sw_freq_word[15: 8]
	HDMI_WriteI2C_Byte(0x50,(u8)((DDS_initial_value&0x00ff0000)>>16)); //strm_sw_freq_word[23:16]
	HDMI_WriteI2C_Byte(0x51,0x80); 

//  HDMI_WriteI2C_Byte(0x4e,0x3E); //strm_sw_freq_word[ 7: 0]
//	HDMI_WriteI2C_Byte(0x4f,0xE9); //strm_sw_freq_word[15: 8]
//	HDMI_WriteI2C_Byte(0x50,0xD3); //strm_sw_freq_word[23:16]
//	HDMI_WriteI2C_Byte(0x51,0x80); //pattern en
	///////////////////////////////////////////////////////////

	HdmiOutput(1);
//	LvdsOutput(1);
}


u8 LT8912_Get_HPD(void) 
{ 
    //(0x90)0xc1[bit7] description HPD is High or Low 
    I2CADR = 0x90;
    if((HDMI_ReadI2C_Byte(0xc1)&0x80)==0x80)
	{
//		printk("%s LT8912_Get_HPD: high\n",LOG_FLAG);
		return 1;
	}
    else
	{
//		printk("%s LT8912_Get_HPD: low\n",LOG_FLAG);
		return 0;
	}
} 

void LT8912B_chip_ID(void)
{
	I2CADR=0x90;
	printk("%s chip ID: 0x%x, 0x%x\n",LOG_FLAG,HDMI_ReadI2C_Byte(0x00), HDMI_ReadI2C_Byte(0x01));
}


void LT8912B_Suspend(bool on)
{
    //9mA,HPD detect is normal.
	if(on)
	{
	    if(!suspend_on)
        {
		    // enter suspend mode
		    I2CADR = 0x90;
		    HDMI_WriteI2C_Byte( 0x54, 0x1d );
		    HDMI_WriteI2C_Byte( 0x51, 0x15 );
		    HDMI_WriteI2C_Byte( 0x44, 0x31 );
		    HDMI_WriteI2C_Byte( 0x41, 0xbd );
		    HDMI_WriteI2C_Byte( 0x5c, 0x11 );
		    suspend_on = 1;
			printk("%s suspend on.\n",LOG_FLAG);
		}

	}else{

	    if(suspend_on)
        {
		    //exist suspend mode
		    I2CADR = 0x90;
            HDMI_WriteI2C_Byte( 0x5c, 0x10 );
		  	HDMI_WriteI2C_Byte( 0x54, 0x1c );
		    HDMI_WriteI2C_Byte( 0x51, 0x2d );
		    HDMI_WriteI2C_Byte( 0x44, 0x30 );
		    HDMI_WriteI2C_Byte( 0x41, 0xbc );
	
		  	msleep(10);
		    HDMI_WriteI2C_Byte(0x03,0x7f);
		    msleep(10);
            HDMI_WriteI2C_Byte(0x03,0xff); 

		    HDMI_WriteI2C_Byte(0x05,0xfb);
		    msleep(10);
            HDMI_WriteI2C_Byte(0x05,0xff);
            suspend_on = 0;
            printk("%s suspend off.\n",LOG_FLAG);
		 }
	}
}


void LT8912B_Config(void)
{
	LT8912B_chip_ID();
	
    #ifdef _pattern_test_
	pattern_test();
	printk("===== step : pattern_test =====\n");
    while(Main_Loop){
    	msleep(1000);
    };
    #else
	DigitalClockEn();
	TxAnalog();
	CbusAnalog();
	HDMIPllAnalog();
	MipiAnalog();
	MipiBasicSet();
	DDSConfig();
	MIPI_Video_Setup(&video_1920x1080_60Hz);
	MIPI_Input_det();
	AudioIIsEn();

	
    #ifdef dds_debug
    lt8912_check_dds(); // can be removed when debug stage.
    #endif
	
	#ifdef _lvds_bypass
	lvds_output_cfg();
	LvdsOutput(1);
	#endif
	
/////////////////////////////////////////////////////////////	test use
//	HdmiOutput(1);
//	while(1);
/////////////////////////////////////////////////////////////	
	
    #ifdef _hdmi_output
    if(LT8912_Get_HPD()) 
    {
	   if(!Tx_HPD)
	   {
          HdmiOutput(1);
          Tx_HPD = 1;
//          printk("%s LT8912_Get_HPD: high\n",LOG_FLAG);
	   }

	}else{

	   if(Tx_HPD)
	   {
          LT8912B_Suspend(1);
//          printk("%s LT8912 Suspend!\n",LOG_FLAG);
	  	  HdmiOutput(0);
          Tx_HPD = 0;
//          printk("%s LT8912_Get_HPD: low\n",LOG_FLAG);
	   }
	}
   #endif
	
   #endif

    while(Main_Loop)
	{
		  MIPI_Input_det();
		  msleep(300);

	      #ifdef _hdmi_output
		  if(LT8912_Get_HPD()) 
		  {
		  	  if(!Tx_HPD)
		  	  {
	             LT8912B_Suspend(0);
	             HdmiOutput(1);
	             Tx_HPD = 1;
		  	   }

		  }else{

		  	  if(Tx_HPD)
		  	  {
	             LT8912B_Suspend(1);
	             HdmiOutput(0);
	             Tx_HPD = 0;
		  	  }
		  }
		 #endif
	}
}
