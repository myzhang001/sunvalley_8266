/*
    处理模块与mcu直接的串口数据
*/
#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"
#include "common.h"
#include "upgrade.h"
#include "c_types.h"
#include "gpio.h"
#include "user_airkiss.h"
#include "uart_proc.h"
#include "config.h"
#include "mqtt_config.h"
#include "mqtt.h"
#include "debug.h"
#include "uart_proc.h"
#include "user_main.h"




#define UART_DATA_BUF_LEN			1024	//byte，因为牙刷项目一包比较长
#define UART_DATA_PACKAGE_TIME_OUT	10//ms
#define WIFI_recvTaskQueueLen       100


/*ruidezhi*/
uint8 G_fram_serial_number;//帧序号
extern uint8 G_led_mode;
/*********/



uint8 get_checksum(uint8_t *buf,uint8_t len);
void ICACHE_FLASH_ATTR suscibe(void);

LOCAL  os_event_t    WIFI_recvTaskQueue[WIFI_recvTaskQueueLen];

//extern DEV_INFO_ST G_dev_info;
extern uint8 wifi_status;
extern struct espconn tcp_client;
extern uint8_t G_online_mode;
extern char mcu_time[11];
extern u8 G_mqtt_connected;
extern SYSCFG sysCfg;
extern MQTT_Client mqttClient;

typedef struct
{
    uint8_t buf[UART_DATA_BUF_LEN];
    size_t size;
} uart_data_t;

uart_data_t *uart_data = NULL;




void ICACHE_FLASH_ATTR uart_data_send(uint8 *buf, uint16 len)
{
    DBG("uart_data_send:\n");
#if 0
    debug_print_hex(buf,len);
    uart0_tx_buffer(buf,len);
	#endif
}
#if 0
uint8 ICACHE_FLASH_ATTR gen_sn()
{
    static uint8 sn = 0;

    if(sn < 0xFF)
    {
        sn++;
    }
    else
    {
        sn = 0;
    }

    return sn;
}

//wifi获取硬件状态
uint8 ICACHE_FLASH_ATTR wifi_send_cmd(Tran_type type,uint8 sn,uint8 cmd,uint8 *payload,uint8 payload_len)
{
    uint8 buff[64]= {0x0};
    uint8 SN;
#if 0    
    buff[0] = 0xC5;//header

    buff[1] = (payload_len+6)&0xFF;//len L
    buff[2] = ((payload_len+6)&0xFF00)>> 8;//len H

    buff[3] = cmd;//cmd

    if(type == User_type)
    {
        // sn
        buff[4] = gen_sn(); //
        SN = buff[4];
        
        creat_node(SN,payload,payload_len,cmd);

        return;//通过200ms定时器去发
    }
    else if(type == System_type)
    {
        buff[4] = sn;
        SN = sn;
    }
    else
    {
        os_printf("not konw trans type in wifi_send_cmd\n");
        return;
    }

    buff[5] = checksum(buff,5);//header checksum=(header+lenL+lenH+cmd+sn)

    if(payload_len == 0)
    {
        buff[6] = 0;//payload
    }
    else
    {
        os_memcpy(&buff[6],payload,payload_len);
        buff[(payload_len+6)] = checksum(payload,payload_len);//check sum
    }

    uart_data_send(buff,payload_len+7);
  #endif 
    return  SN;
}

//WIFI_REQ_DEV_INFO

void ICACHE_FLASH_ATTR wifi_req_dev_info()
{
    wifi_send_cmd(User_type,0,WIFI_REQ_DEV_INFO,NULL,0);
}

//WIFI_REQ_DEV_STATE
void ICACHE_FLASH_ATTR wifi_req_dev_state()
{
    wifi_send_cmd(User_type,0,WIFI_REQ_DEV_STATE,NULL,0);
}

//WIFI_SET_DEV_STATE

