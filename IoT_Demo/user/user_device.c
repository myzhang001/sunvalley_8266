#include "user_device.h"
#include "string.h"


static iotx_conn_info_t   iot_conn_info;
static iotx_device_info_t iot_device_info;
static int 				  iot_devinfo_inited = 0;



int iot_device_info_init(void)
{
	if(iot_devinfo_inited)
	{
		os_printf("device_info already created,return!");
		return 0;
	}

	




}













