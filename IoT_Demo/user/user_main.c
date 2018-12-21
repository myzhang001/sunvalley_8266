/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"

#include "user_devicefind.h"
#include "user_webserver.h"

#include "driver/uart.h"
#include "driver/uart_register.h"

//------------------------- ���ͷ�ļ�
#include "mqtt/mqtt.h"
#include "modules/wifi.h"
#include "modules/config.h"
#include "mqtt/debug.h"
#include "gpio.h"
#include "user_interface.h"

#include "mem.h"
#include "sntp.h"
#include "c_types.h"
#include "common.h"
#include "smartconfig.h"
#include "user_airkiss.h"
#include "osapi.h"
#include "os_type.h"
#include "uart_proc.h"



#if 0
#define FAC_TEST_SSID     "Sunvalley-Office-E7"
#define FAC_TEST_PASSWORD "near#work123"

#else
#define FAC_TEST_SSID     "TP-LINK_2.4G"
#define FAC_TEST_PASSWORD "abcdef1234"
#endif




uint8_t G_online_mode;//0������1���Ϸ�����
uint8_t dev_mac[6];//mac��ַ,hex
uint8_t device_id[13];//mac��ַ���ַ���




#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif

#if ((SPI_FLASH_SIZE_MAP == 0) || (SPI_FLASH_SIZE_MAP == 1))
#error "The flash map is not supported"
#elif (SPI_FLASH_SIZE_MAP == 2)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0xfb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0xfc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0xfd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 3)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 4)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x81000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0x7c000
#elif (SPI_FLASH_SIZE_MAP == 5)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x1fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x1fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x1fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0xfc000
#elif (SPI_FLASH_SIZE_MAP == 6)
#define SYSTEM_PARTITION_OTA_SIZE							0x6A000
#define SYSTEM_PARTITION_OTA_2_ADDR							0x101000
#define SYSTEM_PARTITION_RF_CAL_ADDR						0x3fb000
#define SYSTEM_PARTITION_PHY_DATA_ADDR						0x3fc000
#define SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR				0x3fd000
#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR           0xfc000
#else
#error "The flash map is not supported"
#endif

#define SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM                SYSTEM_PARTITION_CUSTOMER_BEGIN

uint32 priv_param_start_sec;




MQTT_Client mqttClient;
typedef unsigned long u32_t;
static ETSTimer sntp_timer;


/*sbsq_toothbrush*/
os_timer_t wifi_status_timer;
os_timer_t led_timer;
os_timer_t user_timer;//�û�����
os_timer_t config_router_fail_t;
uint8 G_led_mode;

_TCP_MESS tcp_mess;

uint8 wifi_led_level;
uint8 G_router_mode=ROUTER_DISCONNECT;   //DISCONNECT��SMART_CONFIG��CONNECT_ROUTER��CONNECT_SERVER
uint8 G_last_router_mode=ROUTER_DISCONNECT;
uint8 G_server_mode=SERVER_DISCONNECT;   //SERVER_DISCONNECT��SERVER_CONFIG��SERVER_CONNECT
uint8 G_last_server_mode=SERVER_DISCONNECT;

uint32 G_https_flag;                     //0��http��1��https
uint8  G_connection_flag;                 //0�Ƕ����ӣ�1�ǳ�����
uint8  G_download_flag;                   //1��ʼ�����ļ� 2���������ļ�
uint8  G_download_type;                   //1����wifi����2���ص�Ƭ������3����֤��
uint8  tcp_host_data[1024];
uint8  http_host_data[1024];
uint8  tcp_send_flag;					 // ��ʾ���ڷ���tcp���ݰ�
os_timer_t queue_send_t;				 //
uint8 G_url_config_flag;				 // 1��ʾ�������÷�����




void sntpfn()
{
    u32_t ts = 0;
    ts = sntp_get_current_timestamp();
    os_printf("current time : %s\n", sntp_get_real_time(ts));
    if (ts == 0) {
        //os_printf("did not get a valid time from sntp server\n");
    } else {
            os_timer_disarm(&sntp_timer);
            MQTT_Connect(&mqttClient);
    }
}


void wifiConnectCb(uint8_t status)
{
    if(status == STATION_GOT_IP){
        sntp_setservername(0, "cn.pool.ntp.org");        // set sntp server after got ip address
        sntp_setservername(1, "tw.pool.ntp.org");        // set sntp server after got ip address
        sntp_init();
        os_timer_disarm(&sntp_timer);
        os_timer_setfn(&sntp_timer, (os_timer_func_t *)sntpfn, NULL);
        os_timer_arm(&sntp_timer, 1000, 1);//1s
    } else {
          MQTT_Disconnect(&mqttClient);
    }
}

void mqttConnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    INFO("MQTT: Connected\r\n");
    MQTT_Subscribe(client, "/mqtt/topic/0", 0);
    MQTT_Subscribe(client, "/mqtt/topic/1", 1);
    MQTT_Subscribe(client, "/mqtt/topic/2", 2);
    MQTT_Subscribe(client, "/alexa/input", 2);


    MQTT_Publish(client, "/mqtt/topic/0", "hello0", 6, 0, 0);
    MQTT_Publish(client, "/mqtt/topic/1", "hello1", 6, 1, 0);
    MQTT_Publish(client, "/mqtt/topic/2", "hello2", 6, 2, 0);
    MQTT_Publish(client, "/alexa/input", "hello2", 6, 2, 0);



}

void mqttDisconnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    os_printf("MQTT: Disconnected\r\n");
}

void mqttPublishedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    os_printf("MQTT: Published\r\n");
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
    char *topicBuf = (char*)os_zalloc(topic_len+1),
            *dataBuf = (char*)os_zalloc(data_len+1);

    MQTT_Client* client = (MQTT_Client*)args;

    os_memcpy(topicBuf, topic, topic_len);
    topicBuf[topic_len] = 0;

    os_memcpy(dataBuf, data, data_len);
    dataBuf[data_len] = 0;

    os_printf("Receive topic: %s, data: %s \r\n", topicBuf, dataBuf);
    os_free(topicBuf);
    os_free(dataBuf);
}



static const partition_item_t at_partition_table[] = {
    { SYSTEM_PARTITION_BOOTLOADER, 						0x0, 												0x1000},
    { SYSTEM_PARTITION_OTA_1,   						0x1000, 											SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_OTA_2,   						SYSTEM_PARTITION_OTA_2_ADDR, 						SYSTEM_PARTITION_OTA_SIZE},
    { SYSTEM_PARTITION_RF_CAL,  						SYSTEM_PARTITION_RF_CAL_ADDR, 						0x1000},
    { SYSTEM_PARTITION_PHY_DATA, 						SYSTEM_PARTITION_PHY_DATA_ADDR, 					0x1000},
    { SYSTEM_PARTITION_SYSTEM_PARAMETER, 				SYSTEM_PARTITION_SYSTEM_PARAMETER_ADDR, 			0x3000},
    { SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM,             SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM_ADDR,          0x1000},
};

void ICACHE_FLASH_ATTR user_pre_init(void)
{
    if(!system_partition_table_regist(at_partition_table, sizeof(at_partition_table)/sizeof(at_partition_table[0]),SPI_FLASH_SIZE_MAP)) {
		os_printf("system_partition_table_regist fail\r\n");
		while(1);
	}
}



//����·����ģʽ
void ICACHE_FLASH_ATTR set_G_router_mode(uint8 mode)
{
	G_last_router_mode = G_router_mode;
	G_router_mode 	   = mode;
}


void ICACHE_FLASH_ATTR set_G_server_mode(uint8 mode)
{
	G_last_server_mode=G_server_mode;
	G_server_mode=mode;
}

/*
	smart_config ��������
*/
void ICACHE_FLASH_ATTR smart_config(int32 time_out)
{
	DBG("smart config ver=%s\n",smartconfig_get_version());

	wifi_station_disconnect();//ֹͣ����
    wifi_station_set_auto_connect(FALSE);
	DEBUG();


    system_soft_wdt_feed();
    smartconfig_stop();
    system_soft_wdt_feed();

    smartconfig_set_type(SC_TYPE_ESPTOUCH);

    if(wifi_get_opmode() != STATION_MODE)//sta mode
    {
        wifi_set_opmode(STATION_MODE);
    }

    esptouch_set_timeout(time_out);

    system_soft_wdt_feed();
    
    smartconfig_start(smartconfig_done);     //�����
    
	rount_timer_init(time_out);              //���ó�ʱ��ʱ��  

    DEBUG();
    system_soft_wdt_feed();

}


os_timer_t wifi_status_timer;

