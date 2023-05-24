//#include "BluettiConfig.h"
#include "BTooth.h"
//#include "md_util.h"
//#include "PayloadParser.h"
//#include "BWifi.h"

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteWriteCharacteristic;
static BLERemoteCharacteristic* pRemoteNotifyCharacteristic;
static BLEAdvertisedDevice*     pbluettiDevice;

// The remote Bluetti service we wish to connect to.
static BLEUUID serviceUUID("0000ff00-0000-1000-8000-00805f9b34fb");

// The characteristics of Bluetti Devices
static BLEUUID    WRITE_UUID("0000ff02-0000-1000-8000-00805f9b34fb");
static BLEUUID    NOTIFY_UUID("0000ff01-0000-1000-8000-00805f9b34fb");

int publishErrorCount = 0;
unsigned long lastMQTTMessage = 0;
unsigned long previousDeviceStatePublish = 0;

int pollTick = 0;
ESPBluettiSettings _settings;

// initialize bluetti device data
  // { FIELD_NAME, PAGE, OFFSET, SIZE, SCALE (if scale is needed e.g. decimal value, defaults to 0) , ENUM (if data is enum, defaults to 0) , FIELD_TYPE }
  static device_field_data_t bluetti_device_state[] =
    {
      // Page 0x00 Core
        // FIELD_NAME,          PVALUE,PAGE,  OFFS,  SIZ, SCAL (if scale is needed e.g. decimal value, defaults to 0)
        //                                                   ENUM (if data is enum, defaults to 0)
      // INFO device offset sort                                               FIELD_TYPE
        {DEVICE_TYPE,           NULL,  0x00,  0x0A,  7,   0, 0, STRING_FIELD},
        {SERIAL_NUMBER,         NULL,  0x00,  0x11,  4,   0 ,0, SN_FIELD},
        {ARM_VERSION,           NULL,  0x00,  0x17,  2,   0, 0, VERSION_FIELD},
        {DSP_VERSION,           NULL,  0x00,  0x19,  2,   0, 0, VERSION_FIELD},
        {DC_INPUT_POWER,        NULL,  0x00,  0x24,  1,   0, 0, UINT_FIELD},
        {AC_INPUT_POWER,        NULL,  0x00,  0x25,  1,   0, 0, UINT_FIELD},
        {AC_OUTPUT_POWER,       NULL,  0x00,  0x26,  1,   0, 0, UINT_FIELD},
        {DC_OUTPUT_POWER,       NULL,  0x00,  0x27,  1,   0, 0, UINT_FIELD},
        {POWER_GENERATION,      NULL,  0x00,  0x29,  1,   1, 0, DECIMAL_FIELD},
        {TOTAL_BATT_PERC,       NULL,  0x00,  0x2B,  1,   0, 0, UINT_FIELD},
        {AC_OUTPUT_ON,          NULL,  0x00,  0x30,  1,   0, 0, BOOL_FIELD},
        {DC_OUTPUT_ON,          NULL,  0x00,  0x31,  1,   0, 0, BOOL_FIELD},
      // INFO internal
        {AC_OUTPUT_MODE,        NULL,  0x00,  0x46,  1,   0, 0, UINT_FIELD},
        {INTERN_AC_VOLT,        NULL,  0x00,  0x47,  1,   1, 0, DECIMAL_FIELD},
        {INTERN_CURR_1,         NULL,  0x00,  0x48,  1,   1, 0, DECIMAL_FIELD},
        {INTERN_POWER_1,        NULL,  0x00,  0x49,  1,   0, 0, UINT_FIELD},
        {INTERN_AC_FREQ,        NULL,  0x00,  0x4A,  1,   2, 0, DECIMAL_FIELD},
        {INTERN_CURR_2,         NULL,  0x00,  0x4B,  1,   1, 0, DECIMAL_FIELD},
        {INTERN_POWER_2,        NULL,  0x00,  0x4C,  1,   0, 0, UINT_FIELD},
        {AC_INPUT_VOLT,         NULL,  0x00,  0x4D,  1,   1, 0, DECIMAL_FIELD},
        {INTERN_CURR_3,         NULL,  0x00,  0x4E,  1,   1, 0, DECIMAL_FIELD},
        {INTERN_POWER_3,        NULL,  0x00,  0x4F,  1,   0, 0, UINT_FIELD},
        {AC_INPUT_FREQ,         NULL,  0x00,  0x50,  1,   2, 0, DECIMAL_FIELD},
        {INT_DC_INP_VOLT,       NULL,  0x00,  0x56,  1,   1, 0, DECIMAL_FIELD},
        {INT_DC_INP_POW,        NULL,  0x00,  0x57,  1,   0, 0, UINT_FIELD},
        {INT_DC_INP_CURR,       NULL,  0x00,  0x58,  1,   1, 0, DECIMAL_FIELD},
        {PACK_NUM_MAX,          NULL,  0x00,  0x5B,  1,   0, 0, UINT_FIELD },
        {TOTAL_BATT_VOLT,       NULL,  0x00,  0x5C,  1,   1, 0, DECIMAL_FIELD},
        {TOTAL_BATT_CURR,       NULL,  0x00,  0x5D,  1,   1, 0, DECIMAL_FIELD},
        {PACK_NUM,              NULL,  0x00,  0x60,  1,   0, 0, UINT_FIELD},
        {PACK_STATUS,           NULL,  0x00,  0x61,  1,   1, 0, UINT_FIELD},
        {PACK_VOLTAGE,          NULL,  0x00,  0x62,  1,   2 ,0 ,DECIMAL_FIELD},
        {PACK_BATT_PERC,        NULL,  0x00,  0x63,  1,   0, 0, UINT_FIELD},
        //{CELL_VOTAGES,          NULL,  0x00,  0x48,  1,   1, 0, },
        {PACK_BMS_VERSION,      NULL,  0x00,  0xC9,  1,   0, 0, UINT_FIELD},
      // CONTROL elements
        {UPS_MODE,              NULL,  0x00,  0xB9,  1,   0, 0, UINT_FIELD},
        {SPLIT_PHASE_ON,        NULL,  0x00,  0xBC,  1,   0, 0, UINT_FIELD},
        {SPLIT_PH_MACH_MODE,    NULL,  0x00,  0xBD,  1,   0, 0, UINT_FIELD},
        {SET_PACK_NUM,          NULL,  0x00,  0xBE,  1,   0, 0, UINT_FIELD},
        {SET_AC_OUT_ON,         NULL,  0x0B,  0xBF,  1,   0, 0, BOOL_FIELD},
        {SET_DC_OUT_ON,         NULL,  0x0B,  0xC0,  1,   0, 0, BOOL_FIELD},
        {GRID_CHANGE_ON,        NULL,  0x00,  0xC3,  1,   0, 0, UINT_FIELD},
        {TIME_CTRL_ON,          NULL,  0x00,  0xC5,  1,   0, 0, UINT_FIELD},
        {BATT_RANGE_START,      NULL,  0x00,  0xC7,  1,   0, 0, UINT_FIELD},
        {BATT_RANGE_END,        NULL,  0x00,  0xC8,  1,   0, 0, UINT_FIELD},
        {BLUETOOTH_CONN,        NULL,  0x00,  0xDD,  1,   0, 0, UINT_FIELD},
        {AUTO_SLEEP_MODE,       NULL,  0x00,  0xF5,  1,   0, 0, UINT_FIELD},
        {LED_CONTROL,           NULL,  0x00,  0xDA,  1,   0, 0, UINT_FIELD},
        {FIELD_UNDEFINED,       NULL,  0x00,  0xDF,  1,   0, 0, UINT_FIELD},
    };

  static device_field_data_t bluetti_device_command[] =
      {
        /*Page 0x00 Core */
        {AC_OUTPUT_ON,            NULL,  0x0B, 0xBF, 1, 0, 0, BOOL_FIELD},
        {DC_OUTPUT_ON,            NULL,  0x0B, 0xC0, 1, 0, 0, BOOL_FIELD}
      };
  static device_field_data_t bluetti_polling_command[] =
    {
      {FIELD_UNDEFINED,         NULL,  0x00, 0x0A, 0x07 ,0 , 0, TYPE_UNDEFINED},
      //{FIELD_UNDEFINED,         NULL,  0x00, 0x32, 0x14 ,0 , 0, TYPE_UNDEFINED},
      //{FIELD_UNDEFINED,         NULL,  0x00, 0x46, 0x2C ,0 , 0, TYPE_UNDEFINED},
      //{FIELD_UNDEFINED,         NULL,  0x00, 0xC9, 0x01 ,0 , 0, TYPE_UNDEFINED},
      //{FIELD_UNDEFINED,         NULL,  0x00, 0xB4, 0x3C ,0 , 0, TYPE_UNDEFINED},
      //{FIELD_UNDEFINED,         NULL,  0x0B, 0x00, 0x2F ,0 , 0, TYPE_UNDEFINED},
      //{FIELD_UNDEFINED,         NULL,  0x0B, 0xB9, 0x26 ,0 , 0, TYPE_UNDEFINED},
      //{FIELD_UNDEFINED,         NULL,  0x0B, 0xD7, 0x31 ,0 , 0, TYPE_UNDEFINED}
    };

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
        #ifdef RELAISMODE
          #ifdef DEBUG
            Serial.println(F("deactivate relais contact"));
          #endif
          digitalWrite(RELAIS_PIN, RELAIS_LOW);
        #endif
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
    init_dev_fields(&bluetti_device_state[0], &bluetti_device_command[0], &bluetti_polling_command[0]);
    return &bluetti_device_state[0];
  }

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
        S2VAL("notifyCallback F01 - Write Response ", length, "Bytes");
        /* pData Debug... */
        serHEXdump(pData, length);
          Serial.println();
      #endif

    bt_command_t command_handle;
    if(xQueueReceive(commandHandleQueue, &command_handle, 500))
      {
        parse_bluetooth_data(NULL, command_handle.page, command_handle.offset, pData, length);
      }
  }

bool connectToServer()
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

void handleBTCommandQueue()
  {
    bt_command_t command;
    if(xQueueReceive(sendQueue, &command, 0))
      {
        #ifdef DEBUG
            STXT("handleBTComm Write Request FF02 - Value: ");
            serHEXdump((uint8_t*)&command, 8);
              //for(int i=0; i<8; i++)
              //  {
              //     if ( i % 2 == 0){ Serial.print(" "); };
              //     Serial.printf("%02x", ((uint8_t*)&command)[i]);
              //  }
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
  }

void pollBluetti()
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
  }

bool isBTconnected()
  {
    return connected;
  }

unsigned long getLastBTMessageTime()
  {
      return lastBTMessage;
  }


