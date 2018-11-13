
#include "user_uart_proc.h"

#include "driver/uart.h"




 uart_receive_callback uart_receive_callback_handle = NULL;



void ICACHE_FLASH_ATTR
uart_receive_callback_regist(uart_receive_callback recv_callback)
{
	uart_receive_callback_handle = recv_callback;
}

