#ifndef BTOOTH_H
  #define BTOOTH_H
  #include "Arduino.h"
  #include <prj_conf_bluetti_bridge.h>
  #include <md_util.h>
  #if (USE_CONRAD_UIP)
      #include <NimBLEDevice.h>
    #endif
  #if (USE_BLUETTI)
      #include "BLEDevice.h"
      #include "PayloadParser.h"
      /*
        static boolean doConnect = false;
        static boolean connected = false;
        static boolean doScan = false;
        static BLERemoteCharacteristic* pRemoteWriteCharacteristic;
        static BLERemoteCharacteristic* pRemoteNotifyCharacteristic;
        static BLEAdvertisedDevice* pbluettiDevice;

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
      typedef struct
        {
          //int  salt = EEPROM_SALT;
          //char mqtt_server[40] = "127.0.0.1";
          char mqtt_server[40] = "10.0.0.230";
          char mqtt_port[6]  = "1883";
          char mqtt_username[40] = "";
          char mqtt_password[40] = "";
          //char bluetti_device_id[40] = "Bluetti Blutetooth Id";
          #if (USE_BLUETTI > OFF)
              char bluetti_device_id[40] = "AC3002235000574654";
            #endif
          #if (USE_CONRAD_UIP > OFF)
              char conrad_UIP_dev_id[40] = "Voltcraft";
            #endif
          char ota_username[40] = "MAMD-HomeG";
          char ota_password[40] = "ElaNanniRalf3";
        } ESPBluettiSettings;
      // function declaration
        device_field_data_t* getpDevField();
        void initBluetooth();
        void handleBluetooth();
        bool connectToServer();
        void handleBTCommandQueue();
        void scanBluetti(uint8_t _page, uint8_t _offset = 0, uint8_t _words = 0x40);
        void sendBTCommand(bt_command_t command);
        bool isBTconnected();
        unsigned long getLastBTMessageTime();
      // export data structures
        //extern String map_field_name(enum field_index f_name);
        //static device_field_data_t bluetti_device_state[];
        //extern device_field_data_t bluetti_device_command[];
        //extern device_field_data_t bluetti_polling_command[];
    #endif
  #if (USE_CONRAD_UIP)
      class ClientCallbacksVolt  : public NimBLEClientCallbacks
        {
          void onConnect(NimBLEClient* pClient);
          void onDisconnect(NimBLEClient* pClient);
          bool onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params);
        };
      class VoltCraft
        {
          public:
              VoltCraft();
              void InitBLE();
              void ReadVoltCraft(std::string address, bool daily = false); // public call with MAC Adresse as String
              void ReadVoltCraft(NimBLEAddress address, bool daily = false); //called internally

              static std::string mode;
              static uint8_t poweron;
              static float watt;
              static uint8_t volt;
              static float ampere;
      		    static uint8_t frequency;
              static float consumption;
              static float  kwh[24]; // keeps last 23 hour on Daily Request
              static bool measure_has_data; // is true, if NotifyCallBack measure Data is called
              static bool daily_has_data; // is true, if NotifyCallBack daily Data is called
          private:
              static void NotifyCallbackVolt(BLERemoteCharacteristic* pBLERemoteCharacteristic,uint8_t* pData, size_t length, bool isNotify);
        };
    #endif
#endif
