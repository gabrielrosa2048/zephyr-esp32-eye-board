#include "wifi.h"

#define WIFI_SSID       "Apto_1404"
#define WIFI_PASSWORD   "zurick1140"

#define NET_EVENT_WIFI_MASK (NET_EVENT_WIFI_SCAN_RESULT | NET_EVENT_WIFI_SCAN_DONE | \
                             NET_EVENT_WIFI_CONNECT_RESULT | NET_EVENT_WIFI_DISCONNECT_RESULT | \
                             NET_EVENT_WIFI_AP_STA_CONNECTED | NET_EVENT_WIFI_AP_STA_DISCONNECTED)

LOG_MODULE_REGISTER(wifi, LOG_LEVEL_INF);

static struct net_if *iface;

static struct wifi_connect_req_params sta_config;

static struct net_mgmt_event_callback cb;

static void wifi_event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event, struct net_if *iface){

    if(mgmt_event == NET_EVENT_WIFI_SCAN_RESULT){

        const struct wifi_scan_result *result = (const struct wifi_scan_result *)cb->info;

        LOG_INF("SSID: %-32s RSSI: %d", result->ssid, result->rssi);

        k_msleep(10);
    
    } else if(mgmt_event == NET_EVENT_WIFI_SCAN_DONE){

        LOG_INF("Scan Finished");
    
    } else if(mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT){

        struct wifi_iface_status status = {0};

        net_mgmt(NET_REQUEST_WIFI_IFACE_STATUS, iface, &status, sizeof(status));

        k_msleep(10);
        LOG_INF("Connected: %s", status.ssid);
        k_msleep(10);
        LOG_INF("BSSID: %02x:%02x:%02x:%02x:%02x:%02x", status.bssid[0], status.bssid[1], status.bssid[2], status.bssid[3], status.bssid[4], status.bssid[5]);
    }
}

void wifi_connect(char *ssid, char *password){

	sta_config.ssid = (const uint8_t *)ssid;
	sta_config.ssid_length = strlen(ssid);
	sta_config.psk = (const uint8_t *)password;
	sta_config.psk_length = strlen(password);
	sta_config.security = WIFI_SECURITY_TYPE_PSK;
	sta_config.channel = WIFI_CHANNEL_ANY;
	sta_config.band = WIFI_FREQ_BAND_2_4_GHZ;

	LOG_INF("Connecting to SSID: %s\n", sta_config.ssid);

	int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &sta_config, sizeof(struct wifi_connect_req_params));
	
    if (ret) {
	
        LOG_ERR("Unable to Connect to (%s)", ssid);
	}
}

void wifi_thread(){

    LOG_INF("Wifi started");

    net_mgmt_init_event_callback(&cb, wifi_event_handler, NET_EVENT_WIFI_MASK);
	net_mgmt_add_event_callback(&cb);

    iface = net_if_get_wifi_sta();

    // net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0);

    wifi_connect(WIFI_SSID, WIFI_PASSWORD);

    while(1){


        k_sleep(K_SECONDS(1));
    }
}

K_THREAD_DEFINE(wifi_tid, 8192, wifi_thread, NULL, NULL, NULL, 7, 0, 0); 
