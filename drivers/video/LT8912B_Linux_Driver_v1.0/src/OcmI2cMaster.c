#include "../include/include.h"





int HDMI_WriteI2C_Byte(u8 addr,u8 data)
{
	int ret = 0;
	mutex_lock(&lt8912b->ocm_lock);
	
    if(I2CADR==0x90)
    {
		ret = regmap_write(lt8912b->chip_regmap, addr, data);
		if (ret < 0) {
			printk("regmap_write error: (0x%02x 0x%02x 0x%02x) %d",
				lt8912b->i2c_stu->addr, addr, data, ret);
            mutex_unlock(&lt8912b->ocm_lock);
			return ret;
		}
    }else if(I2CADR==0x92){
        ret = regmap_write(lt8912b->addr_1->regmap, addr, data);
		if (ret < 0) {
			printk("regmap_write error: (0x%02x 0x%02x 0x%02x) %d",
				lt8912b->addr_1->i2c_stu->addr, addr, data, ret);
            mutex_unlock(&lt8912b->ocm_lock);
			return ret;
		} 

    }else if(I2CADR==0x94){
        ret = regmap_write(lt8912b->addr_2->regmap, addr, data);
		if (ret < 0) {
			printk("regmap_write error: (0x%02x 0x%02x 0x%02x) %d",
				lt8912b->addr_2->i2c_stu->addr, addr, data, ret);
            mutex_unlock(&lt8912b->ocm_lock);
			return ret;
		}
    }

        printk(KERN_INFO "LT8912B I2C W: [0x%02X] Reg=0x%02X, Val=0x%02X\n", I2CADR, addr, data);
	
    mutex_unlock(&lt8912b->ocm_lock);
	return 0;
}





int HDMI_ReadI2C_Byte(u8 addr)
{
	int ret = 0;
	unsigned int  val = 0;
    mutex_lock(&lt8912b->ocm_lock);

    if(I2CADR==0x90){

		ret = regmap_read(lt8912b->chip_regmap, addr, &val);
		if (ret < 0) {
			printk("regmap_read error: (0x%02x 0x%02x) %d",
			lt8912b->i2c_stu->addr, addr, ret);
			mutex_unlock(&lt8912b->ocm_lock);
			return ret;
		}
    }else if(I2CADR==0x92){

        ret = regmap_read(lt8912b->addr_1->regmap, addr, &val);
		if (ret < 0) {
			printk("regmap_read error: (0x%02x 0x%02x) %d",
			lt8912b->addr_1->i2c_stu->addr, addr, ret);
			mutex_unlock(&lt8912b->ocm_lock);
			return ret;
		}

    }else if(I2CADR==0x94){

        ret = regmap_read(lt8912b->addr_2->regmap, addr, &val);
		if (ret < 0) {
			printk("regmap_read error: (0x%02x 0x%02x) %d",
			lt8912b->addr_2->i2c_stu->addr, addr, ret);
			mutex_unlock(&lt8912b->ocm_lock);
			return ret;
		}

    }

    mutex_unlock(&lt8912b->ocm_lock);
    return (u8)val;
}


EXPORT_SYMBOL(HDMI_WriteI2C_Byte);
EXPORT_SYMBOL(HDMI_ReadI2C_Byte);
