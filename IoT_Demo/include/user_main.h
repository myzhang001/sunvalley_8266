#ifndef _USER_MAIN_H
#define _USER_MAIN_H



#define DOWN_CER_ENABLE //定义这个宏，才能下载证书
#define DOWN_MCU_FIRMWARE_ENABLE ////定义这个宏，才能通过串口发送MCU固件

//G_router_mode
#define ROUTER_DISCONNECT 1
#define SMART_CONFIG 2//正在配置路由器
#define ROUTER_CONNECT 3//连接上路由器

#define SERVER_DISCONNECT 1
//#define SERVER_CONFIG 2//正在配置服务器
#define SERVER_CONNECT 3//连接上服务器
#define CER_ADDRESS (ESP_PARAM_START_SEC+ESP_PARAM_SAVE_2)

#define WIFI_STATE_IO_NUM        15//wifi led
#define WIFI_STATE_IO_NAME       PERIPHS_IO_MUX_MTDO_U
#define WIFI_STATE_IO_FUNC       FUNC_GPIO15

#define HTTP 0
#define HTTPS 1

#define CLOSE 0
#define kEEPALIVE 1

#define MAIN_VERSION 1
#define SECONDARY_VERSION 0

#define OTA_URL "http://tools.xy-jit.cc"
#define FILE_URL "/brush_stream"

#define HTTP_FORMAT "POST %s HTTP/1.1\r\n\
Host:%s\r\nContent-Type:application/msgpack\r\n\
Content-Length:%d\r\nConnection: keep-alive\r\nCache-Control:no-cache\r\nUser-Agent: Esp8266/%d.%d\r\n\r\n"


extern uint8 wifi_led_level;
extern uint8 G_router_mode;
extern uint8 G_last_router_mode;
extern uint8 G_server_mode;//
extern uint8 G_last_server_mode;
extern os_timer_t config_router_fail_t;
extern _TCP_MESS tcp_mess;
extern uint8 G_download_flag;
extern uint8 G_download_type;
extern uint8 tcp_host_data[1024];
extern uint8 tcp_send_flag;
extern uint8 http_host_data[1024];
extern os_timer_t queue_send_t;
extern uint32 G_https_flag;
extern uint8 G_url_config_flag;
extern uint8_t dev_mac[6];
extern uint8_t  device_id[13];

void ICACHE_FLASH_ATTR set_G_router_mode(uint8 mode);
void ICACHE_FLASH_ATTR set_G_server_mode(uint8 mode);
void ICACHE_FLASH_ATTR read_config();



#endif
