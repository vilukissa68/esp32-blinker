#ifndef WIFI_H_
#define WIFI_H_

#include "esp_wifi.h"


void wifi_init();
int wifi_scan();
int wifi_connect_default();

#endif // WIFI_H_
