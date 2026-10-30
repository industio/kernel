#include "../include/include.h"


void LT8912B_Power_On(void)
{
    gpiod_set_value(lt8912b->power_gpio,1);
    msleep(5);
    printk("%s Set power High level.\n ",LOG_FLAG);

}

void LT8912B_Reset(void)
{
    //complete chip reset logic
    gpiod_set_value(lt8912b->reset_gpio,1);
    msleep(5);	
    gpiod_set_value(lt8912b->reset_gpio,0);
    msleep(5);
    gpiod_set_value(lt8912b->reset_gpio,1);
    msleep(100);
    printk("%s Reset chip.\n",LOG_FLAG);
}


int LT8912B_Main(void *data)
{  
    printk("%s Init Start!\n",LOG_FLAG);
    LT8912B_Power_On();
    LT8912B_Reset();
    LT8912B_Config();

    Tx_HPD = 0;

    return 0;	
}
