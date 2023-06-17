#include "BTooth.h"

// system declarations
  struct command_handle
    {
      uint8_t page;
      uint8_t offset;
      int length;
    };
  static boolean doConnect = false;
  static boolean connected = false;
  static boolean doScan = false;
  static QueueHandle_t commandHandleQueue;
  static QueueHandle_t sendQueue;
  static unsigned long lastBTMessage = 0;
// bluetti declarations
  #if (USE_BLUETTI > OFF)
      static BLEAdvertisedDevice*     bluettiDevice;
      static BLERemoteCharacteristic* pRemoteWriteCharacteristic;
      static BLERemoteCharacteristic* pRemoteNotifyCharacteristic;

      // The remote Bluetti service we wish to connect to.
        //static BLEUUID    BLU_SERVICE_UUID("0000ff00-0000-1000-8000-00805f9b34fb");
      static BLEUUID    BLU_SERVICE_UUID(BLUETTI_UUID_SERVICE);

      // The characteristics of Bluetti Devices
        //static BLEUUID    BLU_NOTIFY_UUID("0000ff01-0000-1000-8000-00805f9b34fb");
      static BLEUUID    BLU_NOTIFY_UUID(BLUETTI_UUID_NOTIFY);
        //static BLEUUID    BLU_WRITE_UUID("0000ff02-0000-1000-8000-00805f9b34fb");
      static BLEUUID    BLU_WRITE_UUID(BLUETTI_UUID_WRITE);

      int publishErrorCount = 0;
      unsigned long lastMQTTMessage = 0;
      unsigned long previousDeviceStatePublish = 0;

      int blu_pollTick = 0;
      ESPBluettiSettings bluetti_settings;
      // initialize bluetti device data
      // { FIELD_NAME, PAGE, OFFSET, SIZE, SCALE (if scale is needed e.g. decimal value, defaults to 0) , ENUM (if data is enum, defaults to 0) , FIELD_TYPE }
      static bluetti_dev_f_data_t bluetti_device_state[] =
        {
          // Page 0x00 Core
            // FIELD_NAME,          PVALUE,PAGE,  OFFS,  SIZ, SCAL (if scale is needed e.g. decimal value, defaults to 0)
            //                                            ENUM (if data is enum, defaults to 0)
            //                                               FIELD_TYPE
            {AC_OUTPUT_ON,          NULL,  0x00,  0x30,  1,   0, 0, BOOL_FIELD},
            {DC_OUTPUT_ON,          NULL,  0x00,  0x31,  1,   0, 0, BOOL_FIELD},
            {DC_OUTPUT_POWER,       NULL,  0x00,  0x27,  1,   0, 0, UINT_FIELD},
            {AC_OUTPUT_POWER,       NULL,  0x00,  0x26,  1,   0, 0, UINT_FIELD},
            {POWER_GENERATION,      NULL,  0x00,  0x29,  1,   1, 0, DECIMAL_FIELD},
            {TOTAL_BATTERY_PERCENT, NULL,  0x00,  0x2B,  1,   0, 0, UINT_FIELD},
            {DC_INPUT_POWER,        NULL,  0x00,  0x24,  1,   0, 0, UINT_FIELD},
            {AC_INPUT_POWER,        NULL,  0x00,  0x25,  1,   0, 0, UINT_FIELD},
            {PACK_VOLTAGE,          NULL,  0x00,  0x62,  1,   2 ,0 ,DECIMAL_FIELD},
            {SERIAL_NUMBER,         NULL,  0x00,  0x11,  4,   0 ,0, SN_FIELD},
            {ARM_VERSION,           NULL,  0x00,  0x17,  2,   0, 0, VERSION_FIELD},
            {DSP_VERSION,           NULL,  0x00,  0x19,  2,   0, 0, VERSION_FIELD},
            {DEVICE_TYPE,           NULL,  0x00,  0x0A,  7,   0, 0, STRING_FIELD},
            //Page 0x00 Details
            {INTERNAL_AC_VOLTAGE,   NULL,  0x00,  0x47,  1,   1, 0, DECIMAL_FIELD},
            {INTERNAL_CURRENT_ONE,  NULL,  0x00,  0x48,  1,   1, 0, DECIMAL_FIELD},
            //Page 0x00 Battery Details
            {PACK_NUM_MAX,          NULL,  0x00,  0x5B,  1,   0, 0, UINT_FIELD },
            //Page 0x00 Battery Data
        };
      static bluetti_dev_f_data_t bluetti_device_command[] =
        {
          /*Page 0x00 Core */
          {AC_OUTPUT_ON,            NULL,  0x0B, 0xBF, 1, 0, 0, BOOL_FIELD},
          {DC_OUTPUT_ON,            NULL,  0x0B, 0xC0, 1, 0, 0, BOOL_FIELD}
        };
      static bluetti_dev_f_data_t bluetti_polling_command[] =
        {
          {FIELD_UNDEFINED,         NULL,  0x00, 0x0A, 0x28 ,0 , 0, TYPE_UNDEFINED},
          {FIELD_UNDEFINED,         NULL,  0x00, 0x46, 0x15 ,0 , 0, TYPE_UNDEFINED},
          {FIELD_UNDEFINED,         NULL,  0x0B, 0xB9, 0x3D ,0 , 0, TYPE_UNDEFINED}
        };
    #endif

