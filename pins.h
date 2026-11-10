#ifndef PINS_H
#define PINS_H

// ==========================================
// RELAYS (Active LOW modules)
// ==========================================
#define RELAY_LIGHT_1   13
#define RELAY_LIGHT_2   14
#define RELAY_FAN_1     16
#define RELAY_FAN_2     17
#define RELAY_DOOR_LOCK 21
#define RELAY_PUMP      25
#define BUZZER_PIN      27

#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// ==========================================
// I2C BUS (shared: BME280, BH1750, OLED)
// ==========================================
#define I2C_SDA         32
#define I2C_SCL         33

// ==========================================
// DIGITAL SENSORS
// ==========================================
#define PIR_PIN         26

// ==========================================
// ANALOG SENSORS (ADC1 only - safe to use with WiFi)
// ==========================================
#define MQ2_PIN         34   // Smoke / general combustible gas
#define MQ5_PIN         36   // LPG / natural gas
#define WATER_PIN       35   // Water level (resistive/float analog)

// ==========================================
// RFID (VSPI)
// ==========================================
#define RFID_SS_PIN     5
#define RFID_RST_PIN    22

// ==========================================
// PZEM-004T POWER METER (UART2, software-driven)
// GPIO4 is free now that DHT22 has been replaced by the I2C BME280.
// ==========================================
#define PZEM_RX_PIN     4    // ESP32 RX2 <- PZEM TX
#define PZEM_TX_PIN     2    // ESP32 TX2 -> PZEM RX

// RFID uses the default VSPI bus: SCK=18, MISO=19, MOSI=23 (wired, not
// #defined - SPI.begin() picks them up). Do NOT reuse 18/19/23 elsewhere.

// ==========================================
// MANUAL PHYSICAL SWITCHES (local override, works without WiFi)
// Momentary push-buttons between the pin and GND.
//   GPIO12 / GPIO15 / GPIO0 are boot-strapping pins: fine for buttons to
//   GND, but don't hold them pressed while the board is powering up.
//   GPIO39 is input-only with NO internal pull-up: fit a 10k resistor
//   from GPIO39 to 3V3.
// ==========================================
#define SWITCH_LIGHT_1  12
#define SWITCH_LIGHT_2  39   // needs external 10k pull-up to 3V3
#define SWITCH_FAN_1    0    // also the on-board BOOT button on most dev kits
#define SWITCH_DOOR     15

#endif // PINS_H
