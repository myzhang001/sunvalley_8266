#ifndef __USER_GPIO_H__
#define __USER_GPIO_H__

extern os_timer_t rount_t;

void ICACHE_FLASH_ATTR rount_timer_func();
void ICACHE_FLASH_ATTR rount_timer_init(void);
void ICACHE_FLASH_ATTR rount_res_send(uint8 res);
//void ICACHE_FLASH_ATTR host_res_send(uint8 res);
void ICACHE_FLASH_ATTR WIFI_CNN_set(uint8 level);
void ICACHE_FLASH_ATTR user_gpio_init();
void ICACHE_FLASH_ATTR user_key_intr_handler();
void ICACHE_FLASH_ATTR gpio_inter_init();





#endif
