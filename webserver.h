#ifndef WEBSERVER_MGR_H
#define WEBSERVER_MGR_H

#include <Arduino.h>

void webserver_init();
void webserver_service();  // call every loop - handleClient()

#endif // WEBSERVER_MGR_H