/*wifi����״̬������*/
 void ICACHE_FLASH_ATTR work_status_cb(void)
{
	static uint8 last_router_mode;
	static uint8 last_server_mode;
	static uint8 i=0;
	
	if(last_router_mode != G_router_mode)      // ����·��״̬�����˸ı�
	{
		last_router_mode= G_router_mode;
		DBG("###G_router_mode change to %d from %d!\n",G_router_mode,G_last_router_mode);
		//Report_status(G_wifi_status);
		switch(G_router_mode)
			{
			case ROUTER_DISCONNECT:
				DBG("###ROUTER_DISCONNECT!\n");
				WIFI_CNN_set(0);
				//
				set_G_server_mode(SERVER_DISCONNECT);
				if(G_last_router_mode==SMART_CONFIG);//����·��������ʲô������
				else if(G_last_router_mode==ROUTER_CONNECT);//֮ǰ������״̬
				{
					rount_res_send(0);//����·����ʧ��
				}
				break;
			case SMART_CONFIG:
				DBG("###SMART_CONFIG!\n");

				smart_config(120);

				#if 0
				smartconfig_stop();
				smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS);
				smartconfig_start(smartconfig_done);
				#endif
				break;
			case ROUTER_CONNECT:
				DBG("###ROUTER_CONNECT!\n");
				 rount_res_send(1);//����·�����ɹ�
				if(G_last_router_mode==SMART_CONFIG)//����·���������ӳɹ�
				{
					os_timer_disarm(&config_router_fail_t);
				}
				if(tcp_mess.flag == 0xfdc0)//���������tcp��������Ϣ
				{
					//tcp_clent_conn();//���ӷ�����
				}
				break;
			default:break;
			}
		last_router_mode != G_router_mode;
	}
	if(last_server_mode!=G_server_mode)
	{

		last_server_mode=G_server_mode;
		DBG("###G_server_mode change to %d from:%d!\n",G_server_mode,G_last_server_mode);
		//Report_status(G_wifi_status);
		switch(last_server_mode)
			{
			case SERVER_DISCONNECT:
				WIFI_CNN_set(0);
				if(tcp_mess.flag==0xfdc0)//���������tcp��������Ϣ
				{
					//host_res_send(0);
				}
				if(G_router_mode==ROUTER_CONNECT)
				{
					//if(G_url_config_flag!=1)
					//tcp_clent_conn();//���ӷ�����
				}

				DBG("###-------  zmy  SERVER_DISCONNECT!\n");

				break;
			case SERVER_CONNECT:
				WIFI_CNN_set(1);
				//host_res_send(1);

				DBG("###-------  zmy  SERVER_CONNECT!\n");

				break;
			default:break;
			}
	}

}

 void ICACHE_FLASH_ATTR wifi_handle_event_cb(System_Event_t *evt)
 {
     uint8 i;

 	static uint32 event=0xff;
 	if(event!=evt->event)
 	{
 		os_printf("\n wifi_handle_event_cb event:%d\n",evt->event);
 		event=evt->event;
 	    switch(evt->event)
 	    {
			case EVENT_STAMODE_GOT_IP:  //������·����
				DBG("\n ##EVENT_STAMODE_GOT_IP! \n");
				set_G_router_mode(ROUTER_CONNECT);
			break;

			case EVENT_STAMODE_CONNECTED:
				DBG("\n ##EVENT_STAMODE_CONNECTED! \n");
			break;

 		    case EVENT_STAMODE_DISCONNECTED://·��������
				DBG("\n ##EVENT_STAMODE_DISCONNECTED! \n");
				set_G_router_mode(ROUTER_DISCONNECT);
				/*
				DBG("EVENT_STAMODE_DISCONNECTED!\n");
				//break;
				default :
				G_wifi_status=WIFI_DISCONNECT;
				if(G_led_mode!=SMART_CONFIG)
				G_led_mode=DISCONNECT;
				*/
 		        break;
 	    }
 	}
 }

 void ICACHE_FLASH_ATTR Sta_init(void)
 {
     struct station_config inf;

     wifi_set_opmode(STATION_MODE);         //����Ϊsta ģʽ

     //��ʼ��ȡ��һ�ε����� ����·����
     wifi_station_get_config(&inf);         //��һ�α����wifi �˺ź�����

     inf.bssid_set = 0;

     if(os_strlen(inf.ssid))//����������
     {

		os_printf("\r\n zmy_debug  ssid %s ", inf.ssid);
		os_printf("\r\n zmy_debug  password %s ", inf.password);

		wifi_station_connect();			 //�����ɹ���Ͳ��������ü�������
		wifi_station_set_auto_connect(TRUE);//���Զ����ӹ���
     }
     else//��������
     {
     	 #if 1
         os_sprintf(inf.ssid,FAC_TEST_SSID);
         os_sprintf(inf.password,FAC_TEST_PASSWORD);
         wifi_station_set_config(&inf);
 	     wifi_station_connect();			 //�����ɹ���Ͳ��������ü�������
         wifi_station_set_auto_connect(TRUE);////���Զ����ӹ���
 	     #endif
     }
 }


 //�û��������
