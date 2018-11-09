#ifndef _USER_AISKISS_H_
#define _USER_AISKISS_H_

#include "smartconfig.h"

uint8 ICACHE_FLASH_ATTR user_airkiss_start(void);
void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata);
extern u8 device_type[30];


#endif



