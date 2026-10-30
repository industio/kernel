
#ifndef		_LT8912B_H
#define		_LT8912B_H

#define _1080P_60Hz

/* output mode */
#define _hdmi_output
#define _lvds_bypass

//#define _pattern_test_

// #define _lane_swap_	//mipi input 3210
// #define _pn_swap_   //mipi input lane pn swap

#define dds_debug

/* lanes */
#define lane_cnt   0 /*0: 4lane; 1: 1lane; 2: 2lane; 3: 3lane;*/


typedef struct video_timing{
u16 hfp;
u16 hs;
u16 hbp;
u16 hact;
u16 htotal;
u16 vfp;
u16 vs;
u16 vbp;
u16 vact;
u16 vtotal;
bool h_polarity;
bool v_polarity;
u16	vic;
u8 aspact_ratio;  // 0=no data, 1=4:3, 2=16:9, 3=no data.
u32 pclk_khz;
}_video_timing;

typedef enum
{
 I2S_2CH,
 I2S_8CH,
 SPDIF
}_Audio_Input_Mode;

#define     Audio_Input_Mode    I2S_2CH

#define AR_ND 0x00
#define AR_4_3 0x01
#define AR_16_9 0x02 

typedef struct panel_parameter
{
		u16 hfp;
		u16 hs;
		u16 hbp;
		u16 hact;
		u16 htotal;
		u16 vfp;
		u16 vs;
		u16 vbp;
		u16 vact;
		u16 vtotal;
		u32 pclk_khz;
}_panel_parameter;

extern void LT8912B_Config(void);

extern u8 Tx_HPD;

#endif