void user_cb()
{
	//DB("####################\n");
	os_printf("Project name:sunvalley \n");
	os_printf("SDK version:%s\n", system_get_sdk_version());
    os_printf("FW Compiled date:%s--%s\n",__DATE__,__TIME__);
	os_printf("version %d.%d,running in user%d\n",MAIN_VERSION,SECONDARY_VERSION,system_upgrade_userbin_check() +1);//�����̼��Ļ��ǵø��İ汾��
	wifi_get_macaddr(STATION_IF,dev_mac);//��ȡstamac��ַ
	os_sprintf(device_id,"%X%X%X%X%X%X",dev_mac[0],dev_mac[1],dev_mac[2],dev_mac[3],dev_mac[4],dev_mac[5]);
	os_printf("mac_string:%s\n",device_id);


	//��ӡ���ļ�����������
	if(system_upgrade_userbin_check() == UPGRADE_FW_BIN1)
	{
		os_printf("---sys run from bin1---\n");
	}
	else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2)
	{
		os_printf("---sys run from bin2---\n");
	}


	system_set_os_print(1);//disable when release


	struct rst_info *rst_info = system_get_rst_info();

	DBG("reset reason: %x\n", rst_info->reason);

	if(rst_info->reason == REASON_WDT_RST || rst_info->reason == REASON_EXCEPTION_RST || rst_info->reason == REASON_SOFT_WDT_RST)
	{
		if (rst_info->reason == REASON_EXCEPTION_RST)
		{
			DBG("Fatal exception (%d):\n", rst_info->exccause);
		}
		DBG("epc1=0x%08x, epc2=0x%08x, epc3=0x%08x, excvaddr=0x%08x, depc=0x%08x\n",
			rst_info->epc1, rst_info->epc2, rst_info->epc3, rst_info->excvaddr, rst_info->depc);
	}


	gpio_inter_init();      //�����������
	//uart_proc_init();		//�������ݴ���
	//tcp_client_init();	//����tcp�ͻ��˲���

	#if 0
	wifi_get_macaddr(STATION_IF,device_mac);//��ȡstamac��ַ
	os_sprintf(device_id,"%02X",device_mac[0]);
	os_sprintf(device_id+2,"%02X",device_mac[1]);
	os_sprintf(device_id+4,"%02X",device_mac[2]);
	os_sprintf(device_id+6,"%02X",device_mac[3]);
	os_sprintf(device_id+8,"%02X",device_mac[4]);
	os_sprintf(device_id+10,"%02X",device_mac[5]);
	#endif

    #if 1
	//read_config();    //��ȡ��������Ϣ

	Sta_init();		    //��ȡ·����������·����

	//tcp_client_init();  //�����ͻ��˺ͷ�����ͨ��

	wifi_set_event_handler_cb(wifi_handle_event_cb);

	#if 1
	os_timer_disarm(&wifi_status_timer);
	os_timer_setfn(&wifi_status_timer, (os_timer_func_t *)work_status_cb, NULL);
	os_timer_arm(&wifi_status_timer, 10, 1);
	#endif
	
	#endif

	//user_wifi_led_timer_init(200);//wifiָʾ��
}



//ɨ�����
void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status)
{
	/*
	status����ɨ������

	statusΪFAIL������ɨ��ʧ�ܣ����磬������wifi_station_connect�ͻ�����������
	statusΪOK������ɨ���ȵ�ɹ���

	*/
	PIN_FUNC_SELECT(WIFI_ROUTER_NAME, WIFI_ROUTER_FUNC);
	
	os_printf("###scan_done :%d!\n",status);
	if(status == OK)
	{
		os_printf("###scan_done OK\n");
		struct bss_info *bss_link = (struct bss_info *)arg;
		if(bss_link)
		{
			os_printf("bss_link is not null.\n");
			if(os_memcmp(bss_link->ssid,FAC_TEST_SSID,os_strlen(bss_link->ssid))==0)
			{
				os_printf(" find AP factory_test success...\n");
				os_printf("----------- Factory OK -----------\n");
				GPIO_OUTPUT_SET(GPIO_ID_PIN(WIFI_ROUTER_NUM), 0);//��ָʾ��
				//test_counter = 0;
				//G_factory_test = 0xF;
				//os_timer_disarm(&factory_test_timer);
				//os_timer_setfn(&factory_test_timer, (os_timer_func_t *)product_test_relay,0);
				//os_timer_arm(&factory_test_timer, 500, 1);
				//wifiָʾ�Ƴ���
				//ra_gpio_output(PIN_WIFI_STATE_LED,WIFI_LED_ON);
				//GPIO_OUTPUT_SET(GPIO_ID_PIN(PIN_WIFI_STATE_LED), WIFI_LED_ON);
				//return;  //ɨ�赽�ȵ��ֱ����������ִ���û�����
				os_timer_disarm(&wifi_status_timer);
    				os_timer_setfn(&wifi_status_timer, (os_timer_func_t *)user_cb, NULL);
    				os_timer_arm(&wifi_status_timer, 1500, 0);
			}
		}
		else
		{
			os_printf("bss_link is  null.\n");
			GPIO_OUTPUT_SET(GPIO_ID_PIN(WIFI_ROUTER_NUM), 1);//�ر�ָʾ��
			os_timer_disarm(&wifi_status_timer);			//û�в����ȵ㣬ֱ�ӽ��û�����
    			os_timer_setfn(&wifi_status_timer, (os_timer_func_t *)user_cb, NULL);
    			os_timer_arm(&wifi_status_timer, 10, 0);
			    /*�û�����*/
		}

	}
	else if(status ==FAIL)
	{
		os_printf("###scan_done fail!\n");
	}
}


