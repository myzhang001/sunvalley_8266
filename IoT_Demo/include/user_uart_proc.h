#ifndef __user_uart_proc_h
#define __user_uart_proc_h


#include "c_types.h"



typedef void (*uart_receive_callback)(uint8 data);




extern uart_receive_callback uart_receive_callback_handle ;













#endif