void ICACHE_FLASH_ATTR wifi_set_dev_state(uint8 cmd_type,uint8 value)
{
	uint8 payload[2];

	payload[0] = cmd_type;
	payload[1] = value;
	
	switch(cmd_type)
	{
		case DSGJ:
		{
			if(value != 0 || value != 2 || value != 4 ||value != 8 || value != 12 || value != 18 || value != 24)
			{
				DBG(" data error 1\n");
			}
			break;
		}
		case ZYJS:
		{
			if(value < 1 || value > 5)
			{
				DBG(" data error 2\n");
			}
			break;
		}
		case JSQGJ:
		{
			break;
		}
		case JSQKJ:
		{
			break;
		}
		case HS:
		{
			if(value != 35 || value != 40 || value != 45 ||value != 50 || value != 55 || value != 60)
			{
				DBG(" data error 3\n");
			}
			break;
		}
		case NWKQ:
		{
			break;
		}
		case NWGB:
		{
			break;
		}
		case YDKQ:
		{
			if(value < 1 || value > 4)
			{
				DBG(" data error 4\n");
			}
			break;
		}
		case YDGB:
		{
			break;
		}
		default:
		{
			break;
		}
	}

	if(cmd_type == DSGJ || cmd_type == ZYJS || cmd_type == HS || cmd_type == YDKQ)	
	    wifi_send_cmd(User_type,0,WIFI_SET_DEV_STATE,payload,2);
    else
        wifi_send_cmd(User_type,0,WIFI_SET_DEV_STATE,payload,1);
}

//WIFI_SEND_BEAT
void ICACHE_FLASH_ATTR wifi_send_beat()
{
    wifi_send_cmd(User_type,0,WIFI_SEND_BEAT,NULL,0);
}

//WIFI_INFORM_STATE
/*
	0: AP 模式；（快闪）
	1：未连接路由器（慢闪）
	2：连接上路由器（灭）
	3：连接上服务器 （常亮）
*/
void ICACHE_FLASH_ATTR wifi_inform_state(uint8 state)
{
    wifi_send_cmd(User_type,0,WIFI_INFORM_STATE,&state,1);
}

//WIFI_QUERY_DEV_MODEL
void ICACHE_FLASH_ATTR wifi_query_dev_model()
{
    wifi_send_cmd(User_type,0,WIFI_QUERY_DEV_MODEL,NULL,0);
}

//MCU_UPLOAD_DEV_STATE_ACK
void ICACHE_FLASH_ATTR mcu_upload_dev_state_ack(uint8 sn)
{
    wifi_send_cmd(System_type,sn,MCU_UPLOAD_DEV_STATE_ACK,NULL,0);
}

//MCU_REQ_RESET_WIFI_ACK
void ICACHE_FLASH_ATTR mcu_req_reset_wifi_ack(uint8 sn)
{
    wifi_send_cmd(System_type,sn,MCU_REQ_RESET_WIFI_ACK,NULL,0);
}

//MCU_SET_WIFI_MODE_ACK
void ICACHE_FLASH_ATTR mcu_set_wifi_mode_ack(uint8 sn)
{
    wifi_send_cmd(System_type,sn,MCU_SET_WIFI_MODE_ACK,NULL,0);
}
#endif




/*msg_id:保存的信息，arg:返回地址值作为和单片机通信的id*/
u8 ICACHE_FLASH_ATTR Save_msg_id(char *msg_id,u32 *arg)
{
	//os_printf("\n--system_get_free_heap_size:%d---\n",system_get_free_heap_size());
	char *addr=(char *)os_malloc(os_strlen(msg_id)+2);
	INFO("os_strlen(msg_id)+2:%d\n",os_strlen(msg_id)+2);
	if(!addr)
	{
		INFO("##MSG_ID malloc fail!\n");
		return 0;
	}
	*addr=0xC5;//标志位
	u8 len=os_strlen(msg_id);
	os_memcpy(addr+1,msg_id,len);
	*(addr+1+len)='\0';
	*arg=(u32)addr;
	INFO("SAVE :%s\n",msg_id);
	INFO("ADDR :%d addr :%d\n",addr,*arg);
	
	return 1;
}

/*arg是单片机返回的id，msg_id_add用来保存获取的id*/
 u8 ICACHE_FLASH_ATTR Get_msg_id(u32 arg,char *msg_id_add)
{
	char *addr=(char *)arg;
	if(addr[0]!=0XC5)
	{
		INFO("##get msg_id error!\n");
		return 0;
	}
	u8 len=os_strlen(addr+1);
	os_memcpy(msg_id_add,addr+1,len);
	*(msg_id_add+len)='\0';
	os_free(addr);
	INFO("GET :%s\n",msg_id_add);
	return 1;
	
}

/*瑞德智*/
//返回配置结果



/*-------*/




/*************SBSQ*****************/





