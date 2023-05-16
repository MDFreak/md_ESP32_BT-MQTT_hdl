#ifndef BTOOTH_H
#define BTOOTH_H
#include "Arduino.h"
#include "BLEDevice.h"
#include "Config.h"
#include "Device_AC300.h"
#include <md_defines.h>

/*
  static boolean doConnect = false;
  static boolean connected = false;
  static boolean doScan = false;
  static BLERemoteCharacteristic* pRemoteWriteCharacteristic;
  static BLERemoteCharacteristic* pRemoteNotifyCharacteristic;
  static BLEAdvertisedDevice* bluettiDevice;

  // The remote Bluetti service we wish to connect to.
  static BLEUUID serviceUUID("0000ff00-0000-1000-8000-00805f9b34fb");

  // The characteristics of Bluetti Devices
  static BLEUUID    WRITE_UUID("0000ff02-0000-1000-8000-00805f9b34fb");
  static BLEUUID    NOTIFY_UUID("0000ff01-0000-1000-8000-00805f9b34fb");
 */

typedef struct __attribute__ ((packed))
  {
    uint8_t  prefix;            // 1 byte
    uint8_t  field_update_cmd;  // 1 byte
    uint8_t  page;              // 1 byte
    uint8_t  offset;            // 1 byte
    uint16_t len;               // 2 bytes
    uint16_t check_sum;         // 2 bytes
  } bt_command_t;

typedef struct{
  //int  salt = EEPROM_SALT;
  //char mqtt_server[40] = "127.0.0.1";
  char mqtt_server[40] = "10.0.0.230";
  char mqtt_port[6]  = "1883";
  char mqtt_username[40] = "";
  char mqtt_password[40] = "";
  //char bluetti_device_id[40] = "Bluetti Blutetooth Id";
  char bluetti_device_id[40] = "AC3002235000574654";
  char ota_username[40] = "MAMD-HomeG";
  char ota_password[40] = "ElaNanniRalf3";
} ESPBluettiSettings;

extern void initBluetooth();
extern void handleBluetooth();
bool connectToServer();
extern void handleBTCommandQueue();
extern void sendBTCommand(bt_command_t command);
extern bool isBTconnected();
extern unsigned long getLastBTMessageTime();
String map_field_name(enum field_names f_name);

#endif
