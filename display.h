#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void display_init();
void display_service();  // call every loop; internally throttled to DISPLAY_INTERVAL_MS

#endif // DISPLAY_H
