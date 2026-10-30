#ifndef _LONTIUMDRV_H_
#define _LONTIUMDRV_H_


struct _lt8912b{
    struct device *dev;
	struct mutex ocm_lock;
	struct gpio_desc *reset_gpio;
    struct gpio_desc *power_gpio;
	struct i2c_client *i2c_stu;
	struct regmap *chip_regmap;
    struct addr_1{
        struct i2c_client *i2c_stu;
        struct regmap *regmap;
    }*addr_1;
    struct addr_2{
        struct i2c_client *i2c_stu;
        struct regmap *regmap;
    }*addr_2;

};



typedef struct
{
    u8 address;
    u8 value;
} Chip_Control_Args;


extern struct _lt8912b  *lt8912b;
extern volatile  bool  Main_Loop;
extern const char *LOG_FLAG;

extern u8 addr_1;
extern u8 addr_2;

#endif
