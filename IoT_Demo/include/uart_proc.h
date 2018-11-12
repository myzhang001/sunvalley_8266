#ifndef __UART_H__
#define __UART_H__


/*ruizhide*/
#define UPLOAD_CMD 0X01
#define DOWNLOAD_CMD 0X02
#define DISTRIBUTE 0XCD
#define MODULE_CONTROL 0XFE
#define ERROR_CMD 0XFF

//MODULE_CONTROL״̬��
#define CHECK_WIFI_STATUS 0X01//�豸��ѯ״̬
#define AUTO_NOTICE_WIFI_STATUS 0X02//ģ������֪ͨ״̬
#define MODULE_RESTART 0X03//ģ������
#define WIFI_CONFIG 0X04//����
#define CHECK_VERSION 0X05//��ѯwifi�̼��汾
#define GET_MAC 0X06  //�����ȡmac��ַ

//����
#define CONTROL_ERROR 0X01 
#define CHECKSUM_ERROR 0X02
#define COMMAND_NOT_SUPPORT 0X04



/**/
#if 1
#define CONFIG 0X01//����
#define CONFIG_RESP 0X02//��Ӧ����
#define REPORT_ONLINE 0X03//ģ������֪ͨ����״̬
#define REPORT_ONLINE_RESP 0X04
#define CHECK_ONLINE 0X05//��ѯ����״̬
#define CHECK_ONLINE_RESP 0X06 //��Ӧ��ѯ����
#define GET_SIGNAL_INTENSITY 0X07//��ȡ�ź�ǿ��
#define GET_SIGNAL_INTENSITY_RESP 0X08
#define UPLOAD_STATES 0X09  //�豸�����ϱ�״̬
#define UPLOAD_STATES_RESP 0X0A //
#define SERVER_CONTROL 0X0B//�����������豸
#define SERVER_CONTROL_REQ 0X0C
#define GET_MCU_TIME 0X0D  //��ȡ��Ƭ����ʱ��
#define GET_MCU_TIME_RESP 0X0E //��Ƭ����Ӧ��ǰʱ��
#define RESET_MODUL 0X0F//��Ƭ����������ģ��
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
