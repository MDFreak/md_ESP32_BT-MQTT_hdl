#ifndef BTOOTH_H
  #define BTOOTH_H
  #include "Arduino.h"
  #include "BLEDevice.h"
  #include <prj_conf_BT_MQTT_hdl.h>
  //#include "Config.h"
  //#include "Device_AC300.h"
  #include "Bluetti_PayloadParser.h"
  //#include <md_defines.h>
  #include <md_util.h>

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

  // function declaration
    device_field_data_t* getpDevField();
    void initBluetooth();
    void handleBluetooth();
    bool connectToServer();
    void handleBTCommandQueue();
    void sendBTCommand(bt_command_t command);
    bool isBTconnected();
    unsigned long getLastBTMessageTime();
#endif
