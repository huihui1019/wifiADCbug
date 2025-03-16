#include "gpio.h"
#define KEY_STATUS(x) !HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9>>x)
#define KEY_LOOP(x) for(int k=0;k<4;++k){       \
    if(KEY_STATUS(k)&&x[k]<4294967296){++x[k];} \
    else if (!KEY_STATUS(k)){x[k]=0;}            \
}
