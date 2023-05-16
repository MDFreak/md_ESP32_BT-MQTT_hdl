//#include "BluettiConfig.h"
#include "BTooth.h"
#include "utils.h"
#include "PayloadParser.h"
//#include "BWifi.h"

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteWriteCharacteristic;
static BLERemoteCharacteristic* pRemoteNotifyCharacteristic;
static BLEAdvertisedDevice*     bluettiDevice;

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

struct command_handle
  {
    uint8_t page;
    uint8_t offset;
    int length;
  };

QueueHandle_t commandHandleQueue;
QueueHandle_t sendQueue;
unsigned long lastBTMessage = 0;

String map_field_name(enum field_names f_name)
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
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class BluettiAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
  {
    /**
     * Called for each advertising BLE server.
     */
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
            bluettiDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;
          }
      }
  };
BluettiAdvertisedDeviceCallbacks  BluettiAdvDevCallbacks  =  BluettiAdvertisedDeviceCallbacks();
BluettiAdvertisedDeviceCallbacks* pBluettiAdvDevCallbacks = &BluettiAdvDevCallbacks;

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
        for (int i=1; i<=length; i++)
          {
            Serial.printf("%02x", pData[i-1]);
            if(i % 2 == 0)
              {
                Serial.print(" ");
              }
            if(i % 16 == 0)
              {
                Serial.println();
              }
          }
        Serial.println();
      #endif

    bt_command_t command_handle;
    if(xQueueReceive(commandHandleQueue, &command_handle, 500))
      {
        pase_bluetooth_data(command_handle.page, command_handle.offset, pData, length);
      }
  }

bool connectToServer()
  {
            Serial.print(F("Forming a connection to "));
            Serial.println(bluettiDevice->getAddress().toString().c_str());
    BLEClient*  pClient  = BLEDevice::createClient();
            Serial.println(F(" - Created client"));
    //pClient->setClientCallbacks(new MyClientCallback());
    pClient->setClientCallbacks(plocClientCallback);

    // Connect to the remove BLE Server.
    pClient->connect(bluettiDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(F(" - Connected to server"));
    pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print(F("Failed to find our service UUID: "));
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our service"));


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteWriteCharacteristic = pRemoteService->getCharacteristic(WRITE_UUID);
    if (pRemoteWriteCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(WRITE_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our Write characteristic"));

        // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteNotifyCharacteristic = pRemoteService->getCharacteristic(NOTIFY_UUID);
    if (pRemoteNotifyCharacteristic == nullptr) {
      Serial.print(F("Failed to find our characteristic UUID: "));
      Serial.println(NOTIFY_UUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(F(" - Found our Notifyite characteristic"));

    // Read the value of the characteristic.
    if(pRemoteWriteCharacteristic->canRead()) {
      std::string value = pRemoteWriteCharacteristic->readValue();
      Serial.print(F("The characteristic value was: "));
      Serial.println(value.c_str());
    }

    if(pRemoteNotifyCharacteristic->canNotify())
      pRemoteNotifyCharacteristic->registerForNotify(notifyCallback);

    connected = true;
     #ifdef RELAISMODE
      #ifdef DEBUG
        Serial.println(F("activate relais contact"));
      #endif
      digitalWrite(RELAIS_PIN, RELAIS_HIGH);
    #endif

    return true;
  }

void handleBTCommandQueue(){

    bt_command_t command;
    if(xQueueReceive(sendQueue, &command, 0)) {

#ifdef DEBUG
    Serial.print("Write Request FF02 - Value: ");

    for(int i=0; i<8; i++){
       if ( i % 2 == 0){ Serial.print(" "); };
       Serial.printf("%02x", ((uint8_t*)&command)[i]);
    }

    Serial.println("");
#endif
      pRemoteWriteCharacteristic->writeValue((uint8_t*)&command, sizeof(command),true);

     };
}

void sendBTCommand(bt_command_t command){
    bt_command_t cmd = command;
    xQueueSend(sendQueue, &cmd, 0);
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
        #ifdef SLEEP_TIME_ON_BT_NOT_AVAIL
            //esp_deep_sleep_start();
          #else
            ESP.restart();
          #endif
      }

    if (connected)
      {
        // poll for device state
        if ( millis() - lastBTMessage > BLUETOOTH_QUERY_MESSAGE_DELAY)
          {
            bt_command_t command;
            command.prefix = 0x01;
            command.field_update_cmd = 0x03;
            command.page = bluetti_polling_command[pollTick].f_page;
            command.offset = bluetti_polling_command[pollTick].f_offset;
            command.len = (uint16_t) bluetti_polling_command[pollTick].f_size << 8;
            command.check_sum = modbus_crc((uint8_t*)&command,6);
            S2VAL(" command  pollTick  page ", pollTick, bluetti_polling_command[pollTick].f_page);
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


