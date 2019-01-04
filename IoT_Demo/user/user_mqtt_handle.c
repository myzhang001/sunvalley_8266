#include "user_mqtt_handle.h"







/*



*/
void user_mqtt_init(void)
{

	/* Initialize MQTT parameter */



}


#define CMD_LENGTH_CONNECT   (10 + 10)



/*
	发起连接过程
*/ 
void user_mqtt_cmd_conected(void)
{

	static uint8_t s_send_buff[50];

	//fixed header
	s_send_buff[0] = (uint8_t)(E_FIXED_CONNECT << 4);

	//***remaining length   
	s_send_buff[1] = CMD_LENGTH_CONNECT >> 24;
	s_send_buff[2] = CMD_LENGTH_CONNECT >> 16;
    s_send_buff[3] = CMD_LENGTH_CONNECT >> 8;
    s_send_buff[4] = CMD_LENGTH_CONNECT;


	//variable header
	
	//protocol name length  2byte

	s_send_buff[5] = 0x00;
	s_send_buff[6] = 0x00;

	s_send_buff[7] = 'M';
	s_send_buff[8] = 'Q';
	s_send_buff[9] = 'T';
	s_send_buff[10] = 'T';

	//protocol level

	s_send_buff[11] = 0x01;

	//connected flag
	s_send_buff[12]= 0x00;

	//keep alive  2byte
	s_send_buff[13] = (uint8_t)(KEEP_ALIVE_TIME >> 8);
	s_send_buff[14] = (uint8_t)(KEEP_ALIVE_TIME);
	

	//paylod

	//*** client id
	s_send_buff[15] = 0x00;
}