void ICACHE_FLASH_ATTR
host_res_send(uint8 res)//连接服务器的状态
{
    DBG("host_res_send :%d\n",res);
    uint8 send_buf[100]= {0};
    os_memset(send_buf,0,100);
    send_buf[0]=0x02;
    send_buf[1]=os_strlen(tcp_mess.hostname);
    send_buf[2]=res+0x30;
    send_buf[3]='+';
    os_memcpy(&send_buf[4],tcp_mess.hostname,os_strlen(tcp_mess.hostname));
    uart0_tx_buffer(send_buf,os_strlen(send_buf));
}



void ICACHE_FLASH_ATTR
host_conf_send(uint8 res)
{
    os_printf("host name:%s\n",tcp_mess.hostname);
    uint8 send_buf[100];
    os_memset(send_buf,0,100);
    send_buf[0]=0x01;
    send_buf[1]=os_strlen(tcp_mess.hostname);
    send_buf[2]=res+0x30;//获取'0'或者'1'
    send_buf[3]='+';
    os_memcpy(&send_buf[4],tcp_mess.hostname,os_strlen(tcp_mess.hostname));
    uart0_tx_buffer(send_buf,os_strlen(send_buf));
}









os_timer_t tcp_fail_t;//上报数据失败的定时器

void ICACHE_FLASH_ATTR Return_mac(void)
{ 	
	uint8 respond[12];
	os_memset(respond,0,12);
	respond[0]=0x36;
	respond[1]=0x08;
	respond[2]=dev_mac[5];
	respond[3]=dev_mac[4];
	respond[4]=dev_mac[3];
	respond[5]=dev_mac[2];
	respond[6]=dev_mac[1];
	respond[7]=dev_mac[0];
	uart0_tx_buffer(respond,8);

	
}


void ICACHE_FLASH_ATTR Return_Firmware_message(void)
{
	uint8 respond[5];
	os_memset(respond,0,5);
	respond[0]=0x37;
	respond[1]=0x05;
	respond[2]=system_upgrade_userbin_check() +1;
	respond[3]=MAIN_VERSION;
	respond[4]=SECONDARY_VERSION;
	uart0_tx_buffer(respond,5);	
}

void ICACHE_FLASH_ATTR Return_Ota_result(uint8 result)
{
	uint8 respond[3];
	os_memset(respond,0,3);
	respond[0]=0x38;
	respond[1]=0x03;
	respond[2]=result;
	uart0_tx_buffer(respond,3);	
}

void ICACHE_FLASH_ATTR Return_downmcu_result(uint8 result)
{
	uint8 respond[3];
	os_memset(respond,0,3);
	respond[0]=0x39;
	respond[1]=0x03;
	respond[2]=result;
	uart0_tx_buffer(respond,3);	
}

void ICACHE_FLASH_ATTR Return_cer_result(uint8 result)
{
	uint8 respond[3];
	os_memset(respond,0,3);
	respond[0]=0x3A;
	respond[1]=0x03;
	respond[2]=result;
	uart0_tx_buffer(respond,3);	
}

void ICACHE_FLASH_ATTR Return_http_flag(uint32 result)
{
	uint8 respond[3];
	os_memset(respond,0,3);
	respond[0]=0x3B;
	respond[1]=0x03;
	respond[2]=result;
	uart0_tx_buffer(respond,3);	
}



//解析数据 上传至云端
LOCAL void ICACHE_FLASH_ATTR
uart_data_proc(uint8 *data,int len)
{
    DBG("G_router_mode is %d\n",G_router_mode);
	DBG("uart recieve %d bytes\n",len);
    //debugging

#if 0

    if(G_url_config_flag==1)//配置服务器01 15 74 65 73 74 2E 61 70 69 2E 6D 69 72 72 6F 72 63 6C 65 2E 63 6E
    {
        if(data[0]==0x01)//第一个字符正确
        {
        	u8 host_len;
        	u8 url_len;
		uint8 *host_start;
		uint8 *url_start;
		uint8 *tail;
		host_start=&data[2];
		uint8 *port_addr=(u8 *)os_strstr(host_start,":");
		host_len=port_addr-host_start;
		port_addr++;
		DBG("host_len:%d\n",host_len);
            os_memset(tcp_mess.hostname,0,100);//data[1]是url长度，包括端口号
            os_memcpy(tcp_mess.hostname,host_start,host_len); 
            tcp_mess.flag=0xfdc0;
            
            uint8 i;
		
		url_start=(u8 *)os_strstr(port_addr,"/");
		uint8 port_len=url_start-port_addr;
		DBG("hostname:%s\n",tcp_mess.hostname);
		DBG("##port_len:%d\n",port_len);
		
            tcp_mess.port=0;
            for(i=0;i<port_len;i++)
            {
                tcp_mess.port*=10;
                tcp_mess.port+=(port_addr[i]-48);//获取端口

            }
			DBG("##port:%d\n",tcp_mess.port);
			tail=host_start+data[1];
			url_len=tail-url_start;
			os_printf("url_len:%d\n",url_len);
			os_memset(tcp_mess.url,0,100);//
			os_memcpy(tcp_mess.url,url_start,url_len); 
			os_printf("url:%s\n",tcp_mess.url);
            spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0);
            spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_0) * SPI_FLASH_SEC_SIZE,
                            (uint32 *)&tcp_mess, sizeof(tcp_mess));
	
            host_conf_send(1);//返回配置url成功
            G_url_config_flag=0;
            tcp_clent_conn();//连接服务器
        }
        else
        {
            //URLCFG_flag=0;
            host_conf_send(0);//返回配置url失败
        }
    }
    else if(0x35==data[0])//查询时间
    {
    	if(0x02==data[1])	
        	user_Sntp_Init();
    }
