#include "user_device.h"
#include "string.h"
#include "user_errno.h"
#include "stdio.h"





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

	memset(&iot_device_info,0x00,sizeof(iot_device_info));
	memset(&iot_conn_info,0x00,sizeof(iot_conn_info));
	iot_devinfo_inited  = 1;


	os_printf("device_info created successfully!");

	return SUCCESS_RETURN;
}



uint8_t iot_device_info_set(const char *product_key,
							const char *device_name,
							const char *device_secret)
{
	uint8_t ret;
	os_printf("start to set device info !");

	memset(&iot_device_info, 0x00, sizeof(iot_device_info));
    strncpy(iot_device_info.product_key, product_key, PRODUCT_KEY_LEN);
    strncpy(iot_device_info.device_name, device_name, DEVICE_NAME_LEN);
    strncpy(iot_device_info.device_secret, device_secret, DEVICE_SECRET_LEN);

    /* construct device-id(@product_key+@device_name) */
    ret = snprintf(iot_device_info.device_id, DEVICE_ID_LEN, "%s.%s", product_key, device_name);
    if ((ret < 0) || (ret >= DEVICE_ID_LEN)) {
        os_printf("set device info failed");
        return FAIL_RETURN;
    }

    os_printf("device_info set successfully!");
    return SUCCESS_RETURN;
}


iotx_device_info_pt iotx_device_info_get(void)
{
    return &iot_device_info;
}

iotx_conn_info_pt iotx_conn_info_get(void)
{
    return &iot_conn_info;
}