/*
String map_field_name(enum bluetti_field_index f_index)
  {
   switch(blu_f_name)
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
      case TOTAL_BATTERY_PERCENT:
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
      case INTERNAL_AC_VOLTAGE:
        return "internal_ac_voltage";
        break;
      case INTERNAL_CURRENT_ONE:
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
        SVAL("BLE Advertised Device found: ", advertisedDevice.toString().c_str());
          //Serial.print(F("BLE Advertised Device found: "));
          //Serial.println(advertisedDevice.toString().c_str());
          //ESPBluettiSettings bluetti_settings = get_esp32_bluetti_bluetti_settings();
          // We have found a device, let us now see if it contains the service we are looking for.
          //if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLU_SERVICE_UUID) && advertisedDevice.getName().compare(bluetti_settings.bluetti_device_id))
        if (   advertisedDevice.haveServiceUUID()
            && advertisedDevice.isAdvertisingService(BLU_SERVICE_UUID)
            && advertisedDevice.getName().compare(bluetti_settings.bluetti_device_id))
          {
            Serial.println(" onResult stop scan ");
            BLEDevice::getScan()->stop();
            bluettiDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
          }
      }
  };
BluettiAdvertisedDeviceCallbacks  BluettiAdvDevCallbacks  =  BluettiAdvertisedDeviceCallbacks();
BluettiAdvertisedDeviceCallbacks* pBluettiAdvDevCallbacks = &BluettiAdvDevCallbacks;

bluetti_dev_f_data_t* getpDevField()
  {
    S2HEXVAL(" getpDevField pBluetti", (uint32_t) &bluetti_device_state[0], (uint32_t) bluetti_device_state);
    init_blu_dev_fields(&bluetti_device_state[0], &bluetti_device_command[0], &bluetti_polling_command[0]);
    return &bluetti_device_state[0];
  }

// --- initializing
void initBluetooth()
  {
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    //pBLEScan->setAdvertisedDeviceCallbacks(new BluettiAdvertisedDeviceCallbacks());
    pBLEScan->setAdvertisedDeviceCallbacks(pBluettiAdvDevCallbacks);
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);

    commandHandleQueue = xQueueCreate( 5, sizeof(bt_command_t ) );
    sendQueue = xQueueCreate( 5, sizeof(bt_command_t) );
  }

static void notifyCallback( BLERemoteCharacteristic* pBLERemoteCharacteristic,
                            uint8_t* pData, size_t length,   bool isNotify)
  {
    #ifdef DEBUG
        Serial.println("F01 - Write Response");
        /* pData Debug... */
        serHEXdump(pData, length);
          Serial.println();
      #endif

    bt_command_t command_handle;
    if(xQueueReceive(commandHandleQueue, &command_handle, 500))
      {
        parse_blu_bt_data(NULL, command_handle.page, command_handle.offset, pData, length);
      }
  }