else if(0x36==data[0])//查询mac
    {
    	if(0x02==data[1])
        	Return_mac();
    }
else if(0x37==data[0])//查询模块固件信息
    {
	if(0x02==data[1])
        	Return_Firmware_message();
    }
else if((0x38==data[0]))//更新模块程序
    {
    		u16 length;
		os_memcpy(&length,data+1,2);
		length-=3;
		os_printf("msgpack length:%d\n",length);//包含msgpack的结构体长度
    		if(data[7]!=get_checksum1(data+3,length-5))
    		{
    			os_printf("ota get checksum error!\n");
			return;
    		}
   		if(G_download_type==0&&G_download_flag==0&&G_server_mode==SERVER_CONNECT)
		{
			Return_Ota_result(1);
			G_download_flag=1;
			G_download_type=1;
			DB("###OTA START!#####\n");
			os_memset(tcp_host_data,0,sizeof(tcp_host_data));
        		os_memcpy(tcp_host_data,data+3,length);//先把整个包含msgpack的结构体包拷过来
			uint8 ota_url[125]={0};
			os_sprintf(ota_url,"%s%s%s","http://",tcp_mess.hostname,FILE_URL);
			os_printf("UPDATE FROM:%s\n",ota_url);
			ota_init(ota_url);
		}
		else
		{
			Return_Ota_result(0);
		}
    }
	
	else if(0x39==data[0])//下载单片机固件
    {
    		u16 length;
		os_memcpy(&length,data+1,2);
		length-=3;
		os_printf("msgpack length:%d\n",length);//包含msgpack的结构体长度
    		if(data[7]!=get_checksum1(data+3,length-5))
    		{
    			os_printf("ota get checksum error!\n");
			return;
    		}
    		if(G_download_type==0&&G_download_flag==0&&G_server_mode==SERVER_CONNECT)
		{
			Return_downmcu_result(1);
			G_download_type=2;
			add_node(&sendqueue,data+3);
			DB("###DOWNLOAD MCU!#####\n");
			os_timer_disarm(&queue_send_t);
		       os_timer_setfn(&queue_send_t,(os_timer_func_t*)send_queue,&sendqueue);//发送队列数据
		       os_timer_arm(&queue_send_t,10,0);
    		}
		else
		{
			Return_downmcu_result(0);
		}
    }
	else if(0x3A==data[0])//下载证书
	{
		u16 length;
		os_memcpy(&length,data+1,2);
		length-=3;
		os_printf("msgpack length:%d\n",length);//包含msgpack的结构体长度
    		if(data[7]!=get_checksum1(data+3,length-5))
    		{
    			os_printf("ota get checksum error!\n");
			return;
    		}
		if(G_download_flag==0&&G_download_flag==0&&G_server_mode==SERVER_CONNECT)
		{
			Return_cer_result(1);
			G_download_type=3;
			add_node(&sendqueue,data+3);
			DB("###DOWNLOAD CER!#####\n");
			os_timer_disarm(&queue_send_t);
		       os_timer_setfn(&queue_send_t,(os_timer_func_t*)send_queue,&sendqueue);//发送队列数据
		       os_timer_arm(&queue_send_t,10,0);
    		}
		else
		{
			Return_cer_result(0);
		}
	}
	else if((0x3B==data[0])&&(data[1]==3))//切换http和https
	{
		if(0x00==data[2])//http
		{
			G_https_flag=0;
			Return_http_flag(0);
		}
		else if(0x01==data[2])//https
		{
			G_https_flag=1;
			Return_http_flag(1);
		}
		spi_flash_erase_sector(ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1);//保存工作在http或https的信息
            spi_flash_write((ESP_PARAM_START_SEC + ESP_PARAM_SAVE_1) * SPI_FLASH_SEC_SIZE,
                            (uint32 *)&G_https_flag, sizeof(G_https_flag));
	}
	else if(0x3C==data[0])//获得公众号
	{
		u8 length=data[1];
		if(get_checksum(data,length-1)!=data[length-1])//校验失败
		{
			os_printf("##get checksum error!\n");
		}
		else
		{
			os_memset(device_type,0,sizeof(device_type));
			os_memcpy(device_type,data+2,length-3);
			os_printf("device_type:%s\n",device_type);
			uart0_tx_buffer(data,length);
			airkiss_start_discover();
		}
	}
