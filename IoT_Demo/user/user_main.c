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

//------------------------- 添加头文件
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




uint8_t G_online_mode;//0断网，1连上服务器
uint8_t dev_mac[6];//mac地址,hex
uint8_t device_id[13];//mac地址，字符串




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
os_timer_t user_timer;//用户程序
os_timer_t config_router_fail_t;
uint8 G_led_mode;

_TCP_MESS tcp_mess;

uint8 wifi_led_level;
uint8 G_router_mode=ROUTER_DISCONNECT;   //DISCONNECT、SMART_CONFIG、CONNECT_ROUTER、CONNECT_SERVER
uint8 G_last_router_mode=ROUTER_DISCONNECT;
uint8 G_server_mode=SERVER_DISCONNECT;   //SERVER_DISCONNECT，SERVER_CONFIG，SERVER_CONNECT
uint8 G_last_server_mode=SERVER_DISCONNECT;

uint32 G_https_flag;                     //0是http，1是https
uint8  G_connection_flag;                 //0是短连接，1是长连接
uint8  G_download_flag;                   //1开始下载文件 2正在下载文件
uint8  G_download_type;                   //1下载wifi程序，2下载单片机程序，3下载证书
uint8  tcp_host_data[1024];
uint8  http_host_data[1024];
uint8  tcp_send_flag;					 // 表示正在发送tcp数据包
os_timer_t queue_send_t;				 //
uint8 G_url_config_flag;				 // 1表示正在配置服务器




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



//设置路由器模式
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
	smart_config 函数设置
*/
void ICACHE_FLASH_ATTR smart_config(int32 time_out)
{
	DBG("smart config ver=%s\n",smartconfig_get_version());

	wifi_station_disconnect();//停止连接
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
    
    smartconfig_start(smartconfig_done);     //进入绑定
    
	rount_timer_init(time_out);              //设置超时绑定时间  

    DEBUG();
    system_soft_wdt_feed();

}


os_timer_t wifi_status_timer;

