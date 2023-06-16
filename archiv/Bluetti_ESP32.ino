//#include "BWifi.h"
#include "BTooth.h"
//#include "mqttblu.h"
//#include "config.h"

unsigned long lastbluTime1 = 0;
unsigned long blutimerDelay1 = 3000;

static uint32_t freeHeap    = 10000000;
static int32_t  tmpval32;

// --- heap output
  void heapFree(const char* text)
    {
      uint32_t tmp32 = ESP.getFreeHeap();
      //uint32_t tmp32 = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_32BIT);
      SVAL(text, tmp32);
    }

void setup()
  {
    Serial.begin(115200);
    #ifdef RELAISMODE
        pinMode(RELAIS_PIN, OUTPUT);
        #ifdef DEBUG
            Serial.println(F("deactivate relais contact"));
          #endif
        digitalWrite(RELAIS_PIN, RELAIS_LOW);
      #endif
    #ifdef SLEEP_TIME_ON_BT_NOT_AVAIL
        //esp_sleep_enable_timer_wakeup(SLEEP_TIME_ON_BT_NOT_AVAIL * 60 * 1000000ULL);
      #endif
    //initBWifi(false);
    initBluetooth();
    //initmqttblu();
  }

void loop()
  {
      tmpval32 = ESP.getFreeHeap();
      //heapFree("+loop");
      if (tmpval32 < freeHeap)
        {
          freeHeap = tmpval32;
          heapFree(" loop ");
        }
    handleBluetooth();
    //handlemqttblu();
    //handleWebserver();
    usleep(5000);
  }
