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

// ==========================================
// MANUAL PHYSICAL SWITCHES (local override, works without WiFi)
// ==========================================
#define SWITCH_LIGHT_1  18
#define SWITCH_LIGHT_2  19
#define SWITCH_FAN_1    23
#define SWITCH_DOOR     15

#endif // PINS_H