/*wifi连接状态处理函数*/
 void ICACHE_FLASH_ATTR work_status_cb(void)
{
	static uint8 last_router_mode;
	static uint8 last_server_mode;
	static uint8 i=0;
	
	if(last_router_mode != G_router_mode)      // 连接路由状态发生了改变
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
				if(G_last_router_mode==SMART_CONFIG);//配置路由器断网什么都不做
				else if(G_last_router_mode==ROUTER_CONNECT);//之前是连接状态
				{
					rount_res_send(0);//连接路由器失败
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
				 rount_res_send(1);//连接路由器成功
				if(G_last_router_mode==SMART_CONFIG)//配置路由器后，连接成功
				{
					os_timer_disarm(&config_router_fail_t);
				}
				if(tcp_mess.flag == 0xfdc0)//如果保存了tcp服务器信息
				{
					//tcp_clent_conn();//连接服务器
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
				if(tcp_mess.flag==0xfdc0)//如果保存了tcp服务器信息
				{
					//host_res_send(0);
				}
				if(G_router_mode==ROUTER_CONNECT)
				{
					//if(G_url_config_flag!=1)
					//tcp_clent_conn();//连接服务器
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
			case EVENT_STAMODE_GOT_IP:  //连接上路由器
				DBG("\n ##EVENT_STAMODE_GOT_IP! \n");
				set_G_router_mode(ROUTER_CONNECT);
			break;

			case EVENT_STAMODE_CONNECTED:
				DBG("\n ##EVENT_STAMODE_CONNECTED! \n");
			break;

 		    case EVENT_STAMODE_DISCONNECTED://路由器掉线
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

     wifi_set_opmode(STATION_MODE);         //设置为sta 模式

     //开始读取上一次的配置 连接路由器
     wifi_station_get_config(&inf);         //上一次保存的wifi 账号和密码

     inf.bssid_set = 0;

     if(os_strlen(inf.ssid))//保存有数据
     {

		os_printf("\r\n zmy_debug  ssid %s ", inf.ssid);
		os_printf("\r\n zmy_debug  password %s ", inf.password);

		wifi_station_connect();			 //重连成功后就不按新配置继续连接
		wifi_station_set_auto_connect(TRUE);//打开自动连接功能
     }
     else//出厂设置
     {
     	 #if 1
         os_sprintf(inf.ssid,FAC_TEST_SSID);
         os_sprintf(inf.password,FAC_TEST_PASSWORD);
         wifi_station_set_config(&inf);
 	     wifi_station_connect();			 //重连成功后就不按新配置继续连接
         wifi_station_set_auto_connect(TRUE);////打开自动连接功能
 	     #endif
     }
 }


 //用户程序调用
void user_cb()
{
	//DB("####################\n");
	os_printf("Project name:sunvalley \n");
	os_printf("SDK version:%s\n", system_get_sdk_version());
    os_printf("FW Compiled date:%s--%s\n",__DATE__,__TIME__);
	os_printf("version %d.%d,running in user%d\n",MAIN_VERSION,SECONDARY_VERSION,system_upgrade_userbin_check() +1);//升级固件的话记得更改版本号
	wifi_get_macaddr(STATION_IF,dev_mac);//获取stamac地址
	os_sprintf(device_id,"%X%X%X%X%X%X",dev_mac[0],dev_mac[1],dev_mac[2],dev_mac[3],dev_mac[4],dev_mac[5]);
	os_printf("mac_string:%s\n",device_id);


	//打印出文件启动的区域
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


	gpio_inter_init();      //按键处理程序
	//uart_proc_init();		//串口数据处理
	//tcp_client_init();	//设置tcp客户端参数

	#if 0
	wifi_get_macaddr(STATION_IF,device_mac);//获取stamac地址
	os_sprintf(device_id,"%02X",device_mac[0]);
	os_sprintf(device_id+2,"%02X",device_mac[1]);
	os_sprintf(device_id+4,"%02X",device_mac[2]);
	os_sprintf(device_id+6,"%02X",device_mac[3]);
	os_sprintf(device_id+8,"%02X",device_mac[4]);
	os_sprintf(device_id+10,"%02X",device_mac[5]);
	#endif

    #if 1
	//read_config();    //读取服务器信息

	Sta_init();		    //获取路由配置连接路由器

	//tcp_client_init();  //建立客户端和服务器通信

	wifi_set_event_handler_cb(wifi_handle_event_cb);

	#if 1
	os_timer_disarm(&wifi_status_timer);
	os_timer_setfn(&wifi_status_timer, (os_timer_func_t *)work_status_cb, NULL);
	os_timer_arm(&wifi_status_timer, 10, 1);
	#endif
	
	#endif

	//user_wifi_led_timer_init(200);//wifi指示灯
}



//扫描结束
void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status)
{
	/*
	status代表扫描结果；

	status为FAIL，代表扫描失败，比如，调用了wifi_station_connect就会出现这种情况
	status为OK，代表扫描热点成功，

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
				GPIO_OUTPUT_SET(GPIO_ID_PIN(WIFI_ROUTER_NUM), 0);//打开指示灯
				//test_counter = 0;
				//G_factory_test = 0xF;
				//os_timer_disarm(&factory_test_timer);
				//os_timer_setfn(&factory_test_timer, (os_timer_func_t *)product_test_relay,0);
				//os_timer_arm(&factory_test_timer, 500, 1);
				//wifi指示灯常亮
				//ra_gpio_output(PIN_WIFI_STATE_LED,WIFI_LED_ON);
				//GPIO_OUTPUT_SET(GPIO_ID_PIN(PIN_WIFI_STATE_LED), WIFI_LED_ON);
				//return;  //扫描到热点就直接跳出，不执行用户程序
				os_timer_disarm(&wifi_status_timer);
    				os_timer_setfn(&wifi_status_timer, (os_timer_func_t *)user_cb, NULL);
    				os_timer_arm(&wifi_status_timer, 1500, 0);
			}
		}
		else
		{
			os_printf("bss_link is  null.\n");
			GPIO_OUTPUT_SET(GPIO_ID_PIN(WIFI_ROUTER_NUM), 1);//关闭指示灯
			os_timer_disarm(&wifi_status_timer);			//没有产测热点，直接进用户程序
    			os_timer_setfn(&wifi_status_timer, (os_timer_func_t *)user_cb, NULL);
    			os_timer_arm(&wifi_status_timer, 10, 0);
			    /*用户代码*/
		}

	}
	else if(status ==FAIL)
	{
		os_printf("###scan_done fail!\n");
	}
}


//工厂测试模式
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
	if(ret != true)//返回失败说明不能进入扫描，比如设置成了SOFTAP_MODE，并且不会进入scan_done，
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
	UART_SetPrintPort(1);                                // 切换串口1为debug  对应node mcu D4 引脚

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

		
	   system_init_done_cb(product_test_start);               //初始化
	
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

       //WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);         //wifi连接标志位

       INFO("\r\nSystem started ...\r\n");
       
#endif
}

