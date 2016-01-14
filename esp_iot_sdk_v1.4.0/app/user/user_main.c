/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/
#include "ets_sys.h"
#include "osapi.h"

#include "user_interface.h"

#include "user_devicefind.h"
#include "user_webserver.h"

#if ESP_PLATFORM
#include "user_esp_platform.h"
#endif


#include "user_interface.h"
#include "smartconfig.h"

#include "driver/uart.h"

#include "v_upgrade.h"



LOCAL os_timer_t test_timer;

/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : check whether get ip addr or not
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
v_user_esp_platform_check_ip(void)
{
    struct ip_info ipconfig;

   //disarm timer first
    os_timer_disarm(&test_timer);

   //get ip info of ESP8266 station
    wifi_get_ip_info(STATION_IF, &ipconfig);

    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {

      os_printf("got ip !!! \r\n");

    } else {

        if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
                wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
                wifi_station_get_connect_status() == STATION_CONNECT_FAIL)) {

         os_printf("connect fail !!! \r\n");

        } else {

           //re-arm timer to check ip
            os_timer_setfn(&test_timer, (os_timer_func_t *)v_user_esp_platform_check_ip, NULL);
            os_timer_arm(&test_timer, 100, 0);
        }
    }
}


/******************************************************************************
 * FunctionName : user_set_station_config
 * Description  : set the router info which ESP8266 station will connect to
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_set_station_config(void)
{
   // Wifi configuration
   ///char ssid[32] = SSID;
   ///char password[64] = PASSWORD;

   char ssid[32] = "2701";
   char password[64] = "liuming123";

   struct station_config stationConf;

   os_memset(stationConf.ssid, 0, 32);
   os_memset(stationConf.password, 0, 64);
   //need not mac address
   stationConf.bssid_set = 0;

   //Set ap settings
   os_memcpy(&stationConf.ssid, ssid, 32);
   os_memcpy(&stationConf.password, password, 64);
   wifi_station_set_config(&stationConf);

   //set a timer to check whether got ip from router succeed or not.
   os_timer_disarm(&test_timer);
    os_timer_setfn(&test_timer, (os_timer_func_t *)v_user_esp_platform_check_ip, NULL);
    os_timer_arm(&test_timer, 100, 0);

}


/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
/*
void user_init(void)
{
    os_printf("SDK version:%s\n", system_get_sdk_version());

   //Set softAP + station mode
   wifi_set_opmode(STATIONAP_MODE);

   // ESP8266 connect to router.
    user_set_station_config();

}
*/



void ICACHE_FLASH_ATTR
smartconfig_done(sc_status status, void *pdata)
{
    switch(status) {
        case SC_STATUS_WAIT:
            os_printf("SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            os_printf("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            os_printf("SC_STATUS_GETTING_SSID_PSWD\n");
			sc_type *type = pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
                os_printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
                os_printf("SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        case SC_STATUS_LINK:
            os_printf("SC_STATUS_LINK\n");
            struct station_config *sta_conf = pdata;

	        wifi_station_set_config(sta_conf);
	        wifi_station_disconnect();
	        wifi_station_connect();
            break;
        case SC_STATUS_LINK_OVER:
            os_printf("SC_STATUS_LINK_OVER\n");
            if (pdata != NULL) {
                uint8 phone_ip[4] = {0};

                os_memcpy(phone_ip, (uint8*)pdata, 4);
                os_printf("Phone ip: %d.%d.%d.%d\n",phone_ip[0],phone_ip[1],phone_ip[2],phone_ip[3]);
            }
            smartconfig_stop();
            break;
    }

}


//static os_timer_t myuart_test_timer;


void user_rf_pre_init(void)
{
}



static void ICACHE_FLASH_ATTR
vtest_uart()
{

//	uart1_sendStr_no_wait("uart1_sendStr_no_wait ... vicly start sent uart1...;");  //display line
	PRINTF("PRINTF:uart0_sendStr q1  ... \n");
//	uart0_sendStr("q1\n");



	return;
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{

    uart_init(115200, 115200);

    UART_SetPrintPort(0);


    os_printf("init start, SDK version:%s\n", system_get_sdk_version());

    os_sprintf("vicly rom 20150109001");



   //Set softAP + station mode
   wifi_set_opmode(STATIONAP_MODE);

   // ESP8266 connect to router.
    user_set_station_config();


    /*
    static os_timer_t myuart_test_timer;
	os_timer_disarm(&myuart_test_timer);
	//os_timer_setfn(&myuart_test_timer, (os_timer_func_t *)vtest_uart, NULL);
	os_sprintf("os_sprintf:vicly test os time...;;");
	PRINTF("timer:PRINTF:uart0_sendStr q1  ... \n");
	os_timer_arm(&myuart_test_timer, 15000, 1);   //call set_usart_gpu_content 15sec each
*/


    /*
    //---------------smart config------------------s
//    os_printf("SDK version:%s\n", system_get_sdk_version());
	///smartconfig_set_type(SC_TYPE_ESPTOUCH); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
	//smartconfig_set_type(SC_TYPE_AIRKISS); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS
	smartconfig_set_type(SC_TYPE_ESPTOUCH); //SC_TYPE_ESPTOUCH,SC_TYPE_AIRKISS,SC_TYPE_ESPTOUCH_AIRKISS

	wifi_set_opmode(STATION_MODE);
    smartconfig_start(smartconfig_done);
    //---------------smart config------------------e
*/


///#if ESP_PLATFORM
    /*Initialization of the peripheral drivers*/
    /*For light demo , it is user_light_init();*/
    /* Also check whether assigned ip addr by the router.If so, connect to ESP-server  */
///    user_esp_platform_init();
///#endif
    /*Establish a udp socket to receive local device detect info.*/
    /*Listen to the port 1025, as well as udp broadcast.
    /*If receive a string of device_find_request, it rely its IP address and MAC.*/
///    user_devicefind_init();







user_esp_platform_init();
///	v_user_esp_platform_init();

    /*Establish a TCP server for http(with JSON) POST or GET command to communicate with the device.*/
    /*You can find the command in "2B-SDK-Espressif IoT Demo.pdf" to see the details.*/
    /*the JSON command for curl is like:*/
    /*3 Channel mode: curl -X POST -H "Content-Type:application/json" -d "{\"period\":1000,\"rgb\":{\"red\":16000,\"green\":16000,\"blue\":16000}}" http://192.168.4.1/config?command=light      */
    /*5 Channel mode: curl -X POST -H "Content-Type:application/json" -d "{\"period\":1000,\"rgb\":{\"red\":16000,\"green\":16000,\"blue\":16000,\"cwhite\":3000,\"wwhite\",3000}}" http://192.168.4.1/config?command=light      */

    /*
#ifdef SERVER_SSL_ENABLE
    user_webserver_init(SERVER_SSL_PORT);
#else
    user_webserver_init(SERVER_PORT);
#endif
*/

}