//��������ģʽ
void ICACHE_FLASH_ATTR product_test_start(void)
{

	os_printf("#### sunvelly_prodct_test_start  wifi init done");

	bool ret = wifi_set_opmode(STATION_MODE);

	if(ret != true)
	{
		os_printf("set station mode failed.\n");
	}

	wifi_station_disconnect();   //scan test AP


	struct scan_config config;

	os_memset(&config,'\0',sizeof(struct scan_config));
	config.ssid = FAC_TEST_SSID;

	os_printf("product_test_start scan  factory_test AP\n");
	ret = wifi_station_scan(&config,scan_done);
	if(ret != true)//����ʧ��˵�����ܽ���ɨ�裬�������ó���SOFTAP_MODE�����Ҳ������scan_done��
	{
		os_printf("######wifi_station_scan error.\n");
	}
}





/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
	uart_init(BIT_RATE_115200,BIT_RATE_115200);
	UART_SetPrintPort(1);                                // �л�����1Ϊdebug  ��Ӧnode mcu D4 ����

	uart0_sendStr("\r\n-------------zmy hello world ---------");

    partition_item_t partition_item;
    os_printf("SDK version:%s\n", system_get_sdk_version());

    if (!system_partition_get_item(SYSTEM_PARTITION_CUSTOMER_PRIV_PARAM, &partition_item)) {
        os_printf("Get partition information fail\n");
    }
    priv_param_start_sec = partition_item.addr/SPI_FLASH_SEC_SIZE;
#if ESP_PLATFORM
    /*Initialization of the peripheral drivers*/
    /*For light demo , it is user_light_init();*/
    /* Also check whether assigned ip addr by the router.If so, connect to ESP-server  */
    //user_esp_platform_init();
#endif
    /*Establish a udp socket to receive local device detect info.*/
    /*Listen to the port 1025, as well as udp broadcast.
    /*If receive a string of device_find_request, it rely its IP address and MAC.*/
    //user_devicefind_init();

    /*Establish a TCP server for http(with JSON) POST or GET command to communicate with the device.*/
    /*You can find the command in "2B-SDK-Espressif IoT Demo.pdf" to see the details.*/
    /*the JSON command for curl is like:*/
    /*3 Channel mode: curl -X POST -H "Content-Type:application/json" -d "{\"period\":1000,\"rgb\":{\"red\":16000,\"green\":16000,\"blue\":16000}}" http://192.168.4.1/config?command=light      */
    /*5 Channel mode: curl -X POST -H "Content-Type:application/json" -d "{\"period\":1000,\"rgb\":{\"red\":16000,\"green\":16000,\"blue\":16000,\"cwhite\":3000,\"wwhite\",3000}}" http://192.168.4.1/config?command=light      */
#ifdef SERVER_SSL_ENABLE
    user_webserver_init(SERVER_SSL_PORT);
#else
    //user_webserver_init(SERVER_PORT);
#endif

		
	   system_init_done_cb(product_test_start);               //��ʼ��
	
#if 1
       CFG_Load();

       MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
       //MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

       MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
       //MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);

       MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
       MQTT_OnConnected(&mqttClient, mqttConnectedCb);
       MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
       MQTT_OnPublished(&mqttClient, mqttPublishedCb);
       MQTT_OnData(&mqttClient, mqttDataCb);

       //WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);         //wifi���ӱ�־λ

       INFO("\r\nSystem started ...\r\n");
       
#endif
}