#if 1
    else if((0xAB==data[0])&&(0XCD==data[1]))//透传数据
    {
    	u16 length;
	os_memcpy(&length,data+2,2);
	os_printf("data length:%d\n",length);
	if(data[4]!=get_checksum1(data,length))//校验失败
	{
		os_printf("##get passthrough checksum error!\n");
		return;
	}
    	add_node(&sendqueue,data);
	os_timer_disarm(&queue_send_t);
       os_timer_setfn(&queue_send_t,(os_timer_func_t*)send_queue,&sendqueue);//发送队列数据
       os_timer_arm(&queue_send_t,10,0);

        
    }
	#endif
#endif

}


uint8 get_checksum(uint8_t *buf,uint8_t len)
{
	uint16_t sum=0;
	uint8_t i;
	for(i=0;i<len;i++)
	{
		sum+=*buf;
		buf++;
	}
	os_printf("##sum=:%x\n",(uint8)sum);
	return (uint8)sum;
}


uint8 get_checksum1(uint8_t *buf,u16 len)
{
	u32 sum=0;
	u16 i;
	buf+=5;//移动到data开始检验
	for(i=0;i<len;i++)
	{
		sum+=*buf;
		buf++;
	}
	os_printf("##sum=:%x\n",(uint8)sum);
	return (uint8)sum;
}
/************SBSQ*********************/








LOCAL void ICACHE_FLASH_ATTR uart_data_check(void *arg)
{
	os_printf("##uart_data_check\n");
    ETS_UART_INTR_DISABLE();

    	uart_data_t **data = arg;
    	uint8_t len;
	uint8_t check_sum;
    //debug_print_hex((*data)->buf,(*data)->size);
    //uart_data_proc()
    //os_printf("xmd:%x\n",(*data)->buf[0] );
	//os_printf("size:%d\n",(*data)->size);
    uart_data_proc((*data)->buf, (*data)->size);
    (*data)->size = 0;

    ETS_UART_INTR_ENABLE();
}

/*
 * @功能：将串口0收到的数据进行打包发送给app
 */
void uart_data_recv_proc(uint8_t data)
{
    static ETSTimer timer;

    //DBG("##uart_data_callbcak:%02x\n",data);

    if(uart_data == NULL)
    {
        uart_data = (uart_data_t *)os_malloc(sizeof(uart_data_t));
        if (uart_data == NULL)
        {
            return;
        }
        uart_data->size = 0;
        TIMER_REGISTER(&timer, uart_data_check, &uart_data, UART_DATA_PACKAGE_TIME_OUT, 0);
    }

    uart_data->buf[uart_data->size++] = data;

    if(uart_data->size >= UART_DATA_BUF_LEN)
    {
        os_timer_disarm(&timer);
        uart_data_check(uart_data);
    }

    TIMER_START(&timer, UART_DATA_PACKAGE_TIME_OUT, 0);
}

void ICACHE_FLASH_ATTR uart_data_recvTask(os_event_t *events)
{
    if(events->sig == 1)
    {
        uint8 vchar = (uint8)(events->par);

        uart_data_recv_proc(vchar);
		//DBG("receive a char :%x\n",vchar);
    }
    else
    {
        DBG("uart_data_recvTask should not receive the sig =%d\n",events->sig);
    }
}



//串口程序初始化

void ICACHE_FLASH_ATTR uart_proc_init()
{
	//queue_init(sendqueue);
	system_os_task(uart_data_recvTask, 1,WIFI_recvTaskQueue , WIFI_recvTaskQueueLen);

	//wifi_send_list_init();
}




