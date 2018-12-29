#ifndef __user_device_h
#define __user_device_h



/* From device.h */
#define PRODUCT_KEY_LEN     (20)                   
#define DEVICE_NAME_LEN     (32)					 /*设备名字*/
#define DEVICE_ID_LEN       (64)                     /*设备ID 长度*/
#define DEVICE_SECRET_LEN   (64)                     /*密码长度*/
#define PRODUCT_SECRET_LEN  (64)                     /*产品密钥*/

#define LINKKIT_VERSION     "2.2.1"
#define MODULE_VENDOR_ID    (32)   					 /* Partner ID */

#define HOST_ADDRESS_LEN    (128)
#define HOST_PORT_LEN       (8)
#define CLIENT_ID_LEN       (256)
#define USER_NAME_LEN       (512)  				 	 /* Extend length for ID2 */
#define PASSWORD_LEN        (256)  					 /* Extend length for ID2 */
#define AESKEY_STR_LEN      (32)
#define AESKEY_HEX_LEN      (128/8)


typedef struct {
    char        product_key[PRODUCT_KEY_LEN + 1];
    char        device_name[DEVICE_NAME_LEN + 1];
    char        device_id[DEVICE_ID_LEN + 1];
    char        device_secret[DEVICE_SECRET_LEN + 1];
    char        module_vendor_id[MODULE_VENDOR_ID + 1];
} iotx_device_info_t, *iotx_device_info_pt;



typedef struct {
    uint16_t        port;
    char            host_name[HOST_ADDRESS_LEN + 1];
    char            client_id[CLIENT_ID_LEN + 1];
    char            username[USER_NAME_LEN + 1];
    char            password[PASSWORD_LEN + 1];
    const char     *pub_key;
} iotx_conn_info_t, *iotx_conn_info_pt;







#endif



