/******************************************************************************


v_upgrade.c

*******************************************************************************/
#include "ets_sys.h"
#include "os_type.h"
#include "mem.h"
#include "osapi.h"
#include "user_interface.h"

#include "espconn.h"
#include "user_esp_platform.h"
#include "upgrade.h"



#define ESP_DEBUG

#ifdef ESP_DEBUG
#define ESP_DBG os_printf
#else
#define ESP_DBG
#endif


#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"

LOCAL os_timer_t client_timer;

void v_user_esp_platform_check_ip(void);

/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_cb
 * Description  : Processing the downloaded data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
v_user_esp_platform_upgrade_rsp(void *arg)
{
    struct upgrade_server_info *server = arg;
    if (server->upgrade_flag == true) {
    	ESP_DBG("user_esp_platform_upgrade_successful\n");

    } else {
        ESP_DBG("user_esp_platform_upgrade_failed\n");

    }

    os_free(server->url);
    server->url = NULL;
    os_free(server);
    server = NULL;
}

/******************************************************************************
 * FunctionName : user_esp_platform_upgrade_begin
 * Description  : Processing the received data from the server
 * Parameters   : pespconn -- the espconn used to connetion with the host
 *                server -- upgrade param
 * Returns      : none
*******************************************************************************/
LOCAL void ICACHE_FLASH_ATTR
v_user_esp_platform_upgrade_begin(struct espconn *pespconn, struct upgrade_server_info *server)
{
    uint8 user_bin[9] = {0};


    server->pespconn = pespconn;
    server->port = 80;
    server->check_cb = v_user_esp_platform_upgrade_rsp;
    server->check_times = 120000;
    const char esp_server_ip[4] = {52,68,48,85};
    os_memcpy(server->ip, esp_server_ip, 4);

    if (server->url == NULL) {
        server->url = (uint8 *)os_zalloc(512);
    }
#if 0 // in general, we need to check user bin here
    if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1) {
        os_memcpy(user_bin, "user2.bin", 10);
    } else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2) {
        os_memcpy(user_bin, "user1.bin", 10);
    }
    
    os_sprintf(server->url, "GET /%s HTTP/1.0\r\nHost: "IPSTR":%d\r\n"pheadbuffer"",
           user_bin, IP2STR(server->ip),
           80);
#else

    os_sprintf(server->url, "GET /%s HTTP/1.0\r\nHost: "IPSTR":%d\r\n"pheadbuffer"",
           "download/file/user1.1024.new.bin", IP2STR(server->ip),
           80);


    //http://192.168.31.229/test/user1.1024.new.2.bin

    //52.68.48.85/download/file/user1.1024.new.bin

#endif


    if (system_upgrade_start(server) == false) {

        ESP_DBG("upgrade is already started\n");
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : espconn struct parame init when get ip addr
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
v_user_esp_platform_check_ip(void)
{
    struct ip_info ipconfig;

    os_timer_disarm(&client_timer);

    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (ipconfig.ip.addr != 0) {
    	struct espconn *pespconn = NULL;
    	struct upgrade_server_info *server = NULL;
    	server = (struct upgrade_server_info *)os_zalloc(sizeof(struct upgrade_server_info));
    	v_user_esp_platform_upgrade_begin(pespconn , server);
    } else {
        os_timer_setfn(&client_timer, (os_timer_func_t *)v_user_esp_platform_check_ip, NULL);
        os_timer_arm(&client_timer, 100, 0);
    }
}

/******************************************************************************
 * FunctionName : user_esp_platform_init
 * Description  : device parame init based on espressif platform
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
v_user_esp_platform_init(void)
{
	struct station_config *config = (struct station_config *) os_zalloc(
					sizeof(struct station_config));
//	os_sprintf(config->ssid, AP_SSID);
//	os_sprintf(config->password, AP_PASSWORD);

	wifi_station_set_config(config);
	wifi_set_opmode(STATIONAP_MODE);

    if (wifi_get_opmode() != SOFTAP_MODE) {
        os_timer_disarm(&client_timer);
        os_timer_setfn(&client_timer, (os_timer_func_t *)v_user_esp_platform_check_ip, NULL);
        os_timer_arm(&client_timer, 100, 0);
    }
}

