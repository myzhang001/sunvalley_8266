#define OST_DEBUG //uncomment this when release

#ifdef OST_DEBUG
#define DBG(x...)	os_printf(x)
#define DEBUG()     os_printf("%s line=%d\n",__func__,__LINE__);
#define DB(x...)   os_printf("%s line=%d--%s\n",__func__,__LINE__,x)
#else
#define DBG(x...)
#define DEBUG()
#endif



#define TIMER_REGISTER(P_TIMER,FUNC,PARAM,TIME,CYCLE)	do \
														{ \
															os_timer_disarm(P_TIMER); \
															os_timer_setfn(P_TIMER, (os_timer_func_t *)FUNC, PARAM); \
														}while(0)

#define TIMER_START(P_TIMER,TIME,CYCLE)	do \
														{ \
															os_timer_arm(P_TIMER, TIME, CYCLE); \
														}while(0)

															
															
															typedef enum {
    UPLOAD_SUCCESS      = 0x00,
    UPLOAD_INVALID      = 0x01, //invalid   ID or pw
    UPLOAD_NO_CONNECT   = 0x02,
    UPLOAD_UNKNOW       = 0x03, //unknow error
    UPLOAD_NO_TASK      = 0x04  //no task for upload
}RESULT_T;

#define NET_STATUS_ERROR     0x00      //鏃犺繛鎺�
#define NET_STATUS_CONNECTED 0x01      //杩炴帴浜唚ifi
#define NET_STATUS_INTERNET  0x02      //鍙互杩炴帴浜掕仈缃�

#define SOTF_STA 0 // 1 涓篴p銆�0涓簊ta
//鎺ユ敹鏉ヨ嚜app鐨勯厤缃俊鎭�
typedef struct app_config
{
    uint8 ssid[64];
    uint8 passwd[64];
    uint8 id[64];
    uint8 key[64];

}app_config_st;

typedef struct 
{
	uint8 ip[4];
	int port;
	uint16 flag;
}UDP_STUAS;
#define ESP_PARAM_START_SEC  0x7c
#define ESP_PARAM_SAVE_0    1
#define ESP_PARAM_SAVE_1    2
#define ESP_PARAM_SAVE_2    3



#define WIFI_URLCFG_NUM     5
#define WIFI_URLCFG_NAME   PERIPHS_IO_MUX_GPIO5_U
#define WIFI_URLCFG_FUNC    FUNC_GPIO5   

#define WIFI_ROUTER_NUM    4
#define WIFI_ROUTER_NAME  PERIPHS_IO_MUX_GPIO4_U
#define WIFI_ROUTER_FUNC   FUNC_GPIO4

#define WIFI_WKUP_NUM     14
#define WIFI_WKUP_NAME   PERIPHS_IO_MUX_MTMS_U
#define WIFI_WKUP_FUNC	   FUNC_GPIO14  

#define WIFI_CNN_NUM	   12
#define WIFI_CNN_NAME 	   PERIPHS_IO_MUX_MTDI_U
#define WIFI_CNN_FUNC	   FUNC_GPIO12 

#define WIFI_OTA_NUM	   13
#define WIFI_OTA_NAME 	   PERIPHS_IO_MUX_MTCK_U
#define WIFI_OTA_FUNC	   FUNC_GPIO13

#define WIFI_HTTPS_NUM	   	15
#define WIFI_HTTPS_NAME 	PERIPHS_IO_MUX_MTDO_U
#define WIFI_HTTPS_FUNC	FUNC_GPIO15







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


typedef struct
{
	uint8 hostname[100];
	uint8 url[100];
	uint16 port;
	uint16 flag;
}_TCP_MESS;


typedef struct
{
	uint8 smart_flag;
	uint8 uar_flag;
}_FLAG_F;
