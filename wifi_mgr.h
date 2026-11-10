#ifndef WIFI_MGR_H
#define WIFI_MGR_H

#include <Arduino.h>

void wifi_init();
void wifi_service();     // call every loop - non-blocking connect/reconnect
bool wifi_isConnected();

#endif // WIFI_MGR_H
