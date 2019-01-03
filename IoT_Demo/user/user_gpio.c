
#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"
#include "common.h"
#include "gpio.h"
#include "smartconfig.h"
#include "eagle_soc.h"
#include "user_main.h"
struct espconn tcp_client;
void   rount_res_send(uint8 res);
os_timer_t rount_t;
uint8 URLCFG_flag=0;//1���������÷�����
uint8 ROUTCFG_flag=0;//����·������־λadd by zc
uint8 url_config;//��������url�ı�־λadd by zc
uint8 url_router_config;//����·�������߷������ı�־λ,Ϊ1ʱ����Ҫ�ظ�����������״̬��add by zc




extern _TCP_MESS tcp_mess;
extern uint8 tcp_connect_status;
extern os_timer_t wifi_conn_t;

extern uint8 smart_uar_flag;
//extern struct espconn tcp_client;
void ICACHE_FLASH_ATTR
wifi_conn_func();



void ICACHE_FLASH_ATTR
rount_timer_func()//����·����ʧ��
{
	DBG("\r\n ----zmy   smart smart_config  timeout ");
	
	smartconfig_stop();                    
	wifi_set_opmode(STATION_MODE);		   //����Ϊsta ģʽ

	wifi_station_connect();
	wifi_station_set_auto_connect(TRUE);////���Զ����ӹ���

    //rount_res_send(0);
    //ROUTCFG_flag=0;
}

void ICACHE_FLASH_ATTR
rount_timer_init(uint8 time_out)
{
    os_timer_disarm(&config_router_fail_t);
    os_timer_setfn(&config_router_fail_t,(os_timer_func_t *)rount_timer_func,NULL);
    os_timer_arm(&config_router_fail_t,time_out*1000,0);
}


void ICACHE_FLASH_ATTR
rount_res_send(uint8 res)//·�������ӽ��
{
    os_printf("\n rount_res_send :%d\n",res);
    struct station_config inf;
    wifi_set_opmode(STATION_MODE);
    wifi_station_get_config(&inf);//��ȡ���õ�·������Ϣ
    if(!os_strlen(inf.ssid))//û����·������Ϣ
        return;
    uint8 send_buf[100]= {0};
    uint8 i=0;
    uint8 len;
    send_buf[0]=0x13;
    send_buf[2]=res+0x30;//���ý��:
    send_buf[3]='+';
    i=os_strlen(inf.ssid);//��ȡssid����
    os_memcpy(&send_buf[4],inf.ssid,i);//�ӵ�4�ֽ�֮�󱣴�ssid
    len=3+i+1;
    send_buf[len]='+';
    i=os_strlen(inf.password);
    os_memcpy(&send_buf[len+1],inf.password,i);
    len=len+1+i;
    send_buf[1]=len-1;
    uart0_tx_buffer(send_buf,os_strlen(send_buf));
}







void ICACHE_FLASH_ATTR
WIFI_CNN_set(uint8 level)
{
    GPIO_OUTPUT_SET(GPIO_ID_PIN(WIFI_CNN_NUM),level);
    os_printf("\nWIFI_CNN_set :%d\n",level);

}
void ICACHE_FLASH_ATTR
user_gpio_init()
{
    PIN_FUNC_SELECT(WIFI_URLCFG_NAME,WIFI_URLCFG_FUNC);
    PIN_FUNC_SELECT(WIFI_ROUTER_NAME, WIFI_ROUTER_FUNC);
    PIN_FUNC_SELECT(WIFI_WKUP_NAME     , WIFI_WKUP_FUNC);
    PIN_FUNC_SELECT(WIFI_CNN_NAME     , WIFI_CNN_FUNC);
#if 0
    GPIO_OUTPUT_SET(GPIO_ID_PIN(WIFI_URLCFG_NUM),0);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(WIFI_ROUTER_NUM),0);
#else
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(WIFI_URLCFG_NUM));//����Ϊ����
    GPIO_DIS_OUTPUT(GPIO_ID_PIN(WIFI_ROUTER_NUM));//����Ϊ����
    PIN_PULLUP_DIS(WIFI_ROUTER_NAME);
    PIN_PULLUP_DIS(WIFI_URLCFG_NAME);
#endif   
    GPIO_OUTPUT_SET(GPIO_ID_PIN(WIFI_WKUP_NUM),1);//����WIFI_WKUP
    //WIFI_CNN_set(0);//����CNN

}
//uint8 WIFI_URLCFG_flag=0;

void ICACHE_FLASH_ATTR
user_key_intr_handler()
{
    //os_printf("\nkey_intr_handler\n");
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    //   ETS_GPIO_INTR_DISABLE();
    if (gpio_status & BIT(GPIO_ID_PIN(WIFI_ROUTER_NUM)))
    {
		DB("\n enter WIFI_ROUTER_NUM ir!\n");
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(GPIO_ID_PIN(WIFI_ROUTER_NUM)));
		set_G_router_mode(SMART_CONFIG);
    }
    else if (gpio_status & BIT(GPIO_ID_PIN(WIFI_URLCFG_NUM)))//���÷�����
    {
		DB("\n enter WIFI_URLCFG_NUM ir!\n");
		GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(GPIO_ID_PIN(WIFI_URLCFG_NUM))); 
		G_url_config_flag=1;
		//espconn_disconnect(&tcp_client);
		//set_G_server_mode(SERVER_CONFIG);
    }
    // ETS_GPIO_INTR_ENABLE();
}
void ICACHE_FLASH_ATTR
gpio_inter_init()  //��io���ж�
{
    ETS_GPIO_INTR_ATTACH(user_key_intr_handler, NULL);

    ETS_GPIO_INTR_DISABLE();		//�ر�gpio�ж�

    user_gpio_init();				//gpio��ʼ��

    gpio_pin_intr_state_set(GPIO_ID_PIN(WIFI_ROUTER_NUM),GPIO_PIN_INTR_POSEDGE);//�����������ж�
    gpio_pin_intr_state_set(GPIO_ID_PIN(WIFI_URLCFG_NUM),GPIO_PIN_INTR_POSEDGE);
    ETS_GPIO_INTR_ENABLE();			//ʹ��gpio�ж�
}



