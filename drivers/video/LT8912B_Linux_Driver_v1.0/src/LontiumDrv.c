#include "../include/include.h"


struct _lt8912b  *lt8912b;
static struct task_struct *kthread_obj;
volatile  bool Main_Loop=true;
char *ChipName = "lt8912b";                    //Define Lontium Chip Name
static int major = 0;
static struct cdev chip_cdev;
static struct class *chip_class;
const char *LOG_FLAG = "LT8912B";

u8 I2CADR;
u8  addr_1_value = 0x00;
u8  addr_2_value = 0x00;


static int chip_open(struct inode *inode, struct file *filp)
{
    printk("chip open\n"); 
    return 0;
}


static int chip_close (struct inode *node, struct file *file)
{
    printk("chip close\n");
	return 0;
}


static ssize_t chip_read(struct file *filp, char __user *buf, size_t cnt, loff_t *offt)
{
	Chip_Control_Args parameter;
		
	if (copy_from_user(&parameter, buf, sizeof(parameter))) {
		return -EFAULT;
	}
		
	parameter.value = HDMI_ReadI2C_Byte(parameter.address);
	
	if (copy_to_user(buf, &parameter, sizeof(parameter))) {
		return -EFAULT;
	}
	
	return sizeof(parameter);

}

static ssize_t chip_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *offt)
{
    Chip_Control_Args parameter;
	
    if (copy_from_user(&parameter, buf, sizeof(parameter))) {
         return -EFAULT;
     }

	HDMI_WriteI2C_Byte(parameter.address, parameter.value);
	
	return sizeof(parameter);
	
}




static struct file_operations chip_drv = {
	.owner	 = THIS_MODULE,
	.open    = chip_open,
	.release = chip_close,
	.read    = chip_read,
	.write   = chip_write,
};



static int  chip_dev_init(void)
{
	int ret;
	dev_t devid;
	
	ret = alloc_chrdev_region(&devid, 0, 1, ChipName);
	if (ret < 0) {
	    return ret;
	}
	
	major = MAJOR(devid);
	cdev_init(&chip_cdev, &chip_drv);
	cdev_add(&chip_cdev, devid, 1);
	chip_class = class_create(THIS_MODULE, "chip_class");
	if (IS_ERR(chip_class)) {
		unregister_chrdev(major, ChipName);
		return -1;
	}
	
	device_create(chip_class, NULL, MKDEV(major, 0), NULL, ChipName); /* /dev/ChipName */
	
	return 0;
}


static void  chip_dev_exit(void)
{
	device_destroy(chip_class, MKDEV(major, 0));
	class_destroy(chip_class);
	cdev_del(&chip_cdev);
	unregister_chrdev_region(MKDEV(major,0), 1);	
}



static const struct regmap_range chip_ranges[] = {
	{ .range_min = 0, .range_max = 0xffff },
};

static const struct regmap_access_table chip_table = {
	.yes_ranges = chip_ranges,
	.n_yes_ranges = ARRAY_SIZE(chip_ranges),
};

static const struct regmap_config chip_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
	.volatile_table = &chip_table,
	.cache_type = REGCACHE_NONE,
};



static int chip_parse_dts(struct device *dev)
{

    /*
     * devm_gpiod_get_optional  - If this fails, NULL is returned and no error pointer.
     *
	 * devm_gpiod_get  -  If this fails, an error pointer is returned, usually ERR_PTR(-ENOENT).
	 *
	 * The devm_gpiod_get function is recommended for practical use.
     */

	
	//Gets the handle to power gpio in dts 
	 
    lt8912b->power_gpio=devm_gpiod_get_optional(dev, "power", GPIOD_OUT_HIGH);
	if (IS_ERR(lt8912b->power_gpio)) {
	  dev_err(dev, "Failed to get power GPIO\n");
      return PTR_ERR(lt8912b->power_gpio);
	}


    //Gets the handle to reset gpio in dts
	  
	lt8912b->reset_gpio=devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(lt8912b->reset_gpio)) {
		dev_err(dev, "Failed to get reset GPIO\n");
		return PTR_ERR(lt8912b->reset_gpio);
	}


	// Additional GPIO pins can be added here if needed

	return 0;
}