bool connectToServer()
  {
    // create client
        SVAL("Forming a connection to ", bluettiDevice->getAddress().toString().c_str());
      BLEClient*  pClient  = BLEDevice::createClient();
        STXT(" - Created client");
      pClient->setClientCallbacks(plocClientCallback);
    // Connect to the remove BLE Server.
    pClient->connect(bluettiDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
        STXT(" - Connected to server");
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(BLU_SERVICE_UUID);
    if (pRemoteService == nullptr)
      {
        SVAL("Failed to find our service UUID: ", BLU_SERVICE_UUID.toString().c_str());
        pClient->disconnect();
        return false;
      }
    STXT(" - Found our service");
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteWriteCharacteristic = pRemoteService->getCharacteristic(BLU_WRITE_UUID);
    if (pRemoteWriteCharacteristic == nullptr)
      {
        SVAL("Failed to find our characteristic UUID: ", BLU_WRITE_UUID.toString().c_str());
        pClient->disconnect();
        return false;
      }
    STXT(" - Found our Write characteristic");
    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteNotifyCharacteristic = pRemoteService->getCharacteristic(BLU_NOTIFY_UUID);
    if (pRemoteNotifyCharacteristic == nullptr)
      {
        SVAL("Failed to find our characteristic UUID: ", BLU_NOTIFY_UUID.toString().c_str());
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

void handleBTCommandQueue()
  {
    bt_command_t command;
    if(xQueueReceive(sendQueue, &command, 0))
      {
        #ifdef DEBUG
            Serial.print("Write Request FF02 - Value: ");
            for(int i=0; i<8; i++)
              {
                 if ( i % 2 == 0){ Serial.print(" "); };
                 Serial.printf("%02x", ((uint8_t*)&command)[i]);
              }
            Serial.println("");
          #endif
        pRemoteWriteCharacteristic->writeValue((uint8_t*)&command, sizeof(command),true);
     };
  }

void sendBTCommand(bt_command_t command)
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

void handleBluetooth()
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
    if ((millis() - lastBTMessage) > (MAX_DISCONN_TIME_TO_REBOOT * 60000))
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
        // poll for device state
        if ( millis() - lastBTMessage > BT_QUERY_MSG_DELAY)
          {
            bt_command_t command;
            // build command[index: polltick]
              command.prefix = 0x01;
              command.field_update_cmd = 0x03;
              command.page = bluetti_polling_command[blu_pollTick].f_page;
              command.offset = bluetti_polling_command[blu_pollTick].f_offset;
              command.len = (uint16_t) bluetti_polling_command[blu_pollTick].f_size << 8;
              command.check_sum = bt_modbus_crc((uint8_t*) &command,6);
            S2VAL(" command  blu_pollTick  page ", blu_pollTick, bluetti_polling_command[blu_pollTick].f_page);
            xQueueSend(commandHandleQueue, &command, portMAX_DELAY);
            xQueueSend(sendQueue, &command, portMAX_DELAY);

            if (blu_pollTick == sizeof(bluetti_polling_command)/sizeof(bluetti_dev_f_data_t)-1 )
              {
                blu_pollTick = 0;
              }
            else
              {
                blu_pollTick++;
              }
            lastBTMessage = millis();
          }

        handleBTCommandQueue();

      }
    else if(doScan)
      {
        BLEDevice::getScan()->start(0);
      }
  }

bool isBTconnected()
  {
    return connected;
  }

unsigned long getLastBTMessageTime()
  {
      return lastBTMessage;
  }


