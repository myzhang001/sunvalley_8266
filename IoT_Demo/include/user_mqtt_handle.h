#ifndef __user_mqtt_h
#define __user_mqtt_h

#include "c_types.h"


typedef enum{
	E_FIXED_CONNECT = 1,
	E_FIXED_CONNACK = 2,
	E_FIXED_PUBLISH = 3,
	E_FIXED_PUBACK  = 4,
	E_FIXED_PUBREC  = 5,
	E_FIXED_PUREL   = 6,
	E_FIXED_PUBCOMP = 7,
	E_FIXED_SUBSCRIBE = 8,
	E_FIXED_SUBACK    = 9,
	E_FIXED_UNSUBSCRIBE = 10,
	E_FIXED_UNSUBACK    = 11,
	E_FIXED_PING        = 12,
	E_FIXED_PINGRSP     = 13,
	E_FIXED_DISCONNECT  = 14

}fixed_head_e;



//连接标志位
typedef struct{
	uint8_t user_flag;
	uint8_t password_flag;
	uint8_t willretain_flag;
	uint8_t willqos_flag;
	uint8_t willflag;
	uint8_t cleansession_flag;
	uint8_t reserve;
}connect_flag_s;


#define KEEP_ALIVE_TIME    (120)           //每个客户端可自定义设置连接保持时间，最短120秒，最长65535秒。 










#endif