static int chip_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct i2c_board_info addr_1_board_info = {
        I2C_BOARD_INFO("lt8912b_addr_1", addr_1_value), 
    };
    struct i2c_board_info addr_2_board_info = {
        I2C_BOARD_INFO("lt8912b_addr_2", addr_2_value), 
    };
    struct device_node *np = client->dev.of_node;
    struct device_node *addr_1_node;
    struct device_node *addr_2_node;
	int ret;

    if (!np) {
        dev_err(&client->dev, "Device tree node missing");
        return -EINVAL;
    }
	
    chip_dev_init();

    lt8912b = devm_kzalloc(&client->dev, sizeof(*lt8912b), GFP_KERNEL);
    if (lt8912b == NULL)
        return -ENOMEM;
	
    lt8912b->i2c_stu = client;
	
    lt8912b->dev = &client->dev;

    ////////////////////////////////////////////////
    //addr_0
    
    lt8912b->chip_regmap = devm_regmap_init_i2c(client, &chip_regmap_config);
    if (IS_ERR(lt8912b->chip_regmap)) {
        dev_err(&client->dev, "Failed to initialize regmap\n");
        return PTR_ERR(lt8912b->chip_regmap);
     }

    ////////////////////////////////////////////////

    /////////////////////////////////////////////////
    //addr_1

    addr_1_node = of_get_child_by_name(np, "lt8912b_addr_1");
    if (!addr_1_node) {
        dev_err(&client->dev, "Failed to find lt8912b_addr_1 node");
        return -EINVAL;
    }

    lt8912b->addr_1 = devm_kzalloc(&client->dev, sizeof(struct addr_1), GFP_KERNEL);
    if (!lt8912b->addr_1)
        return -ENOMEM;
    
    if (of_property_read_u8(addr_1_node, "addr_1_reg", &addr_1_value)) {
        dev_err(&client->dev, "Failed to get addr_1_reg address");
        return -EINVAL;
    }
    addr_1_board_info.addr =addr_1_value;

    lt8912b->addr_1->i2c_stu = i2c_new_client_device(client->adapter, &addr_1_board_info);
    if (!lt8912b->addr_1->i2c_stu) {
        dev_err(&client->dev, "Failed to create I2C client for addr_1");
        return -ENODEV;
    }

    lt8912b->addr_1->regmap = devm_regmap_init_i2c(lt8912b->addr_1->i2c_stu, &chip_regmap_config);
    if (IS_ERR(lt8912b->addr_1->regmap)) {
        dev_err(&client->dev, "Failed to initialize addr_1 regmap");
         i2c_unregister_device(lt8912b->addr_1->i2c_stu);
        return PTR_ERR(lt8912b->addr_1->regmap);
    }
    
    ///////////////////////////////////////////////////

    /////////////////////////////////////////////////
    //addr_2

    addr_2_node = of_get_child_by_name(np, "lt8912b_addr_2");
    if (!addr_2_node) {
        dev_err(&client->dev, "Failed to find lt8912b_addr_2 node");
        return -EINVAL;
    }

    lt8912b->addr_2 = devm_kzalloc(&client->dev, sizeof(struct addr_2), GFP_KERNEL);
    if (!lt8912b->addr_2)
        return -ENOMEM;
    
    if (of_property_read_u8(addr_2_node, "addr_2_reg", &addr_2_value)) {
        dev_err(&client->dev, "Failed to get addr_2_reg address");
        return -EINVAL;
    }
    addr_2_board_info.addr =addr_2_value;

    lt8912b->addr_2->i2c_stu = i2c_new_client_device(client->adapter, &addr_2_board_info);
    if (!lt8912b->addr_2->i2c_stu) {
        dev_err(&client->dev, "Failed to create I2C client for addr_2");
        return -ENODEV;
    }

    lt8912b->addr_2->regmap = devm_regmap_init_i2c(lt8912b->addr_2->i2c_stu, &chip_regmap_config);
    if (IS_ERR(lt8912b->addr_2->regmap)) {
        dev_err(&client->dev, "Failed to initialize addr_2 regmap");
         i2c_unregister_device(lt8912b->addr_2->i2c_stu);
        return PTR_ERR(lt8912b->addr_2->regmap);
    }
    ///////////////////////////////////////////////////

    mutex_init(&lt8912b->ocm_lock);
	
    ret = chip_parse_dts(lt8912b->dev);				
    if (ret<0){
	dev_err(&client->dev, "Failed to parse device tree\n");
	return ret;
     }
	
    i2c_set_clientdata(client, lt8912b);
	
	
    kthread_obj = kthread_run(LT8912B_Main, NULL, "LT8912B_kthread");
    if (IS_ERR(kthread_obj)) {
        dev_err(&client->dev, "Failed to create kernel thread\n");
        return PTR_ERR(kthread_obj);
     }
	
	return 0;
	
}

static int chip_remove(struct i2c_client *client)
{  
   
    Main_Loop = false;
    msleep(500);

    mutex_destroy(&lt8912b->ocm_lock);

    i2c_unregister_device(lt8912b->addr_1->i2c_stu);

    i2c_unregister_device(lt8912b->addr_2->i2c_stu);

    chip_dev_exit();
	
    printk("driver removed\n");
    return 0;
}


static int chip_suspend(struct device *dev)
{

    printk(KERN_INFO "%s Suspend.\n",LOG_FLAG);

    gpiod_set_value(lt8912b->reset_gpio,0);
    msleep(10);

    Main_Loop =false;

    return 0;
}



static int chip_resume(struct device *dev)
{

    printk(KERN_INFO "%s Resume.\n",LOG_FLAG);


    gpiod_set_value(lt8912b->reset_gpio,1);
    msleep(5);
	
    Main_Loop = true;
    msleep(5);

    kthread_obj = kthread_run(LT8912B_Main, NULL, "LT8912B_kthread"); 
    
    return 0;
}


static const struct dev_pm_ops chip_pm_ops = {
    .suspend = chip_suspend,
    .resume =  chip_resume,
};




static const struct i2c_device_id chip_ids[] = {
	{"lt8912b", 0},
	{ }
};
MODULE_DEVICE_TABLE(i2c, chip_ids);


static const struct of_device_id chip_id_table[] = {
	{.compatible = "lontium,lt8912b"},
	{ }
};
MODULE_DEVICE_TABLE(of, chip_id_table);


static struct i2c_driver chip_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = "lt8912b",
		.pm = &chip_pm_ops,
		.of_match_table = chip_id_table,

	},
	.probe    = chip_probe,
	.remove   = chip_remove,
	.id_table = chip_ids,
};


module_i2c_driver(chip_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("LT8912B driver");
MODULE_AUTHOR("Tony Yang<syyang@lontium.com>");
