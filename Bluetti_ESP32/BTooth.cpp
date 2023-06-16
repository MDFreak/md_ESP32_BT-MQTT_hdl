#include "BTooth.h"

#if (USE_CONRAD_UIP > OFF)

  #endif
#if (USE_BLUETTI > OFF)
  static boolean doConnect = false;
  static boolean connected = false;
  static boolean doScan    = false;
  static BLERemoteCharacteristic* pRemoteWriteCharacteristic;
  static BLERemoteCharacteristic* pRemoteNotifyCharacteristic;
  static BLEAdvertisedDevice*     pbluettiDevice;


    // The remote Bluetti service we wish to connect to.
    static BLEUUID serviceUUID("0000ff00-0000-1000-8000-00805f9b34fb");
    // The characteristics of Bluetti Devices
    static BLEUUID WRITE_UUID ("0000ff02-0000-1000-8000-00805f9b34fb");
    static BLEUUID NOTIFY_UUID("0000ff01-0000-1000-8000-00805f9b34fb");

  #if (USE_CONRAD_UIP)
    #endif

  int publishErrorCount = 0;
  unsigned long lastmqttbluMessage = 0;
  unsigned long previousDeviceStatePublish = 0;

  int      pollTick = 0;
  uint16_t scanTick = 0;
  uint8_t  isScan   = FALSE;
  #if (USE_CONRAD_UIP > OFF)
      ESPBluettiSettings _settings;
      // initialize bluetti device data
        // { FIELD_NAME, PAGE, OFFSET, SIZE, SCALE (if scale is needed e.g. decimal value, defaults to 0) , ENUM (if data is enum, defaults to 0) , FIELD_TYPE }
        static device_field_data_t bluetti_device_state[] =
          {
            // Page 0x00 Core
              // FIELD_NAME,          PVALUE,PAGE,  OFFS,  SIZ, SCAL (if scale is needed e.g. decimal value, defaults to 0)
              //                                                   ENUM (if data is enum, defaults to 0)
            // INFO device offset sort                                new FIELD_TYPE
              {DEVICE_TYPE,           NULL,  0x00,  0x0A,  7,   0, 0, 0,  STRING_FIELD},
              {SERIAL_NUMBER,         NULL,  0x00,  0x11,  4,   0 ,0, 0,  SN_FIELD},
              {ARM_VERSION,           NULL,  0x00,  0x17,  2,   0, 0, 0,  VERSION_FIELD},
              {DSP_VERSION,           NULL,  0x00,  0x19,  2,   0, 0, 0,  VERSION_FIELD},
              {DC_INPUT_POWER,        NULL,  0x00,  0x24,  1,   0, 0, 0,  UINT_FIELD},
              {AC_INPUT_POWER,        NULL,  0x00,  0x25,  1,   0, 0, 0,  UINT_FIELD},
              {AC_OUTPUT_POWER,       NULL,  0x00,  0x26,  1,   0, 0, 0,  UINT_FIELD},
              {DC_OUTPUT_POWER,       NULL,  0x00,  0x27,  1,   0, 0, 0,  UINT_FIELD},
              {POWER_GENERATION,      NULL,  0x00,  0x29,  1,   1, 0, 0,  DECIMAL_FIELD},
              {TOTAL_BATT_PERC,       NULL,  0x00,  0x2B,  1,   0, 0, 0,  UINT_FIELD},
              {AC_OUTPUT_ON,          NULL,  0x00,  0x30,  1,   0, 0, 0,  BOOL_FIELD},
              {DC_OUTPUT_ON,          NULL,  0x00,  0x31,  1,   0, 0, 0,  BOOL_FIELD},
            // INFO internal
              {AC_OUTPUT_MODE,        NULL,  0x00,  0x46,  1,   0, 0, 0,  UINT_FIELD},
              {INTERN_AC_VOLT,        NULL,  0x00,  0x47,  1,   1, 0, 0,  DECIMAL_FIELD},
              {INTERN_CURR_1,         NULL,  0x00,  0x48,  1,   1, 0, 0,  DECIMAL_FIELD},
              {INTERN_POWER_1,        NULL,  0x00,  0x49,  1,   0, 0, 0,  UINT_FIELD},
              {INTERN_AC_FREQ,        NULL,  0x00,  0x4A,  1,   2, 0, 0,  DECIMAL_FIELD},
              {INTERN_CURR_2,         NULL,  0x00,  0x4B,  1,   1, 0, 0,  DECIMAL_FIELD},
              {INTERN_POWER_2,        NULL,  0x00,  0x4C,  1,   0, 0, 0,  UINT_FIELD},
              {AC_INPUT_VOLT,         NULL,  0x00,  0x4D,  1,   1, 0, 0,  DECIMAL_FIELD},
              {INTERN_CURR_3,         NULL,  0x00,  0x4E,  1,   1, 0, 0,  DECIMAL_FIELD},
              {INTERN_POWER_3,        NULL,  0x00,  0x4F,  1,   0, 0, 0,  UINT_FIELD},
              {AC_INPUT_FREQ,         NULL,  0x00,  0x50,  1,   2, 0, 0,  DECIMAL_FIELD},
              {INT_DC_INP_VOLT,       NULL,  0x00,  0x56,  1,   1, 0, 0,  DECIMAL_FIELD},
              {INT_DC_INP_POW,        NULL,  0x00,  0x57,  1,   0, 0, 0,  UINT_FIELD},
              {INT_DC_INP_CURR,       NULL,  0x00,  0x58,  1,   1, 0, 0,  DECIMAL_FIELD},
              {PACK_NUM_MAX,          NULL,  0x00,  0x5B,  1,   0, 0, 0,  UINT_FIELD },
              {TOTAL_BATT_VOLT,       NULL,  0x00,  0x5C,  1,   1, 0, 0,  DECIMAL_FIELD},
              {TOTAL_BATT_CURR,       NULL,  0x00,  0x5D,  1,   1, 0, 0,  DECIMAL_FIELD},
              {PACK_NUM,              NULL,  0x00,  0x60,  1,   0, 0, 0,  UINT_FIELD},
              {PACK_STATUS,           NULL,  0x00,  0x61,  1,   1, 0, 0,  UINT_FIELD},
              {PACK_VOLTAGE,          NULL,  0x00,  0x62,  1,   2 ,0 ,0,  DECIMAL_FIELD},
              {PACK_BATT_PERC,        NULL,  0x00,  0x63,  1,   0, 0, 0,  UINT_FIELD},
              //{CELL_VOTAGES,          NULL,  0x00,  0x48,  1,   1, 0, 0,  },
              {PACK_BMS_VERSION,      NULL,  0x00,  0xC9,  1,   0, 0, 0,  UINT_FIELD},
            // CONTROL elements
              {UPS_MODE,              NULL,  0x0B,  0xB9,  1,   0, 0, 0,  UINT_FIELD},
              {SPLIT_PHASE_ON,        NULL,  0x0B,  0xBC,  1,   0, 0, 0,  UINT_FIELD},
              {SPLIT_PH_MACH_MODE,    NULL,  0x0B,  0xBD,  1,   0, 0, 0,  UINT_FIELD},
              {SET_PACK_NUM,          NULL,  0x0B,  0xBE,  1,   0, 0, 0,  UINT_FIELD},
              {SET_AC_OUT_ON,         NULL,  0x0B,  0xBF,  1,   0, 0, 0,  BOOL_FIELD},
              {SET_DC_OUT_ON,         NULL,  0x0B,  0xC0,  1,   0, 0, 0,  BOOL_FIELD},
              {GRID_CHANGE_ON,        NULL,  0x0B,  0xC3,  1,   0, 0, 0,  UINT_FIELD},
              {TIME_CTRL_ON,          NULL,  0x0B,  0xC5,  1,   0, 0, 0,  UINT_FIELD},
              {BATT_RANGE_START,      NULL,  0x0B,  0xC7,  1,   0, 0, 0,  UINT_FIELD},
              {BATT_RANGE_END,        NULL,  0x0B,  0xC8,  1,   0, 0, 0,  UINT_FIELD},
              {BLUETOOTH_CONN,        NULL,  0x0B,  0xDD,  1,   0, 0, 0,  UINT_FIELD},
              {AUTO_SLEEP_MODE,       NULL,  0x0B,  0xF5,  1,   0, 0, 0,  UINT_FIELD},
              {DATE_TIME,             NULL,  0x0B,  0xD7,  3,   0, 0, 0,  DATIME_FIELD},
              {LED_CONTROL,           NULL,  0x0B,  0xDA,  1,   0, 0, 0,  UINT_FIELD},
              {FIELD_UNDEFINED,       NULL,  0x0B,  0xDF,  1,   0, 0, 0,  UINT_FIELD},
          };

        static device_field_data_t bluetti_device_command[] =
            {
              /*Page 0x00 Core */
              {AC_OUTPUT_ON,            NULL,  0x0B, 0xBF, 1, 0, 0, 0,  BOOL_FIELD},
              {DC_OUTPUT_ON,            NULL,  0x0B, 0xC0, 1, 0, 0, 0,  BOOL_FIELD}
            };
        static device_field_data_t bluetti_polling_command[] =
          {
            {FIELD_UNDEFINED,         NULL,  0x00, 0x0A, 0x12 ,0 , 0, 0,  TYPE_UNDEFINED}, // Device_TYPE     - DSP_VERSION
            {FIELD_UNDEFINED,         NULL,  0x00, 0x24, 0x08 ,0 , 0, 0,  TYPE_UNDEFINED}, // DC_INPUT_POWER  - TOTAL_BATT_PERC
            {FIELD_UNDEFINED,         NULL,  0x00, 0x46, 0x14 ,0 , 0, 0,  TYPE_UNDEFINED}, // AC_OUTPUT_MODE  - INT_DC_INP_CURR
            {FIELD_UNDEFINED,         NULL,  0x00, 0x56, 0x0E ,0 , 0, 0,  TYPE_UNDEFINED}, // INT_DC_INP_VOLT - PACK_BATT_PERC
            {FIELD_UNDEFINED,         NULL,  0x00, 0x6E, 0x20 ,0 , 0, 0,  TYPE_UNDEFINED}, // alles 0
            {FIELD_UNDEFINED,         NULL,  0x00, 0xC9, 0x01 ,0 , 0, 0,  TYPE_UNDEFINED}, //     = 0
            //{FIELD_UNDEFINED,         NULL,  0x0B, 0x00, 0x30 ,0 , 0, 0,  TYPE_UNDEFINED},
            //{FIELD_UNDEFINED,         NULL,  0x0B, 0x30, 0x30 ,0 , 0, 0,  TYPE_UNDEFINED},
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xB9, 0x10 ,0 , 0, 0,  TYPE_UNDEFINED}, // SET_AC_OUT_ON   - SET_DC_OUT_ON
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xD7, 0x03 ,0 , 0, 0,  TYPE_UNDEFINED}, // time-date  JJ MM DD hh mm ss
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xDA, 0x05 ,0 , 0, 0,  TYPE_UNDEFINED},
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xDF, 0x11 ,0 , 0, 0,  TYPE_UNDEFINED},
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xF1, 0x01 ,0 , 0, 0,  TYPE_UNDEFINED},
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xF2, 0x01 ,0 , 0, 0,  TYPE_UNDEFINED},
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xF3, 0x01 ,0 , 0, 0,  TYPE_UNDEFINED},
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xF4, 0x01 ,0 , 0, 0,  TYPE_UNDEFINED},
            {FIELD_UNDEFINED,         NULL,  0x0B, 0xF5, 0x01 ,0 , 0, 0,  TYPE_UNDEFINED}
          };
        static device_field_data_t bluetti_scan_command[] =
          {
            {FIELD_UNDEFINED,         NULL,  0x00, 0x00, 0x01 ,0 , 0, 0,  TYPE_UNDEFINED}, // temporary request storage
            {FIELD_UNDEFINED,         NULL,  0x00, 0x03, 0x40 ,0 , 0, 0,  TYPE_UNDEFINED}  // storage for scan params  pages 0-3, 64 words/scan
          };
    #endif
  struct command_handle
    {
      uint8_t page;
      uint8_t offset;
      int length;
    };

  QueueHandle_t commandHandleQueue;
  QueueHandle_t sendQueue;
  unsigned long lastBTMessage = 0;
  /*
    String map_field_name(enum field_index f_index)
    {
     switch(f_name)
      {
        case DC_OUTPUT_POWER:
          return "dc_output_power";
          break;
        case AC_OUTPUT_POWER:
          return "ac_output_power";
          break;
        case DC_OUTPUT_ON:
          return "dc_output_on";
          break;
        case AC_OUTPUT_ON:
          return "ac_output_on";
          break;
        case POWER_GENERATION:
          return "power_generation";
          break;
        case TOTAL_BATT_PERC:
          return "total_battery_percent";
          break;
        case DC_INPUT_POWER:
          return "dc_input_power";
          break;
        case AC_INPUT_POWER:
          return "ac_input_power";
          break;
        case PACK_VOLTAGE:
          return "pack_voltage";
          break;
        case SERIAL_NUMBER:
          return "serial_number";
          break;
        case ARM_VERSION:
          return "arm_version";
          break;
        case DSP_VERSION:
          return "dsp_version";
          break;
        case DEVICE_TYPE:
          return "device_type";
          break;
        case UPS_MODE:
          return "ups_mode";
          break;
        case AUTO_SLEEP_MODE:
          return "auto_sleep_mode";
          break;
        case GRID_CHANGE_ON:
          return "grid_change_on";
          break;
        case INTERN_AC_VOLT:
          return "internal_ac_voltage";
          break;
        case INTERN_CURR_1:
          return "internal_current_one";
          break;
        case PACK_NUM_MAX:
          return "pack_max_num";
          break;
        default:
          return "unknown";
          break;
      }
    }
   */
  class MyClientCallback : public BLEClientCallbacks
    {
      void onConnect(BLEClient* pclient)
        {
          Serial.println(F("onConnect"));
        }
      void onDisconnect(BLEClient* pclient)
        {
          connected = false;
          Serial.println(F("onDisconnect"));
        }
    };
  MyClientCallback  locClientCallback   = MyClientCallback();
  MyClientCallback* plocClientCallback  = &locClientCallback;

  //BLEAdvertisedDevice
  /* Scan for BLE servers and find the first one that advertises the service we are looking for.
   */
  class BluettiAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
    {
      void onResult(BLEAdvertisedDevice advertisedDevice)
        {
          //Serial.print(F("BLE Advertised Device found: "));
          //Serial.println(advertisedDevice.toString().c_str());
          SVAL("BLE Advertised Device found: ", advertisedDevice.toString().c_str());
          //ESPBluettiSettings settings = get_esp32_bluetti_settings();
          // We have found a device, let us now see if it contains the service we are looking for.
          //if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID) && advertisedDevice.getName().compare(settings.bluetti_device_id))
          if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID) && advertisedDevice.getName().compare(_settings.bluetti_device_id))
            {
              Serial.println(" onResult stop scan ");
              BLEDevice::getScan()->stop();
              pbluettiDevice = new BLEAdvertisedDevice(advertisedDevice);
              doConnect = true;
              doScan = true;
            }
        }
    };
  BluettiAdvertisedDeviceCallbacks  BluettiAdvDevCallbacks  =  BluettiAdvertisedDeviceCallbacks();
  BluettiAdvertisedDeviceCallbacks* pBluettiAdvDevCallbacks = &BluettiAdvDevCallbacks;

  device_field_data_t* getpDevField()
    {
      S2HEXVAL(" getpDevField pBluetti", (uint32_t) &bluetti_device_state[0], (uint32_t) bluetti_device_state);
      init_dev_fields(&bluetti_device_state[0], &bluetti_device_command[0], &bluetti_polling_command[0], &bluetti_scan_command[0]);
      return &bluetti_device_state[0];
    }
  void     initBluetooth()
    {
      BLEDevice::init("");
      BLEScan* pBLEScan = BLEDevice::getScan();
      //pBLEScan->setAdvertisedDeviceCallbacks(new BluettiAdvertisedDeviceCallbacks());
      pBLEScan->setAdvertisedDeviceCallbacks(pBluettiAdvDevCallbacks);
      pBLEScan->setInterval(1349);
      pBLEScan->setWindow(449);
      pBLEScan->setActiveScan(true);
      pBLEScan->start(8, false);

      commandHandleQueue = xQueueCreate( 5, sizeof(bt_command_t ) );
      sendQueue = xQueueCreate( 5, sizeof(bt_command_t) );
      lastBTMessage =  millis();
    }
  static void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic,
                              uint8_t* pData, size_t length,   bool isNotify)
    {
      #ifdef DEBUG
          // pData Debug...
            //S2VAL("notifyCallback Response ", length, "Bytes");
          if (isScan == FALSE)
            {
              //Serial.println();
              serHEXdump(pData, length);
              //Serial.println();
            }
          else
            {
              //Serial.println();
              //serHEXdump(pData, length, FALSE);
              //Serial.println();
            }
        #endif
      bt_command_t command_handle;
      //SOUT((uint32_t) &command_handle); SOUT("    ");
      //if(xQueueReceive(commandHandleQueue, &command_handle, 500))
      if(xQueueReceive(commandHandleQueue, &command_handle, 500))
        {
          //SOUT(isScan); SOUT("  ");
          if (isScan == FALSE) // normal mode
            { parse_bluetooth_data(NULL, command_handle.page, command_handle.offset, pData, length); }
          else
            { parse_bluetooth_data(NULL, command_handle.page, command_handle.offset, pData, length + 0x100); }
        }
    }
  bool     connectToServer()
    {
      // create client
          SVAL("Forming a connection to ", pbluettiDevice->getAddress().toString().c_str());
      BLEClient*  pClient  = BLEDevice::createClient();
          STXT(" - Created client");
      pClient->setClientCallbacks(plocClientCallback);
      // Connect to the remove BLE Server.
      pClient->connect(pbluettiDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
          STXT(" - Connected to server");
      pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
      // Obtain a reference to the service we are after in the remote BLE server.
      BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
      if (pRemoteService == nullptr)
        {
          SVAL("Failed to find our service UUID: ", serviceUUID.toString().c_str());
          pClient->disconnect();
          return false;
        }
      STXT(" - Found our service");
      // Obtain a reference to the characteristic in the service of the remote BLE server.
      pRemoteWriteCharacteristic = pRemoteService->getCharacteristic(WRITE_UUID);
      if (pRemoteWriteCharacteristic == nullptr)
        {
          SVAL("Failed to find our characteristic UUID: ", WRITE_UUID.toString().c_str());
          pClient->disconnect();
          return false;
        }
      STXT(" - Found our Write characteristic");
      // Obtain a reference to the characteristic in the service of the remote BLE server.
      pRemoteNotifyCharacteristic = pRemoteService->getCharacteristic(NOTIFY_UUID);
      if (pRemoteNotifyCharacteristic == nullptr)
        {
          SVAL("Failed to find our characteristic UUID: ", NOTIFY_UUID.toString().c_str());
          pClient->disconnect();
          return false;
        }
      STXT(" - Found our Notifyite characteristic");
      // Read the value of the characteristic.
      if(pRemoteWriteCharacteristic->canRead())
        {
          std::string value = pRemoteWriteCharacteristic->readValue();
          SVAL("The characteristic value was: ", value.c_str());
        }
      if(pRemoteNotifyCharacteristic->canNotify())
        { pRemoteNotifyCharacteristic->registerForNotify(notifyCallback); }

      connected = true;
      return true;
    }

  void     handleBTCommandQueue()
    {
      bt_command_t command;
      if(xQueueReceive(sendQueue, &command, 0))
        {
          #ifdef DEBUG
              //STXT("handleBTComm Write Request FF02 - Value: ");
              if (isScan == FALSE) serHEXdump((uint8_t*)&command, 8);
              //else                 serHEXdump((uint8_t*)&command, 8, FALSE);
                //for(int i=0; i<8; i++)
                //  {
                //     if ( i % 2 == 0){ Serial.print(" "); };
                //     Serial.printf("%02x", ((uint8_t*)&command)[i]);
                //  }
            #endif
          pRemoteWriteCharacteristic->writeValue((uint8_t*)&command, sizeof(command),true);
        };
    }

  void     sendBTCommand(bt_command_t command)
    {
      bt_command_t cmd = command;
      xQueueSend(sendQueue, &cmd, 0);
    }

  uint16_t bt_crc16_update (uint16_t crc, uint8_t a)
    {
      int i;
      crc ^= a;
      for (i = 0; i < 8; ++i)
        {
          if (crc & 1)
              crc = (crc >> 1) ^ 0xA001;
            else
              crc = (crc >> 1);
        }
      return crc;
    }
  uint16_t bt_modbus_crc(uint8_t buf[], int len)
    {
      unsigned int crc = 0xFFFF;
      for (unsigned int i = 0; i < len; i++)
       {
        crc = bt_crc16_update(crc, buf[i]);
       }
       return crc;
    }
  void     handleBluetooth()
    {
      //STXT(" handleBluetooth ");
      if (doConnect == true)
        {
          //STXT("  doConnect ");
          if (connectToServer())
            {
              Serial.println(F("We are now connected to the Bluetti BLE Server."));
            }
          else
            {
              Serial.println(F("We have failed to connect to the server; there is nothing more we will do."));
            }
          doConnect = false;
        }
      if ((millis() - lastBTMessage) > (MAX_DISCONNECTED_TIME_UNTIL_REBOOT * 60000))
        {
          Serial.println(F("BT is disconnected over allowed limit, reboot device"));
          if (SLEEP_TIME_ON_BT_NOT_AVAIL > OFF)
              { ESP.restart(); }
              //{ esp_deep_sleep_start(); }
            else
              { ESP.restart(); }
        }
      if (connected)
        {
          //STXT("   is connected ");
          // poll for device state
          if ( millis() - lastBTMessage > BLUETOOTH_QUERY_MESSAGE_DELAY)
            {
              bt_command_t command;
              // build command[index: polltick]
                command.prefix = 0x01;
                command.field_update_cmd = 0x03;
                command.page = bluetti_polling_command[pollTick].f_page;
                command.offset = bluetti_polling_command[pollTick].f_offset;
                command.len = (uint16_t) bluetti_polling_command[pollTick].f_size << 8;
                command.check_sum = bt_modbus_crc((uint8_t*) &command,6);
              Serial.println();
              S2VAL("command  pollTick  page ", pollTick, bluetti_polling_command[pollTick].f_page);
              xQueueSend(commandHandleQueue, &command, portMAX_DELAY);
              xQueueSend(sendQueue, &command, portMAX_DELAY);

              if (pollTick == sizeof(bluetti_polling_command)/sizeof(device_field_data_t)-1 )
                {
                  pollTick = 0;
                }
              else
                {
                  pollTick++;
                }
              lastBTMessage = millis();
            }
          handleBTCommandQueue();
        }
      else
        {
          //STXT("   else ");
          if(doScan)
            {
              BLEDevice::getScan()->start(0);
            }
        }
    }

  void     pollBluetti()
    {
      if (doConnect == true)
        {
          if (connectToServer())
            {
              Serial.println(F("We are now connected to the Bluetti BLE Server."));
            }
          else
            {
              Serial.println(F("We have failed to connect to the server; there is nothing more we will do."));
            }
          doConnect = false;
        }
      if (connected)
        {
          // poll for device state
          if ( millis() - lastBTMessage > BLUETOOTH_QUERY_MESSAGE_DELAY)
            {
              bt_command_t command;
              // build command[index: polltick]
                command.prefix = 0x01;
                command.field_update_cmd = 0x03;
                command.page = bluetti_polling_command[pollTick].f_page;
                command.offset = bluetti_polling_command[pollTick].f_offset;
                command.len = (uint16_t) bluetti_polling_command[pollTick].f_size << 8;
                command.check_sum = bt_modbus_crc((uint8_t*) &command,6);
              Serial.println();
              S2VAL("command  pollTick  page ", pollTick, bluetti_polling_command[pollTick].f_page);
              xQueueSend(commandHandleQueue, &command, portMAX_DELAY);
              xQueueSend(sendQueue, &command, portMAX_DELAY);

              if (pollTick == sizeof(bluetti_polling_command)/sizeof(device_field_data_t)-1 )
                {
                  pollTick = 0;
                }
              else
                {
                  pollTick++;
                }
              lastBTMessage = millis();
            }

          handleBTCommandQueue();

        }
      else if(doScan)
        {
          BLEDevice::getScan()->start(0);
        }
      if ((millis() - lastBTMessage) > (MAX_DISCONNECTED_TIME_UNTIL_REBOOT * 60000))
        {
          Serial.println(F("BT is disconnected over allowed limit, reboot device"));
          if (SLEEP_TIME_ON_BT_NOT_AVAIL > OFF)
            { ESP.restart();
              //esp_deep_sleep_start();
            }
            else
            { ESP.restart(); }
        }
    }
  void     scanBluetti(const uint8_t _page, const uint8_t _offset, uint8_t _words)
    {
      uint8_t i;
      bt_command_t command;
      if (doConnect == true)
        { if (connectToServer()) Serial.println(F("We are now connected to the Bluetti BLE Server."));
          else                   Serial.println(F("We have failed to connect to the server; there is nothing more we will do."));
          doConnect = false;
        }
      if ((millis() - lastBTMessage) > (MAX_DISCONNECTED_TIME_UNTIL_REBOOT * 60000))
        {
          Serial.println(F("BT is disconnected over allowed limit, reboot device"));
          if (SLEEP_TIME_ON_BT_NOT_AVAIL > OFF)
              { ESP.restart(); }
              //{ esp_deep_sleep_start(); }
            else
              { ESP.restart(); }
        }
      if (connected)
        { // scan for device entries - only used for monitoring
          isScan = TRUE;
          for (i = 0 ; i < _words ; i++ )
            {
              SOUT(" scanBluetti page offs i "); SOUT(_page); SOUT(" "); SOUT(_offset); SOUT(" ");SOUT(i); SOUT(" ");
              if ( millis() - lastBTMessage > BLUETOOTH_QUERY_MESSAGE_DELAY)
                {
                  // build command[index: polltick]
                    command.prefix = 0x01;
                    command.field_update_cmd = 0x03;
                    command.page   = _page;
                    command.offset = _offset + i;
                    command.len    = (uint16_t) 1 << 8;
                    command.check_sum = bt_modbus_crc((uint8_t*) &command,6);
                    //serHEXdump((uint8_t*) &command, 8 , FALSE);

                  xQueueSend(commandHandleQueue, &command, portMAX_DELAY);
                  xQueueSend(sendQueue, &command, portMAX_DELAY);
                  lastBTMessage = millis();
                  handleBTCommandQueue();
                  sleep(1);
                }
            }
          isScan = FALSE;
        }
      else if(doScan)
        {
          BLEDevice::getScan()->start(0);
        }
      if ((millis() - lastBTMessage) > (MAX_DISCONNECTED_TIME_UNTIL_REBOOT * 60000))
        {
          Serial.println(F("BT is disconnected over allowed limit, reboot device"));
          if (SLEEP_TIME_ON_BT_NOT_AVAIL > OFF)
            { ESP.restart();
              //esp_deep_sleep_start();
            }
            else
            { ESP.restart(); }
        }
    }

  bool     isBTconnected()
    {
      return connected;
    }

  unsigned long getLastBTMessageTime()
    {
        return lastBTMessage;
    }
  #endif
#if (USE_CONRAD_UIP > OFF)
    float  VoltCraft::kwh[24] = {0};
    std::string VoltCraft::mode="";
    uint8_t VoltCraft::poweron =0;
    float VoltCraft::watt=0;
    uint8_t VoltCraft::volt=0;
    float VoltCraft::ampere=0;
    uint8_t VoltCraft::frequency=0;
    float VoltCraft::consumption=0;
    bool VoltCraft::measure_has_data=false;
    bool VoltCraft::daily_has_data=false;

    // Create a single global instance of the callback class to be used by all clients
    static ClientCallbacksVolt clientCBVolt;
    NimBLEUUID bmeVoltServiceUUID("FFF0");
    NimBLEUUID bmeVoltNotifyCharacteristicsUUID("FFF4");
    NimBLEUUID bmeVoltWriteCharacteristicsUUID("FFF3");
    //  None of these are required as they will be handled by the library with defaults.
    //                       Remove as you see fit for your needs
        void ClientCallbacksVolt::onConnect(NimBLEClient* pClient)
          {
            Serial.println("ONCONNECT - Connected");
            // After connection we should change the parameters if we don't need fast response times.
            // These settings are 150ms interval, 0 latency, 450ms timout.
            // Timeout should be a multiple of the interval, minimum is 100ms.
            // I find a multiple of 3-5 * the interval works best for quick response/reconnect.
            // Min interval: 120 * 1.25ms = 150, Max interval: 120 * 1.25ms = 150, 0 latency, 60 * 10ms = 600ms timeout
            //pClient->updateConnParams(120,120,0,60);
            pClient->updateConnParams(60,60,0,60);
          };
        void ClientCallbacksVolt::onDisconnect(NimBLEClient* pClient)
          {
            Serial.print(pClient->getPeerAddress().toString().c_str());
            Serial.println(" Disconnected - Starting scan");
          };
        //  Called when the peripheral requests a change to the connection parameters.
         // Return true to accept and apply them or false to reject and keep
         // the currently used parameters. Default will return true.
        bool ClientCallbacksVolt::onConnParamsUpdateRequest(NimBLEClient* pClient, const ble_gap_upd_params* params)
          {
            if(params->itvl_min < 24)  // 1.25ms units
              {
                    return false;
              }
            else if(params->itvl_max > 40)
              { // 1.25ms units
                  return false;
              }
            else if(params->latency > 2)
              { // Number of intervals allowed to skip
                  return false;
              }
            else if(params->supervision_timeout > 100)  // 10ms units
              {
                  return false;
              }
            return true;
          };
    VoltCraft::VoltCraft()
      {}
    void VoltCraft::InitBLE()
      {
          NimBLEDevice::init("");
      	NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
      	NimBLEDevice::setMTU(160); // ?????
          // Optional: set the transmit power, default is 3db */
        #ifdef ESP_PLATFORM
            NimBLEDevice::setPower(ESP_PWR_LVL_P9); // +9db
          #else
            NimBLEDevice::setPower(9); // +9db
          #endif
      }
    //When the BLE Server sends a new temperature reading with the notify property
    void VoltCraft::NotifyCallbackVolt(NimBLERemoteCharacteristic* pRemoteCharacteristic,
                                       uint8_t* pData, size_t length, bool isNotify)
      {
        // Notification handle = 0x002e value: 0f 33 0a 00 00 0e 00 0e 00 0e 00 0e 00 0c 00 09 00 08 00 0b
          //                                     |  |  |     |     |     |     |     |     |     |     + Current hour - 16, 2 bytes for Wh
          //                                     |  |  |     |     |     |     |     |     |     + Current hour - 17, 2 bytes for Wh
          //                                     |  |  |     |     |     |     |     |     + Current hour - 18, 2 bytes for Wh
          //                                     |  |  |     |     |     |     |     + Current hour - 19, 2 bytes for Wh
          //                                     |  |  |     |     |     |     + Current hour - 20, 2 bytes for Wh
          //                                     |  |  |     |     |     + Current hour - 21, 2 bytes for Wh
          //                                     |  |  |     |     + Current hout - 22, 3 bytes for Wh
          //                                     |  |  |     + Current hour - 23, 2 bytes for Wh
          //                                     |  |  + 0x0a00, Request data for day request
          //                                     |  + Length of payload starting w/ next byte incl. checksum
          //                                     + static start sequence for message, 0x0f

        // Notification handle = 0x002e value: 00 0e 00 0e 00 11 00 0f 00 10 00 0f 00 0d 00 0e 00 0e 00 0e
        // Notification handle = 0x002e value: 00 0e 00 0e 00 0e 00 0e 00 0d 00 00 42 ff ff
          //                                                             |     |     |  + static end sequence of message, 0xffff
          //                                                             |     |     + checksum byte starting with length-byte, ending w/ byte before
          //                                                             |     + Current hour, 2 bytes for Wh
          //                                                             + Current hour - 2 , 2 bytes for Wh    Serial.println("CALLBACK Daily");




          //                                        +- Length of payload starting w/ next byte. -+
          //                                        |                                            |
        // Notification handle = 0x0014 value: 0f 0f 04 00 01 00 88 50 dc 00 d6 32 01 00 00 00 00 67 2a
          //                                     |     |     |  |        |  |     |        |           |
          //                                     |     |     |  |        |  |     |        |           + checksum byte starting with length-byte
          //                                     |     |     |  |        |  |     |        + total consumption, 4 bytes (hardware versions >= 3 there is a value)
          //                                     |     |     |  |        |  |     + frequency (Hz)
          //                                     |     |     |  |        |  + Ampere/1000 (A), 2 bytes
          //                                     |     |     |  |        + Voltage (V)
          //                                     |     |     |  |
          //                                     |     |     |  + Watt/1000, 3 bytes
          //                                     |     |     + Power, 0 = off, 1 = on
          //                                     |     + Capture measurement response 0x0400
          //                                     + static start sequence for message, 0x0f
        if (pData[1] == 6)
          {
            Serial.println("AUTH DAILY NOTIFY");
          }
        if (pData[1] == 15)
          {
            Serial.println("Measure Notify");
            mode ="measure";
            poweron = pData[4];
            watt = float((pData[5] * 256 * 256) + (pData[6] * 256) + (pData[7])) / 1000; // byte 5 um 16 bit "verschieben" = 65536, byte 6 um 8 bit "verschieben" ) = 256, Byte 7 nich verschieben.... Damit werden die einzelnen Bytes zu einer Gesamtzahl -- dividiert durch 1000 ergibt die Watt (das ganze muss vor der division als float definiert werden, sonst wird die Summe als int gewertet)
            volt = pData[8];
            ampere = float((pData[9] * 256) + (pData[10])) / 1000; // byte 9 um 8 bit "verschieben" ) = 256, Byte 10 nich verschieben.... Damit werden die einzelnen Bytes zu einer Gesamtzahl -- dividiert durch 1000 ergibt die Ampere (das ganze muss vor der division als float definiert werden, sonst wird die Summe als int gewertet)
            frequency = pData[11]; // Frequenz scheint nur das erste Byte zu enthalten ohne verschieben
            consumption = float((pData[14] * 256 * 256 *256) + (pData[15] * 256 * 256) + (pData[16] * 256)  + pData[17]) / 1000;
            measure_has_data = true;
            Serial.print("Watt: ");
            printf("%.4lf\n",watt);

            Serial.print("volt: ");
            Serial.println(volt);

            Serial.print("ampere: ");
            Serial.println(ampere);

            Serial.print("frequency: ");
            Serial.println(frequency);

            Serial.print("consumption: ");
            printf("%.4lf\n",consumption);
          }
        if (pData[1] == 51 && pData[2] == 10)  // pData1 = 0x31 und pData2 = 0x0a
          {
            Serial.println("Daily Notify");
            mode ="daily";
            int byte = 4;
            kwh[24] = {0};
            for (int d = 0; d < 24; d++)
              {
                kwh[d] = float((pData[byte] * 256) + (pData[byte+1])) / 1000;
                byte += 2;
              }
            daily_has_data = true;
            /*int count = 23;
              for (int d = 0; d < 24; d++)
                {
                  Serial.print("-" );
                  Serial.print(count );
                  Serial.print("h: " );
                  printf("%.4lf\n",kwh[d]);
                }
             */
          }
        for (int i = 0; i < length; i++)
          {
            Serial.print(pData[i],HEX);
            Serial.print(" ");
          }
        Serial.println("");
      }

    void VoltCraft::ReadVoltCraft(std::string address, bool daily)
      {
    		ReadVoltCraft(*new NimBLEAddress(address,BLE_ADDR_PUBLIC),daily);
      }

    void VoltCraft::ReadVoltCraft(NimBLEAddress address, bool daily) {
      NimBLEClient* pClient = nullptr;
      if(NimBLEDevice::getClientListSize())
        {
          Serial.println(" - RE CONNECT ...");
          pClient = NimBLEDevice::getClientByPeerAddress(address);
          Serial.println(" - RE CONNECT  2 ...");
          if(pClient)
            {
                Serial.println(" - CLIENT VORHANDEN -- CONNECTING");
                if(!pClient->connect(address))
                  {
                    Serial.println("Reconnect 2 failed");
                    //delay(40000);
                  }
                else
                  {
                    Serial.println("Reconnected client");
                  }
            }
          else
            {
                Serial.println("no Client in Reconnect");
                //pClient = NimBLEDevice::getDisconnectedClient();
            }
        }
      if (!pClient)
        {
          pClient = NimBLEDevice::createClient();
          pClient->setClientCallbacks(&clientCBVolt, false);
          // Set initial connection parameters: These settings are 15ms interval, 0 latency, 120ms timout.
          // These settings are safe for 3 clients to connect reliably, can go faster if you have less
          // connections. Timeout should be a multiple of the interval, minimum is 100ms.
          // Min interval: 12 * 1.25ms = 15, Max interval: 12 * 1.25ms = 15, 0 latency, 51 * 10ms = 510ms timeout
          pClient->setConnectionParams(12,12,0,51);
          // Set how long we are willing to wait for the connection to complete (seconds), default is 30.
          pClient->setConnectTimeout(10);
          Serial.println(" - Connecting device...");
          if (pClient->connect(address))
            {
              Serial.println(" - Connected to server");
            }
          else
            {
              NimBLEDevice::deleteClient(pClient);
              Serial.println(" - Failed to connect, deleted client");
              volt = -1;
              return;
            }
        }
      if(!pClient->isConnected())
        {
          Serial.println(" - not connected");
          volt = -1;
          return;
        }
      Serial.println("Hole SERVICE");
      NimBLERemoteService* pRemoteService = pClient->getService(bmeVoltServiceUUID);
      if(pRemoteService)
        {     // make sure it's not null
          Serial.println("Service gefunden");
          NimBLERemoteCharacteristic* pRemoteCharacteristic = pRemoteService->getCharacteristic(bmeVoltNotifyCharacteristicsUUID);
          Serial.println("Char ermittelt?");
          if(pRemoteCharacteristic)
            {     // make sure it's not null
              Serial.println("Char nicht null");
              //  Subscribe parameter defaults are: notifications=true, notifyCallback=nullptr, response=false.
              //  Unsubscribe parameter defaults are: response=false.
              if(pRemoteCharacteristic->canNotify())
                {
                  Serial.println("CAN NOTIFY = OK");
                  bool subscribeSucess = false ;

                  Serial.println("VOLT SUBSCRIBE");
                  subscribeSucess = pRemoteCharacteristic->subscribe(true, NotifyCallbackVolt);
                  Serial.println("SUBSCRIBE erfolgreich?");
                  if(!subscribeSucess)
                    {
                      // Disconnect if subscribe failed
                      Serial.print("Fehler beim Subscribe");
                      pClient->disconnect();
                      volt = -1;
                      return;
                    }
                  else
                    {
                      Serial.print("SUBSCRIBED SUCCESSFULLY ");
                    }
                }
              else
                {
                  Serial.print("Kein CanNotify ");
                  volt = -1;
                }
            }
          else
            {
              Serial.println("Char nicht ermittelt = NULL");
            }
          Serial.println("Hole RemoteChar");
          pRemoteCharacteristic = pRemoteService->getCharacteristic(bmeVoltWriteCharacteristicsUUID);
          if(pRemoteCharacteristic)
            {     // make sure it's not null
              Serial.println("RemoteChar erfolgreich");
              if(pRemoteCharacteristic->canWriteNoResponse())
                {
                   Serial.println("CanWrite ist True");
                  //AUTH
                  //0f0c170000000000000000000018ffff
                    //| | |   | |     | |       | +  static end sequence of message, 0xffff
                    //| | |   | |     | |       + checksum byte starting with length-byte, ending w/ byte before
                    //| | |   | |     | + always 0x00000000
                    //| | |   | + PIN, 4 bytes e.g. 01020304
                    //| | |   + 0x00 for authorization request
                    //| | + Authorization command 0x1700
                    //| + Length of payload starting w/ next byte incl. checksum
                    //+ static start sequence for message, 0x0f
                  byte reqAuth[16] = { 0x0f,0x0c,0x17,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0xff,0xff };
                  Serial.println("put regAuth Byte");
                  if(pRemoteCharacteristic->writeValue(reqAuth, 16, false))
                    {
                        Serial.print("Wrote new value to: ");
                        Serial.println(pRemoteCharacteristic->getUUID().toString().c_str());
                    }
                  else
                    {
                        // Disconnect if write failed
                        Serial.println("Error WriteValue");
                        pClient->disconnect();
                        volt = -1;
                        return;
                    }
                  byte reqMeasure[9];
                  if (daily)
                    {
                      //0f050b0000000cffff
                      //| | | |     | + static end sequence of message, 0xffff
                      //| | | |     + checksum byte starting with length-byte, ending w/ byte before
                      //| | | + Static 0x000000
                      //| | + 0a = last 24h per hour, 0b = last 30 days per day, 0c = last year per month
                      //| + Length of payload starting w/ next byte incl. checksum
                      //+ static start sequence for message, 0x0f
                      Serial.println("set Daily Byte?");

                      reqMeasure[0] = 0x0f;
                      reqMeasure[1] = 0x05;
                      reqMeasure[2] = 0x0a;
                      reqMeasure[3] = 0x00;
                      reqMeasure[4] = 0x00;
                      reqMeasure[5] = 0x00;
                      reqMeasure[6] = 0x0b;
                      reqMeasure[7] = 0xff;
                      reqMeasure[8] = 0xff;
                      Serial.println("Daily Byte done");
                    }
                  else
                    {
                      //0f050400000005ffff
                      //| | |       | + static end sequence of message, 0xffff
                      //| | |       + checksum byte starting with length-byte, ending w/ byte before
                      //| | + Capture measurement command 0x040000
                      //| + Length of payload starting w/ next byte incl. checksum
                      //+ static start sequence for message, 0x0f
                      Serial.println("set measure Byte?");
                      reqMeasure[0] = 0x0f;
                      reqMeasure[1] = 0x05;
                      reqMeasure[2] = 0x04;
                      reqMeasure[3] = 0x00;
                      reqMeasure[4] = 0x00;
                      reqMeasure[5] = 0x00;
                      reqMeasure[6] = 0x05;
                      reqMeasure[7] = 0xff;
                      reqMeasure[8] = 0xff;
                      //reqMeasure = { 0x0f,0x05,0x04,0x00,0x00,0x00,0x05,0xff,0xff };
                      Serial.println("measure Byte done");
                    }
                  if(pRemoteCharacteristic->writeValue(reqMeasure, 9, false))
                    {
                      Serial.print("Wrote new value to: ");
                      Serial.println(pRemoteCharacteristic->getUUID().toString().c_str());
                    }
                  else
                    {
                      // Disconnect if write failed
                      Serial.println("TRENNE canwrite 2");
                      pClient->disconnect();
                      volt = -1;
                      return;
                    }
                }
              else
                {
                  Serial.println("no CANWRITE");
                }
            }
        }
      else
        {
          Serial.println("Remoteservice not found.");
          volt = -1;
        }
    	return;
    }

  #endif

