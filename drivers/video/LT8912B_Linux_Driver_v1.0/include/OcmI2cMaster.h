#ifndef	  _OCMI2CMASTER_H_
#define	  _OCMI2CMASTER_H_


extern int HDMI_WriteI2C_Byte(u8 addr,u8 data);
extern int HDMI_ReadI2C_Byte(u8 addr);

extern u8 I2CADR;

#endif
