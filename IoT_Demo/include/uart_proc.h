#ifndef __UART_H__
#define __UART_H__


/*ruizhide*/
#define UPLOAD_CMD 0X01
#define DOWNLOAD_CMD 0X02
#define DISTRIBUTE 0XCD
#define MODULE_CONTROL 0XFE
#define ERROR_CMD 0XFF

//MODULE_CONTROL状态下
#define CHECK_WIFI_STATUS 0X01//设备查询状态
#define AUTO_NOTICE_WIFI_STATUS 0X02//模块主动通知状态
#define MODULE_RESTART 0X03//模块重启
#define WIFI_CONFIG 0X04//配网
#define CHECK_VERSION 0X05//查询wifi固件版本
#define GET_MAC 0X06  //请求获取mac地址

//错误
#define CONTROL_ERROR 0X01 
#define CHECKSUM_ERROR 0X02
#define COMMAND_NOT_SUPPORT 0X04



/**/
#if 1
#define CONFIG 0X01//配网
#define CONFIG_RESP 0X02//回应配网
#define REPORT_ONLINE 0X03//模块主动通知网络状态
#define REPORT_ONLINE_RESP 0X04
#define CHECK_ONLINE 0X05//查询网络状态
#define CHECK_ONLINE_RESP 0X06 //回应查询网络
#define GET_SIGNAL_INTENSITY 0X07//获取信号强度
#define GET_SIGNAL_INTENSITY_RESP 0X08
#define UPLOAD_STATES 0X09  //设备主动上报状态
#define UPLOAD_STATES_RESP 0X0A //
#define SERVER_CONTROL 0X0B//服务器控制设备
#define SERVER_CONTROL_REQ 0X0C
#define GET_MCU_TIME 0X0D  //获取单片机端时间
#define GET_MCU_TIME_RESP 0X0E //单片机回应当前时间
#define RESET_MODUL 0X0F//单片机请求重启模块
#define RESET_MODUL_RESP 0X10
#endif



void Smart_config_respond(uint8 result);
void Check_online_respond(uint8 result);
void Get_signal_intensiry_respond(uint8 result);
void Get_mcu_time(void);
u8 ICACHE_FLASH_ATTR Save_msg_id(char *msg_id,u32 *arg);
uint8 get_checksum(uint8_t *buf,uint8_t len);
uint8 get_checksum1(uint8_t *buf,u16 len);
void ICACHE_FLASH_ATTR Return_downmcu_result(uint8 result);
void ICACHE_FLASH_ATTR Return_cer_result(uint8 result);

/************SBSQ*************/
extern uint8 tcp_send_flag;
/************SBSQ*************/

#endif
