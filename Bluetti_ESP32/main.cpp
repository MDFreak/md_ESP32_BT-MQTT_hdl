
#include <Arduino.h>
#include <md_defines.h>
#include <main.h>
//#include "MQTT.h"
//#include "config.h"

// ----------------------------------------------------------------
// --- declarations
// ----------------------------------------------------------------
  // ------ system -----------------------
    static uint64_t anzUsCycles = 0ul;
    static uint64_t usLast      = 0ul;
    static uint64_t usTmp       = 0ul;
    static uint64_t usPerCycle  = 0ul;
    static uint32_t freeHeap    = 10000000;
    static int32_t  tmpval32;
    static int16_t  tmpval16;
    static char     cmsg[MSG_MAXLEN+1] = "";
    static char     ctmp30[LOGINTXT_MAX_LEN];
    static String   tmpStr;
    static uint8_t  firstrun = true;
    static uint8_t  iret        = 0;
  // --- system cycles ---------------
    #ifdef USE_INPUT_CYCLE
        static msTimer inputT           = msTimer(INPUT_CYCLE_MS);
        static uint8_t inpIdx   = 0;
      #endif
    #ifdef USE_OUTPUT_CYCLE
        static msTimer outpT    = msTimer(OUTPUT_CYCLE_MS);
        static uint8_t outpIdx  = 0;
      #endif
  // --- memory
    #if (USE_FLASH_MEM > OFF)
        md_spiffs   spiffs;
        md_spiffs  *pspiffs = &spiffs;
      #endif
  // --- network ----------------------
    #if (USE_WIFI > OFF)
        md_wifi wifi  = md_wifi();
        msTimer wifiT = msTimer(WIFI_CONN_CYCLE);
        #if (USE_LOCAL_IP > OFF)
          #endif // USE_LOCAL_IP
        #if (USE_NTP_SERVER > OFF)
            msTimer ntpT    = msTimer(NTPSERVER_CYCLE);
            time_t  ntpTime = 0;
            //bool    ntpGet  = true;
            uint8_t ntpOk   = FALSE;
          #endif // USE_WIFI
      #endif
    #if (USE_MQTT > OFF)
        const char cerrMQTT[10][20]  =
          {
            "success",          "UnknownError",
            "TimedOut",         "AlreadyConnected",
            "BadParameter",     "BadProperties",
            "NetworkError",     "NotConnected",
            "TranscientPacket", "WaitingForResult"
          };
        const  String     mqttID       = MQTT_DEVICE;
        const  String     topDevice    = MQTT_TOPDEV;
        static char       cMQTT[20]    = "";
        static String     tmpMQTT      = "";
        static MQTTmsg_t  MQTTmsgs[MQTT_MSG_MAXANZ];
        static MQTTmsg_t* pMQTTWr      = &MQTTmsgs[0];
        static MQTTmsg_t* pMQTTRd      = &MQTTmsgs[0];
        static uint8_t    anzMQTTmsg   = 0;
        static uint8_t    pubMQTT      = TRUE;
        static int8_t     errMQTT      = 0;
        struct MessageReceiver : public Network::Client::MessageReceived
          {
            void messageReceived(const Network::Client::MQTTv5::DynamicStringView & topic,
                                 const Network::Client::MQTTv5::DynamicBinDataView & payload,
                                 const uint16 packetIdentifier,
                                 const Network::Client::MQTTv5::PropertiesView & properties)
              {
                fprintf(stdout, "  Topic: %.*s ", topic.length, topic.data);
                fprintf(stdout, "  Payload: %.*s\n", payload.length, payload.data);
                if (anzMQTTmsg < (MQTT_MSG_MAXANZ - 1))
                  {
                    sprintf(pMQTTWr->topic,   "%.*s", topic.length,   topic.data);
                    sprintf(pMQTTWr->payload, "%.*s", payload.length, payload.data);
                    pMQTTWr->topic[topic.length] = 0;
                    pMQTTWr->payload[payload.length] = 0;
                    anzMQTTmsg++;
                        S3VAL(" topic payload count", pMQTTWr->topic, pMQTTWr->payload, anzMQTTmsg);
                    pMQTTWr = (MQTTmsg_t*) pMQTTWr->pNext;
                  }
                //fprintf(stdout, "Msg received: (%04X)\n", packetIdentifier);
                //readMQTTmsg(topic, payload);
              }
          };
        MessageReceiver msgHdl;
        Network::Client::MQTTv5 mqtt(mqttID.c_str(), &msgHdl);
        uint8_t errMQTTold = MD_OK;
      #endif
  // --- Bluetti
    device_field_data_t* pbluetti_dev_state = NULL;
    // --- value storage
      // INFO device offset sort
        static String    BLUDEVICE_TYPE           = "---";
        static String    BLUSERIAL_NUMBER         = "---";
        static float     BLUARM_VERSION           = 100.0;
        static float     BLUDSP_VERSION           = 100.0;
        static uint16_t  BLUDC_INPUT_POWER        = 100;
        static uint16_t  BLUAC_INPUT_POWER        = 100;
        static uint16_t  BLUAC_OUTPUT_POWER       = 100;
        static uint16_t  BLUDC_OUTPUT_POWER       = 100;
        static float     BLUPOWER_GENERATION      = 100;
        static uint16_t  BLUTOTAL_BATT_PERC       = 100;
        static bool      BLUAC_OUTPUT_ON          = 100;
        static bool      BLUDC_OUTPUT_ON          = 100;
      // INFO internal
        static uint16_t  BLUAC_OUTPUT_MODE        = 100;
        static float     BLUINTERN_AC_VOLT        = 100.0;
        static float     BLUINTERN_CURR_1         = 100.0;
        static uint16_t  BLUINTERN_POWER_1        = 100;
        static float     BLUINTERN_AC_FREQ        = 100.0;
        static float     BLUINTERN_CURR_2         = 100.0;
        static uint16_t  BLUINTERN_POWER_2        = 100;
        static float     BLUAC_INPUT_VOLT         = 100.0;
        static float     BLUINTERN_CURR_3         = 100.0;
        static uint16_t  BLUINTERN_POWER_3        = 100;
        static float     BLUAC_INPUT_FREQ         = 100.0;
        static float     BLUINT_DC_INP_VOLT       = 100.0;
        static uint16_t  BLUINT_DC_INP_POW        = 100;
        static float     BLUINT_DC_INP_CURR       = 100.0;
        static uint16_t  BLUPACK_NUM_MAX          = 100;
        static float     BLUTOTAL_BATT_VOLT       = 100.0;
        static float     BLUTOTAL_BATT_CURR       = 100.0;
        static uint16_t  BLUPACK_NUM              = 100;
        static uint16_t  BLUPACK_STATUS           = 100;
        static float     BLUPACK_VOLTAGE          = 100.0;
        static uint16_t  BLUPACK_BATT_PERC        = 100;
        //static uint16_t  BLUCELL_VOTAGES        = 100;
        static uint16_t  BLUPACK_BMS_VERSION      = 100;
      // CONTROL elements
        static uint16_t  BLUUPS_MODE              = 100;
        static uint16_t  BLUSPLIT_PHASE_ON        = 100;
        static uint16_t  BLUSPLIT_PH_MACH_MODE    = 100;
        static uint16_t  BLUSET_PACK_NUM          = 100;
        static uint16_t  BLUSET_AC_OUT_ON         = 100;
        static uint16_t  BLUSET_DC_OUT_ON         = 100;
        static bool      BLUGRID_CHANGE_ON        = 100;
        static uint16_t  BLUTIME_CTRL_ON          = 100;
        static uint16_t  BLUBATT_RANGE_START      = 100;
        static uint16_t  BLUBATT_RANGE_END        = 100;
        static uint16_t  BLUBLUETOOTH_CONN        = 100;
        static uint16_t  BLUAUTO_SLEEP_MODE       = 100;
        static uint16_t  BLULED_CONTROL           = 100;
        static uint16_t  BLUFIELD_UNDEFINED       = 100;
          /* --- old value
            // INFO device offset sort
              static String    BLUDEVICE_TYPEold       = "---";
              static String    BLUSERIAL_NUMBERold      = "---";
              static float     BLUARM_VERSIONold        = 1000.0;
              static float     BLUDSP_VERSIONold        = 1000.0;
              static uint16_t  BLUDC_INPUT_POWERold     = 1000;
              static uint16_t  BLUAC_INPUT_POWERold     = 1000;
              static uint16_t  BLUAC_OUTPUT_POWERold    = 1000;
              static uint16_t  BLUDC_OUTPUT_POWERold    = 1000;
              static float     BLUPOWER_GENERATIONold   = 1000.0;
              static uint16_t  BLUTOTAL_BATT_PERCold    = 1000;
              static bool      BLUAC_OUTPUT_ONold       = 100;
              static bool      BLUDC_OUTPUT_ONold       = 100;
            // INFO internal
              static uint16_t  BLUAC_OUTPUT_MODEold     = 1000;
              static float     BLUINTERN_AC_VOLTold     = 1000.0;
              static float     BLUINTERN_CURR_1old      = 1000.0;
              static uint16_t  BLUINTERN_POWER_1old     = 1000;
              static float     BLUINTERN_AC_FREQold     = 1000.0;
              static float     BLUINTERN_CURR_2old      = 1000.0;
              static uint16_t  BLUINTERN_POWER_2old     = 1000;
              static float     BLUAC_INPUT_VOLTold      = 1000.0;
              static float     BLUINTERN_CURR_3old      = 1000.0;
              static uint16_t  BLUINTERN_POWER_3old     = 1000;
              static float     BLUAC_INPUT_FREQold      = 1000.0;
              static float     BLUINT_DC_INP_VOLTold    = 1000.0;
              static uint16_t  BLUINT_DC_INP_POWold     = 1000;
              static float     BLUINT_DC_INP_CURRold    = 1000.0;
              static uint16_t  BLUPACK_NUM_MAXold       = 1000;
              static float     BLUTOTAL_BATT_VOLTold    = 1000.0;
              static float     BLUTOTAL_BATT_CURRold    = 1000.0;
              static uint16_t  BLUPACK_NUMold           = 1000;
              static uint16_t  BLUPACK_STATUSold        = 1000;
              static float     BLUPACK_VOLTAGEold       = 1000.0;
              static uint16_t  BLUPACK_BATT_PERCold     = 1000;
              //static uint16_t  BLUCELL_VOTAGESold     = 1000;
              static uint16_t  BLUPACK_BMS_VERSIONold   = 1000;
            // CONTROL elements
              static uint16_t  BLUUPS_MODEold           = 1000;
              static uint16_t  BLUSPLIT_PHASE_ONold     = 1000;
              static uint16_t  BLUSPLIT_PH_MACH_MODEold = 1000;
              static uint16_t  BLUSET_PACK_NUMold       = 1000;
              static uint16_t  BLUSET_AC_OUT_ONold      = 1000;
              static uint16_t  BLUSET_DC_OUT_ONold      = 1000;
              static bool      BLUGRID_CHANGE_ONold     = 100;
              static uint16_t  BLUTIME_CTRL_ONold       = 1000;
              static uint16_t  BLUBATT_RANGE_STARTold   = 1000;
              static uint16_t  BLUBATT_RANGE_ENDold     = 1000;
              static uint16_t  BLUBLUETOOTH_CONNold     = 1000;
              static uint16_t  BLUAUTO_SLEEP_MODEold    = 1000;
              static uint16_t  BLULED_CONTROLold        = 1000;
              static uint16_t  BLUFIELD_UNDEFINEDold    = 1000;
           */
    // --- MQTT value as string
      static String valBLU[FIELD_IDX_MAX];
        /*
          // INFO device offset sort
            static String    valBLUDEVICE_TYPE        = "";
            static String    valBLUSERIAL_NUMBER      = "";
            static String    valBLUARM_VERSION        = "";
            static String    valBLUDSP_VERSION        = "";
            static String    valBLUDC_INPUT_POWER     = "";
            static String    valBLUAC_INPUT_POWER     = "";
            static String    valBLUAC_OUTPUT_POWER    = "";
            static String    valBLUDC_OUTPUT_POWER    = "";
            static String    valBLUPOWER_GENERATION   = "";
            static String    valBLUTOTAL_BATT_PERC    = "";
            static String    valBLUAC_OUTPUT_ON       = "";
            static String    valBLUDC_OUTPUT_ON       = "";
          // INFO internal
            static String    valBLUAC_OUTPUT_MODE     = "";
            static String    valBLUINTERN_AC_VOLT     = "";
            static String    valBLUINTERN_CURR_1      = "";
            static String    valBLUINTERN_POWER_1     = "";
            static String    valBLUINTERN_AC_FREQ     = "";
            static String    valBLUINTERN_CURR_2      = "";
            static String    valBLUINTERN_POWER_2     = "";
            static String    valBLUAC_INPUT_VOLT      = "";
            static String    valBLUINTERN_CURR_3      = "";
            static String    valBLUINTERN_POWER_3     = "";
            static String    valBLUAC_INPUT_FREQ      = "";
            static String    valBLUINT_DC_INP_VOLT    = "";
            static String    valBLUINT_DC_INP_POW     = "";
            static String    valBLUINT_DC_INP_CURR    = "";
            static String    valBLUPACK_NUM_MAX       = "";
            static String    valBLUTOTAL_BATT_VOLT    = "";
            static String    valBLUTOTAL_BATT_CURR    = "";
            static String    valBLUPACK_NUM           = "";
            static String    valBLUPACK_STATUS        = "";
            static String    valBLUPACK_VOLTAGE       = "";
            static String    valBLUPACK_BATT_PERC     = "";
            //static String  valBLUCELL_VOTAGES       = "";
            static String    valBLUPACK_BMS_VERSION   = "";
          // CONTROL elements
            static String    valBLUUPS_MODE           = "";
            static String    valBLUSPLIT_PHASE_ON     = "";
            static String    valBLUSPLIT_PH_MACH_MODE = "";
            static String    valBLUSET_PACK_NUM       = "";
            static String    valBLUSET_AC_OUT_ON      = "";
            static String    valBLUSET_DC_OUT_ON      = "";
            static String    valBLUGRID_CHANGE_ON     = "";
            static String    valBLUTIME_CTRL_ON       = "";
            static String    valBLUBATT_RANGE_START   = "";
            static String    valBLUBATT_RANGE_END     = "";
            static String    valBLUBLUETOOTH_CONN      = "";
            static String    valBLUAUTO_SLEEP_MODE    = "";
            static String    valBLULED_CONTROL        = "";
            static String    valBLUFIELD_UNDEFINED    = "";
        */
    // --- MQTT publish flag
      static int8_t pubBLU[FIELD_IDX_MAX];
        /*
          // INFO device offset sort
            static int8_t    pubBLUDEVICE_TYPE        = 0;
            static int8_t    pubBLUSERIAL_NUMBER      = 0;
            static int8_t    pubBLUARM_VERSION        = 0;
            static int8_t    pubBLUDSP_VERSION        = 0;
            static int8_t    pubBLUDC_INPUT_POWER     = 0;
            static int8_t    pubBLUAC_INPUT_POWER     = 0;
            static int8_t    pubBLUAC_OUTPUT_POWER    = 0;
            static int8_t    pubBLUDC_OUTPUT_POWER    = 0;
            static int8_t    pubBLUPOWER_GENERATION   = 0;
            static int8_t    pubBLUTOTAL_BATT_PERC    = 0;
            static int8_t    pubBLUAC_OUTPUT_ON       = 0;
            static int8_t    pubBLUDC_OUTPUT_ON       = 0;
          // INFO internal
            static int8_t    pubBLUAC_OUTPUT_MODE     = 0;
            static int8_t    pubBLUINTERN_AC_VOLT     = 0;
            static int8_t    pubBLUINTERN_CURR_1      = 0;
            static int8_t    pubBLUINTERN_POWER_1     = 0;
            static int8_t    pubBLUINTERN_AC_FREQ     = 0;
            static int8_t    pubBLUINTERN_CURR_2      = 0;
            static int8_t    pubBLUINTERN_POWER_2     = 0;
            static int8_t    pubBLUAC_INPUT_VOLT      = 0;
            static int8_t    pubBLUINTERN_CURR_3      = 0;
            static int8_t    pubBLUINTERN_POWER_3     = 0;
            static int8_t    pubBLUAC_INPUT_FREQ      = 0;
            static int8_t    pubBLUINT_DC_INP_VOLT    = 0;
            static int8_t    pubBLUINT_DC_INP_POW     = 0;
            static int8_t    pubBLUINT_DC_INP_CURR    = 0;
            static int8_t    pubBLUPACK_NUM_MAX       = 0;
            static int8_t    pubBLUTOTAL_BATT_VOLT    = 0;
            static int8_t    pubBLUTOTAL_BATT_CURR    = 0;
            static int8_t    pubBLUPACK_NUM           = 0;
            static int8_t    pubBLUPACK_STATUS        = 0;
            static int8_t    pubBLUPACK_VOLTAGE       = 0;
            static int8_t    pubBLUPACK_BATT_PERC     = 0;
            //static int8_t  pubBLUCELL_VOTAGES       = 0;
            static int8_t    pubBLUPACK_BMS_VERSION     = 0;
          // CONTROL elements
            static int8_t    pubBLUUPS_MODE           = 0;
            static int8_t    pubBLUSPLIT_PHASE_ON     = 0;
            static int8_t    pubBLUSPLIT_PH_MACH_MODE = 0;
            static int8_t    pubBLUSET_PACK_NUM       = 0;
            static int8_t    pubBLUSET_AC_OUT_ON      = 0;
            static int8_t    pubBLUSET_DC_OUT_ON      = 0;
            static int8_t    pubBLUGRID_CHANGE_ON     = 0;
            static int8_t    pubBLUTIME_CTRL_ON       = 0;
            static int8_t    pubBLUBATT_RANGE_START   = 0;
            static int8_t    pubBLUBATT_RANGE_END     = 0;
            static int8_t    pubBLUBLUETOOTH_CONN     = 0;
            static int8_t    pubBLUAUTO_SLEEP_MODE    = 0;
            static int8_t    pubBLULED_CONTROL        = 0;
            static int8_t    pubBLUFIELD_UNDEFINED    = 0;
        */
      // MQTT topic
      static String topBLU[FIELD_IDX_MAX];
        /*
          // INFO device offset sort
            static String topBLUDEVICE_TYPE        = DEVICE_F_NAMES[DEVICE_TYPE];
            static String topBLUSERIAL_NUMBER      = DEVICE_F_NAMES[SERIAL_NUMBER];
            static String topBLUARM_VERSION        = DEVICE_F_NAMES[ARM_VERSION];
            static String topBLUDSP_VERSION        = DEVICE_F_NAMES[DSP_VERSION];
            static String topBLUDC_INPUT_POWER     = DEVICE_F_NAMES[DC_INPUT_POWER];
            static String topBLUAC_INPUT_POWER     = DEVICE_F_NAMES[AC_INPUT_POWER];
            static String topBLUAC_OUTPUT_POWER    = DEVICE_F_NAMES[AC_OUTPUT_POWER];
            static String topBLUDC_OUTPUT_POWER    = DEVICE_F_NAMES[DC_OUTPUT_POWER];
            static String topBLUPOWER_GENERATION   = DEVICE_F_NAMES[POWER_GENERATION];
            static String topBLUTOTAL_BATT_PERC    = DEVICE_F_NAMES[TOTAL_BATT_PERC];
            static String topBLUAC_OUTPUT_ON       = DEVICE_F_NAMES[AC_OUTPUT_ON];
            static String topBLUDC_OUTPUT_ON       = DEVICE_F_NAMES[DC_OUTPUT_ON];
          // INFO internal
            static String topBLUAC_OUTPUT_MODE     = DEVICE_F_NAMES[AC_OUTPUT_MODE];
            static String topBLUINTERN_AC_VOLT     = DEVICE_F_NAMES[INTERN_AC_VOLT];
            static String topBLUINTERN_CURR_1      = DEVICE_F_NAMES[INTERN_CURR_1];
            static String topBLUINTERN_POWER_1     = DEVICE_F_NAMES[INTERN_POWER_1];
            static String topBLUINTERN_AC_FREQ     = DEVICE_F_NAMES[INTERN_AC_FREQ];
            static String topBLUINTERN_CURR_2      = DEVICE_F_NAMES[INTERN_CURR_2];
            static String topBLUINTERN_POWER_2     = DEVICE_F_NAMES[INTERN_POWER_2];
            static String topBLUAC_INPUT_VOLT      = DEVICE_F_NAMES[AC_INPUT_VOLT];
            static String topBLUINTERN_CURR_3      = DEVICE_F_NAMES[INTERN_CURR_3];
            static String topBLUINTERN_POWER_3     = DEVICE_F_NAMES[INTERN_POWER_3];
            static String topBLUAC_INPUT_FREQ      = DEVICE_F_NAMES[AC_INPUT_FREQ];
            static String topBLUINT_DC_INP_VOLT    = DEVICE_F_NAMES[INT_DC_INP_VOLT];
            static String topBLUINT_DC_INP_POW     = DEVICE_F_NAMES[INT_DC_INP_POW];
            static String topBLUINT_DC_INP_CURR    = DEVICE_F_NAMES[INT_DC_INP_CURR];
            static String topBLUPACK_NUM_MAX       = DEVICE_F_NAMES[PACK_NUM_MAX];
            static String topBLUTOTAL_BATT_VOLT    = DEVICE_F_NAMES[TOTAL_BATT_VOLT];
            static String topBLUTOTAL_BATT_CURR    = DEVICE_F_NAMES[TOTAL_BATT_CURR];
            static String topBLUPACK_NUM           = DEVICE_F_NAMES[PACK_NUM];
            static String topBLUPACK_STATUS        = DEVICE_F_NAMES[PACK_STATUS];
            static String topBLUPACK_VOLTAGE       = DEVICE_F_NAMES[PACK_VOLTAGE];
            static String topBLUPACK_BATT_PERC     = DEVICE_F_NAMES[PACK_BATT_PERC];
            //static String topBLUCELL_VOTAGES     = DEVICE_F_NAMES[CELL_VOTAGES];
            static String topBLUPACK_BMS_VERSION   = DEVICE_F_NAMES[PACK_BMS_VERSION];
          // CONTROL elements
            static String topBLUUPS_MODE           = DEVICE_F_NAMES[UPS_MODE];
            static String topBLUSPLIT_PHASE_ON     = DEVICE_F_NAMES[SPLIT_PHASE_ON];
            static String topBLUSPLIT_PH_MACH_MODE = DEVICE_F_NAMES[SPLIT_PH_MACH_MODE];
            static String topBLUSET_PACK_NUM       = DEVICE_F_NAMES[SET_PACK_NUM];
            static String topBLUSET_AC_OUT_ON      = DEVICE_F_NAMES[SET_AC_OUT_ON];
            static String topBLUSET_DC_OUT_ON      = DEVICE_F_NAMES[SET_DC_OUT_ON];
            static String topBLUGRID_CHANGE_ON     = DEVICE_F_NAMES[GRID_CHANGE_ON];
            static String topBLUTIME_CTRL_ON       = DEVICE_F_NAMES[TIME_CTRL_ON];
            static String topBLUBATT_RANGE_START   = DEVICE_F_NAMES[BATT_RANGE_START];
            static String topBLUBATT_RANGE_END     = DEVICE_F_NAMES[BATT_RANGE_END];
            static String topBLUBLUETOOTH_CONN     = DEVICE_F_NAMES[BLUETOOTH_CONN];
            static String topBLUAUTO_SLEEP_MODE    = DEVICE_F_NAMES[AUTO_SLEEP_MODE];
            static String topBLULED_CONTROL        = DEVICE_F_NAMES[LED_CONTROL];
            static String topBLUFIELD_UNDEFINED    = DEVICE_F_NAMES[FIELD_UNDEFINED];
        */
    unsigned long lastTime1 = 0;
    unsigned long timerDelay1 = 3000;
    uint8_t       scanBluIdx  = 0;
// ----------------------------------------------------------------
// --- system setup -----------------------------------
// ----------------------------------------------------------------
  void setup()
    {
      // --- system
        // disable watchdog
          disableCore0WDT();
          disableLoopWDT();
        // start system
          Serial.begin(115200);
          usleep(3000); // power-up safety delay
          STXT(" setup start ...");
        // FLASH memory
          #if (USE_FLASH_MEM > OFF)
              testFlash();
            #endif
      // --- network
        // start WIFI
          #if (USE_WIFI > OFF)
              uint8_t rep = WIFI_ANZ_LOGIN;
              while(rep > 0)
                {
                      //STXT(" setup   Start WiFi ");
                  iret = startWIFI(true);
                  if (iret == MD_OK)
                      {
                        STXT("  WIFI connected");
                        break;
                      }
                    else
                      {
                        #if (WIFI_IS_DUTY > OFF)
                            STXT("WIFI error -> halted");
                        #else
                            rep--;
                            if (rep > 0)
                              { dispStatus("WIFI error ..."); }
                            else
                              { dispStatus("WIFI not connected"); }
                          #endif
                      }
                  //usleep(50000);
                }
            #endif // USE_WIFI
        // start bluetooth
          #if (USE_BTOOTH)
              SHEXVAL(" init bluetooth...", (uint32_t) pbluetti_dev_state);
              pbluetti_dev_state = getpDevField();
              initBluetooth();
              SHEXVAL(" ...init bluetooth", (uint32_t) pbluetti_dev_state);
            #endif
        // start MQTT
          #if (USE_MQTT > OFF)
              startMQTT();
            #endif
      #ifdef SLEEP_TIME_ON_BT_NOT_AVAIL
          //esp_sleep_enable_timer_wakeup(SLEEP_TIME_ON_BT_NOT_AVAIL * 60 * 1000000ULL);
        #endif
      heapFree(" ... end setup ");
    }

// ----------------------------------------------------------------
// --- system run = endless loop
// ----------------------------------------------------------------
  void loop()
    {
      // head of loop
        if (firstrun == true)
            {
              String taskMessage = "loop task running on core ";
              taskMessage = taskMessage + xPortGetCoreID();
              STXT(taskMessage);
              usLast = micros();
              //firstrun = false;
              anzUsCycles = 0;
            }
        //SOUT(" "); SOUT(millis());
        #if ( USE_DISP > 0 )
            outStr   = "";
          #endif
        tmpval32 = ESP.getFreeHeap();
        //heapFree("+loop");
        if (tmpval32 < freeHeap)
          {
            freeHeap = tmpval32;
            heapFree(" loop ");
          }
      // --- network ---
        #if (USE_NTP_SERVER > OFF)   // get time from NTP server
            if (ntpT.TOut() == true)
              {
                setTime(++ntpTime);
                if (WiFi.status() == WL_CONNECTED)
                  { // WiFi online
                    if (!ntpOk)
                      {
                        initNTPTime();
                        iret = TRUE;
                        if (iret)
                          {
                            iret = wifi.getNTPTime(&ntpTime);
                            if (iret == WIFI_OK)
                              {
                                setTime(ntpTime);
                                STXT(" NTP time syncronized");
                                ntpOk = TRUE;
                              }
                          }
                      }
                  }
                ntpT.startT();
              }
          #endif // USE_NTP_SERVER
      // --- direct input ---
        #if (USE_MQTT > OFF)
            readMQTTmsg();
          #endif
      // --- direct output ---
        #if (USE_BTOOTH)
            // handleBluetooth();
            if (scanBluIdx < 63)
              {
                SVAL(" scanBluetti scanIdx ", scanBluIdx);
                scanBluetti(scanBluIdx, (scanBluIdx % 4) * 0x40, 64);
                scanBluIdx++;
              }
          #endif
      // --- standard output cycle ---
        #ifdef USE_OUTPUT_CYCLE
            if (outpT.TOut())
              {
                outpT.startT();
                //SVAL(" outputcycle idx ", outpIdx);
                if (errMQTT == MD_OK)
                  {
                    if (pbluetti_dev_state[outpIdx].f_new > OFF) // new value
                      {
                        switch(outpIdx)
                          {
                            // INFO device offset sort
                              case DEVICE_TYPE:         valBLU[outpIdx] = BLUDEVICE_TYPE;       break;
                              case SERIAL_NUMBER:       valBLU[outpIdx] = BLUSERIAL_NUMBER;     break;
                              case ARM_VERSION:         valBLU[outpIdx] = BLUARM_VERSION;       break;
                              case DSP_VERSION:         valBLU[outpIdx] = BLUDSP_VERSION;       break;
                              case DC_INPUT_POWER:      valBLU[outpIdx] = BLUDC_INPUT_POWER;    break;
                              case AC_INPUT_POWER:      valBLU[outpIdx] = BLUAC_INPUT_POWER;    break;
                              case AC_OUTPUT_POWER:     valBLU[outpIdx] = BLUAC_OUTPUT_POWER;   break;
                              case DC_OUTPUT_POWER:     valBLU[outpIdx] = BLUDC_OUTPUT_POWER;   break;
                              case POWER_GENERATION:    valBLU[outpIdx] = BLUPOWER_GENERATION;  break;
                              case TOTAL_BATT_PERC:     valBLU[outpIdx] = BLUTOTAL_BATT_PERC;   break;
                              case AC_OUTPUT_ON:        valBLU[outpIdx] = BLUAC_OUTPUT_ON;      break;
                              case DC_OUTPUT_ON:        valBLU[outpIdx] = BLUDC_OUTPUT_ON;      break;
                            // INFO internal
                              case AC_OUTPUT_MODE:      valBLU[outpIdx] = BLUAC_OUTPUT_MODE;    break;
                              case INTERN_AC_VOLT:      valBLU[outpIdx] = BLUINTERN_AC_VOLT;    break;
                              case INTERN_CURR_1:       valBLU[outpIdx] = BLUINTERN_CURR_1;     break;
                              case INTERN_POWER_1:      valBLU[outpIdx] = BLUINTERN_POWER_1;    break;
                              case INTERN_AC_FREQ:      valBLU[outpIdx] = BLUINTERN_AC_FREQ;    break;
                              case INTERN_CURR_2:       valBLU[outpIdx] = BLUINTERN_CURR_2;     break;
                              case INTERN_POWER_2:      valBLU[outpIdx] = BLUINTERN_POWER_2;    break;
                              case AC_INPUT_VOLT:       valBLU[outpIdx] = BLUAC_INPUT_VOLT;     break;
                              case INTERN_CURR_3:       valBLU[outpIdx] = BLUINTERN_CURR_3;     break;
                              case INTERN_POWER_3:      valBLU[outpIdx] = BLUINTERN_POWER_3;    break;
                              case AC_INPUT_FREQ:       valBLU[outpIdx] = BLUAC_INPUT_FREQ;     break;
                              case INT_DC_INP_VOLT:     valBLU[outpIdx] = BLUINT_DC_INP_VOLT;   break;
                              case INT_DC_INP_POW:      valBLU[outpIdx] = BLUINT_DC_INP_POW;    break;
                              case INT_DC_INP_CURR:     valBLU[outpIdx] = BLUINT_DC_INP_CURR;   break;
                              case PACK_NUM_MAX:        valBLU[outpIdx] = BLUPACK_NUM_MAX;      break;
                              case TOTAL_BATT_VOLT:     valBLU[outpIdx] = BLUTOTAL_BATT_VOLT;   break;
                              case TOTAL_BATT_CURR:     valBLU[outpIdx] = BLUTOTAL_BATT_CURR;   break;
                              case PACK_NUM:            valBLU[outpIdx] = BLUPACK_NUM;          break;
                              case PACK_STATUS:         valBLU[outpIdx] = BLUPACK_STATUS;       break;
                              case PACK_VOLTAGE:        valBLU[outpIdx] = BLUPACK_VOLTAGE;      break;
                              case PACK_BATT_PERC:      valBLU[outpIdx] = BLUPACK_BATT_PERC;    break;
                              //case CELL_VOTAGES:      valBLU[outpIdx] = BLUCELL_VOTAGES; break;
                              case PACK_BMS_VERSION:    valBLU[outpIdx] = BLUPACK_BMS_VERSION;  break;
                            // CONTROL elements
                              case UPS_MODE:            valBLU[outpIdx] = BLUUPS_MODE;          break;
                              case SPLIT_PHASE_ON:      valBLU[outpIdx] = BLUSPLIT_PHASE_ON;    break;
                              case SPLIT_PH_MACH_MODE:  valBLU[outpIdx] = BLUSPLIT_PH_MACH_MODE;break;
                              case SET_PACK_NUM:        valBLU[outpIdx] = BLUSET_PACK_NUM;      break;
                              case SET_AC_OUT_ON:       valBLU[outpIdx] = BLUSET_AC_OUT_ON;     break;
                              case SET_DC_OUT_ON:       valBLU[outpIdx] = BLUSET_DC_OUT_ON;     break;
                              case GRID_CHANGE_ON:      valBLU[outpIdx] = BLUGRID_CHANGE_ON;    break;
                              case TIME_CTRL_ON:        valBLU[outpIdx] = BLUTIME_CTRL_ON;      break;
                              case BATT_RANGE_START:    valBLU[outpIdx] = BLUBATT_RANGE_START;  break;
                              case BATT_RANGE_END:      valBLU[outpIdx] = BLUBATT_RANGE_END;    break;
                              case BLUETOOTH_CONN:      valBLU[outpIdx] = BLUBLUETOOTH_CONN;    break;
                              case AUTO_SLEEP_MODE:     valBLU[outpIdx] = BLUAUTO_SLEEP_MODE;   break;
                              case LED_CONTROL:         valBLU[outpIdx] = BLULED_CONTROL;       break;
                              case FIELD_UNDEFINED:     valBLU[outpIdx] = BLUFIELD_UNDEFINED;   break;
                              default:   break;
                          }
                        errMQTT = (int8_t) mqtt.publish(topBLU[outpIdx].c_str(), (uint8_t*) valBLU[outpIdx].c_str(), valBLU[outpIdx].length());
                        soutMQTTerr(valBLU[outpIdx].c_str(), errMQTT);
                        pbluetti_dev_state[outpIdx].f_new = FALSE;
                        //S2VAL(" publish ", topBLU[outpIdx], valBLU[outpIdx].c_str());
                      }
                    outpIdx++;
                    if (outpIdx >= FIELD_IDX_MAX)
                      { outpIdx  = 0; }
                  }
              }
          #endif
      // --- system control --------------------------------
        if (firstrun == true)
            {
              //String taskMessage = "loop task running on core ";
              //taskMessage = taskMessage + xPortGetCoreID();
              //STXT(taskMessage);
              usLast = micros();
              firstrun = false;
            }
        anzUsCycles++;
        if (anzUsCycles >= 1000)
          {
            usPerCycle  = (micros() - usLast) / anzUsCycles;
            usLast      = micros();
            anzUsCycles = 0;
          }
      usleep(5000);
    }


// ----------------------------------------------------------------
// --- subroutine and drivers ----------------
// ----------------------------------------------------------------
  // --- system --------------------------
    // --- heap output
      void heapFree(const char* text)
        {
          uint32_t tmp32 = ESP.getFreeHeap();
          //uint32_t tmp32 = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_32BIT);
          SVAL(text, tmp32);
        }
  // --- memory --------------------------
    // --- flash
      void testFlash()
        {
          STXT(" mounting SPIFFS ... ");
          /*
            if(!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
              { STXT(" ERROR"); return; }
            uint32_t bFree = SPIFFS.totalBytes();
           */
          spiffs.init(pspiffs);
          uint32_t bFree = spiffs.totalBytes();
          SVAL(" found bytes free ", bFree);
          STXT(" dir: test_example.txt ");
          File file = spiffs.open("/test_example.txt");
          if(!file)
            {
              STXT("   Failed to open file for reading");
              return;
            }
          STXT("   File Content: ");
          int8_t n = 0;
          while(n >= 0)
            {
              n = file.read();
              if (n > 0) SOUT((char) n);
            }
          SOUTLN();
          file.close();
          spiffs.end();
          //*/
        }
  // --- network -------------------------
    // --- WIFI
      uint8_t startWIFI(bool startup)
        {
          bool ret = MD_ERR;
          char _cssid[LOGINTXT_MAX_LEN + 1];
          char _cpw[LOGINTXT_MAX_LEN + 1];
              //SVAL(" startWIFI   Start WiFi ", startup);
          #if (USE_WIFI > OFF)
              if (startup)
                {
                  ip_list ipList = ip_list(); // temporary object
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                SHEXVAL(" setup startWIFI created ipList ", (int) &ipList);
                                STXT(" setup startWIFI add WIFI 0");
                              #endif
                  sprintf(_cssid, "%s\0", WIFI_SSID0);
                  sprintf(_cpw, "%s\0", WIFI_SSID0_PW);
                  ipList.append(WIFI_FIXIP0, WIFI_GATEWAY0, WIFI_SUBNET, _cssid, _cpw);
                  #if (WIFI_ANZ_LOGIN > 1)
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                STXT(" setup startWIFI add WIFI 1");
                              #endif
                      sprintf(_cssid, "%s\0", WIFI_SSID1);
                      sprintf(_cpw, "%s\0", WIFI_SSID1_PW);
                      ipList.append(WIFI_FIXIP1, WIFI_GATEWAY1, WIFI_SUBNET, _cssid, _cpw);
                    #endif
                  #if (WIFI_ANZ_LOGIN > 2)
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                STXT(" setup startWIFI add WIFI 2");
                              #endif
                      sprintf(_cssid, "%s\0", WIFI_SSID2);
                      sprintf(_cpw, "%s\0", WIFI_SSID2_PW);
                      ipList.append(WIFI_FIXIP2, WIFI_GATEWAY2, WIFI_SUBNET, _cssid, _cpw);
                    #endif
                  #if (WIFI_ANZ_LOGIN > 3)
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                STXT(" setup startWIFI add WIFI 3");
                              #endif
                      sprintf(_cssid, "%s\0", WIFI_SSID3);
                      sprintf(_cpw, "%s\0", WIFI_SSID3_PW);
                      ipList.append(WIFI_FIXIP3, WIFI_GATEWAY3, WIFI_SUBNET, _cssid, _cpw);
                    #endif
                  #if (WIFI_ANZ_LOGIN > 4)
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                STXT(" setup startWIFI add WIFI 4");
                              #endif
                      sprintf(_cssid, "%s\0", WIFI_SSID4);
                      sprintf(_cpw, "%s\0", WIFI_SSID4_PW);
                      ipList.append(WIFI_FIXIP4, WIFI_GATEWAY4, WIFI_SUBNET, _cssid, _cpw);
                    #endif
                  #if (WIFI_ANZ_LOGIN > 5)
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                STXT(" setup startWIFI add WIFI 5");
                              #endif
                      sprintf(_cssid, "%s\0", WIFI_SSID5);
                      sprintf(_cpw, "%s\0", WIFI_SSID5_PW);
                      ipList.append(WIFI_FIXIP5, WIFI_GATEWAY5, WIFI_SUBNET, _cssid, _cpw);
                    #endif
                  #if (WIFI_ANZ_LOGIN > 6)
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                STXT(" setup startWIFI add WIFI 6");
                              #endif
                      sprintf(_cssid, "%s\0", WIFI_SSID6);
                      sprintf(_cpw, "%s\0", WIFI_SSID6_PW);
                      ipList.append(WIFI_FIXIP6, WIFI_GATEWAY6, WIFI_SUBNET, _cssid, _cpw);
                    #endif
                  #if (WIFI_ANZ_LOGIN > 7)
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                STXT(" setup startWIFI add WIFI 7");
                              #endif
                      sprintf(_cssid, "%s\0", WIFI_SSID7);
                      sprintf(_cpw, "%s\0", WIFI_SSID7_PW);
                      ipList.append(WIFI_FIXIP7, WIFI_GATEWAY7, WIFI_SUBNET, _cssid, _cpw);
                    #endif
                  #if (WIFI_ANZ_LOGIN > 8)
                            #if (DEBUG_MODE > CFG_DEBUG_STARTUP)
                                STXT(" setup add WIFI 8");
                              #endif
                      sprintf(_cssid, "%s\0", WIFI_SSID8);
                      sprintf(_cpw, "%s\0", WIFI_SSID8_PW);
                      ipList.append(WIFI_FIXIP8, WIFI_GATEWAY8, WIFI_SUBNET, _cssid, _cpw);
                    #endif
                            //STXT(UTLN(" setup startWIFI locWIFI fertig");

                            //ip_cell* pip = (ip_cell*) ipList.pFirst();
                            //char stmp[NET_MAX_SSID_LEN] = "";
                                    /*
                                      SOUT(" setup ip_list addr "); SOUT((u_long) &ipList);
                                      SOUT(" count "); SOUTLN(ipList.count());
                                      SOUT(" ip1: addr "); SOUTHEX((u_long) pip);
                                      SOUT(" locIP "); SOUTHEX(pip->locIP());
                                      SOUT(" gwIP ");  SOUTHEX(pip->gwIP());
                                      SOUT(" snIP ");  SOUTHEX(pip->snIP());
                                      pip->getSSID(stmp); SOUT(" ssid "); SOUT(stmp);
                                      pip->getPW(stmp); SOUT(" pw "); SOUTLN(stmp);
                                      pip = (ip_cell*) pip->pNext();
                                      SOUT(" ip2: addr "); SOUTHEX((u_long) pip);
                                      SOUT(" locIP "); SOUTHEX(pip->locIP());
                                      SOUT(" gwIP ");  SOUTHEX(pip->gwIP());
                                      SOUT(" snIP ");  SOUTHEX(pip->snIP());
                                      pip->getSSID(stmp); SOUT(" ssid "); SOUT(stmp);
                                      pip->getPW(stmp); SOUT(" pw "); SOUTLN(stmp);
                                      pip = (ip_cell*) pip->pNext();
                                      SOUT(" ip3: addr "); SOUTHEX((u_long) pip);
                                      SOUT(" locIP "); SOUTHEX(pip->locIP());
                                      SOUT(" gwIP ");  SOUTHEX(pip->gwIP());
                                      SOUT(" snIP ");  SOUTHEX(pip->snIP());
                                      pip->getSSID(stmp); SOUT(" ssid "); SOUT(stmp);
                                      pip->getPW(stmp); SOUT(" pw "); SOUTLN(stmp);
                                    */

                      //heapFree(" ipList generated ");
                  ret = wifi.scanWIFI(&ipList);
                          //SVAL(" scanWIFI ret=", ret);
                      //heapFree(" before deleting ipList ");
                  ipList.~ip_list();
                      //heapFree(" after deleting ipList ");
                }
              ret = wifi.startWIFI();
                  //SVAL(" startWIFI ret ", ret);
              if (ret == MD_OK)
                {
                  #if (USE_MD_ATSMARTHOME > OFF)
                      ctmp30[0]=0;
                      iret               = wifi.getSSID(ctmp30);
                        //S2VAL(" SSID ", iret, ctmp30);
                      atdbSetup.SSID     = ctmp30;
                      ctmp30[0]=0;
                      iret               = wifi.getPW(ctmp30);
                        //S2VAL(" PW ", iret, ctmp30);
                      atdbSetup.password = ctmp30;
                      atdbSetup.useWlan  = true;
                        //database.setupWiFi(&atdbSetup);
                    #endif
                  STXT("WIFI connected");
                }
                else
                {
                  STXT("WIFI error");
                  #if (USE_MD_ATSMARTHOME > OFF)
                      atdbSetup.useWlan  = false;
                    #endif
                }
            #endif // USE_WIFI
          return ret;
        }
    // --- NTP server
      void initNTPTime()
        {
          #if (USE_NTP_SERVER > OFF)
              bool ret = wifi.initNTP();
                    #if (DEBUG_MODE >= CFG_DEBUG_DETAIL)
                      //Serial.print("initNTPTime ret="); Serial.print(ret);
                    #endif
              if (ret = MD_OK)
                {
                  STXT("NTPTime ok");
                }
                else
                {
                  STXT("NTPTime error");
                }
            #endif // USE_NTP_SERVER
        }
    // --- MQTT
      #if (USE_MQTT > OFF)
          void startMQTT()
            {
              STXT("Connecting to MQTT...");
                  S2VAL(" startMQTT msgs-len &msgs[0]", sizeof(MQTTmsg_t), (uint32_t) &MQTTmsgs[0]);
              for ( uint8_t i=0 ; i < MQTT_MSG_MAXANZ - 1; i++)
                {
                  MQTTmsgs[i].pNext = (void*) &MQTTmsgs[i+1];
                      //S3VAL(" startMQTT i msgs[i] pNext", i, (uint32_t) &MQTTmsgs[i], (uint32_t) MQTTmsgs[i].pNext);
                }
              MQTTmsgs[MQTT_MSG_MAXANZ-1].pNext = (void*) &MQTTmsgs[0];
                  //S3VAL(" startMQTT i msgs[i] pNext", MQTT_MSG_MAXANZ-1, (uint32_t) &MQTTmsgs[MQTT_MSG_MAXANZ-1], (uint32_t) MQTTmsgs[MQTT_MSG_MAXANZ-1].pNext);
              connectMQTT();
              if (errMQTT == MD_OK)
                  SVAL(" ...startMQTT errMQTT", errMQTT);
            } // tested -> ok

          void connectMQTT() // TODO: move all subcribes to here -> reconnect
            {
              char cout[50] = "";
              //SVAL(" MQTT connect... errMQTT", errMQTT);
              errMQTT = (int8_t) mqtt.connectTo(MQTT_HOST, MQTT_PORT);
              soutMQTTerr(" MQTT connect", errMQTT);
              if (errMQTT == MD_OK)
                {
                  #if (BLUETTI_TYPE == BLUETTI_AC300)
                      for (uint8_t i = 0; i < FIELD_IDX_MAX ; i++)
                        {
                          switch (i)
                            {
                              // INFO device offset sort
                                case DEVICE_TYPE:
                                    pbluetti_dev_state[i].p_f_value = (void*) &BLUDEVICE_TYPE;
                                  break;
                                case SERIAL_NUMBER:
                                      pbluetti_dev_state[i].p_f_value = (void*) &BLUSERIAL_NUMBER;
                                  break;
                                case  ARM_VERSION:
                                      pbluetti_dev_state[i].p_f_value = (void*) &BLUARM_VERSION;
                                  break;
                                case  DSP_VERSION:
                                      pbluetti_dev_state[i].p_f_value = (void*) &BLUDSP_VERSION;
                                  break;
                                case  DC_INPUT_POWER:
                                      pbluetti_dev_state[DC_INPUT_POWER].p_f_value = (void*) &BLUDC_INPUT_POWER;
                                  break;
                                case  AC_INPUT_POWER:
                                      pbluetti_dev_state[AC_INPUT_POWER].p_f_value = (void*) &BLUAC_INPUT_POWER;
                                  break;
                                case  AC_OUTPUT_POWER:
                                      pbluetti_dev_state[AC_OUTPUT_POWER].p_f_value = (void*) &BLUAC_OUTPUT_POWER;
                                  break;
                                case  DC_OUTPUT_POWER:
                                      pbluetti_dev_state[DC_OUTPUT_POWER].p_f_value = (void*) &BLUDC_OUTPUT_POWER;
                                  break;
                                case  POWER_GENERATION:
                                      pbluetti_dev_state[POWER_GENERATION].p_f_value = (void*) &BLUPOWER_GENERATION;
                                  break;
                                case  TOTAL_BATT_PERC:
                                      pbluetti_dev_state[TOTAL_BATT_PERC].p_f_value = (void*) &BLUTOTAL_BATT_PERC;
                                  break;
                                case  AC_OUTPUT_ON:
                                      pbluetti_dev_state[AC_OUTPUT_ON].p_f_value = (void*) &BLUARM_VERSION;
                                  break;
                                case  DC_OUTPUT_ON:
                                      pbluetti_dev_state[DC_OUTPUT_ON].p_f_value = (void*) &BLUDC_OUTPUT_ON;
                                  break;
                              // INFO internal
                                case  AC_OUTPUT_MODE:
                                      pbluetti_dev_state[AC_OUTPUT_MODE].p_f_value = (void*) &BLUAC_OUTPUT_MODE;
                                  break;
                                case  INTERN_AC_VOLT:
                                      pbluetti_dev_state[INTERN_AC_VOLT].p_f_value = (void*) &BLUINTERN_AC_VOLT;
                                  break;
                                case  INTERN_CURR_1:
                                      pbluetti_dev_state[INTERN_CURR_1].p_f_value = (void*) &BLUINTERN_CURR_1;
                                  break;
                                case  INTERN_POWER_1:
                                      pbluetti_dev_state[INTERN_POWER_1].p_f_value = (void*) &BLUINTERN_POWER_1;
                                  break;
                                case  INTERN_AC_FREQ:
                                      pbluetti_dev_state[INTERN_AC_FREQ].p_f_value = (void*) &BLUINTERN_AC_FREQ;
                                  break;
                                case  INTERN_CURR_2:
                                      pbluetti_dev_state[INTERN_CURR_2].p_f_value = (void*) &BLUINTERN_CURR_2;
                                  break;
                                case  INTERN_POWER_2:
                                      pbluetti_dev_state[INTERN_POWER_2].p_f_value = (void*) &BLUINTERN_POWER_2;
                                  break;
                                case  AC_INPUT_VOLT:
                                      pbluetti_dev_state[AC_INPUT_VOLT].p_f_value = (void*) &BLUAC_INPUT_VOLT;
                                  break;
                                case  INTERN_CURR_3:
                                      pbluetti_dev_state[INTERN_CURR_3].p_f_value = (void*) &BLUINTERN_CURR_3;
                                  break;
                                case  INTERN_POWER_3:
                                      pbluetti_dev_state[INTERN_POWER_3].p_f_value = (void*) &BLUINTERN_POWER_3;
                                  break;
                                case  AC_INPUT_FREQ:
                                      pbluetti_dev_state[AC_INPUT_FREQ].p_f_value = (void*) &BLUAC_INPUT_FREQ;
                                  break;
                                case  INT_DC_INP_VOLT:
                                      pbluetti_dev_state[INT_DC_INP_VOLT].p_f_value = (void*) &BLUINT_DC_INP_VOLT;
                                  break;
                                case  INT_DC_INP_POW:
                                      pbluetti_dev_state[INT_DC_INP_POW].p_f_value = (void*) &BLUINT_DC_INP_POW;
                                  break;
                                case  INT_DC_INP_CURR:
                                      pbluetti_dev_state[INT_DC_INP_CURR].p_f_value = (void*) &BLUINT_DC_INP_CURR;
                                  break;
                                case  PACK_NUM_MAX:
                                      pbluetti_dev_state[PACK_NUM_MAX].p_f_value = (void*) &BLUPACK_NUM_MAX;
                                  break;
                                case  TOTAL_BATT_VOLT:
                                      pbluetti_dev_state[TOTAL_BATT_VOLT].p_f_value = (void*) &BLUTOTAL_BATT_VOLT;
                                  break;
                                case  TOTAL_BATT_CURR:
                                      pbluetti_dev_state[TOTAL_BATT_CURR].p_f_value = (void*) &BLUTOTAL_BATT_CURR;
                                  break;
                                case  PACK_NUM:
                                      pbluetti_dev_state[PACK_NUM].p_f_value = (void*) &BLUPACK_NUM;
                                  break;
                                case  PACK_STATUS:
                                      pbluetti_dev_state[PACK_STATUS].p_f_value = (void*) &BLUPACK_STATUS;
                                  break;
                                case  PACK_VOLTAGE:
                                      pbluetti_dev_state[PACK_VOLTAGE].p_f_value = (void*) &BLUPACK_VOLTAGE;
                                  break;
                                case  PACK_BATT_PERC:
                                      pbluetti_dev_state[PACK_BATT_PERC].p_f_value = (void*) &BLUPACK_BATT_PERC;
                                  break;
                                //case  CELL_VOTAGES:
                                  //break;
                              // CONTROL elements
                                case  UPS_MODE:
                                      pbluetti_dev_state[UPS_MODE].p_f_value = (void*) &BLUUPS_MODE;
                                  break;
                                case  SPLIT_PHASE_ON:
                                      pbluetti_dev_state[SPLIT_PHASE_ON].p_f_value = (void*) &BLUSPLIT_PHASE_ON;
                                  break;
                                case  SPLIT_PH_MACH_MODE:
                                      pbluetti_dev_state[SPLIT_PH_MACH_MODE].p_f_value = (void*) &BLUSPLIT_PH_MACH_MODE;
                                  break;
                                case  SET_PACK_NUM:
                                      pbluetti_dev_state[SET_PACK_NUM].p_f_value = (void*) &BLUSET_PACK_NUM;
                                  break;
                                case  SET_AC_OUT_ON:
                                      pbluetti_dev_state[SET_AC_OUT_ON].p_f_value = (void*) &BLUSET_AC_OUT_ON;
                                  break;
                                case  SET_DC_OUT_ON:
                                      pbluetti_dev_state[SET_DC_OUT_ON].p_f_value = (void*) &BLUSET_DC_OUT_ON;
                                  break;
                                case  GRID_CHANGE_ON:
                                      pbluetti_dev_state[GRID_CHANGE_ON].p_f_value = (void*) &BLUGRID_CHANGE_ON;
                                  break;
                                case  TIME_CTRL_ON:
                                      pbluetti_dev_state[TIME_CTRL_ON].p_f_value = (void*) &BLUTIME_CTRL_ON;
                                  break;
                                case  BATT_RANGE_START:
                                      pbluetti_dev_state[BATT_RANGE_START].p_f_value = (void*) &BLUBATT_RANGE_START;
                                  break;
                                case  BATT_RANGE_END:
                                      pbluetti_dev_state[BATT_RANGE_END].p_f_value = (void*) &BLUBATT_RANGE_END;
                                  break;
                                case  BLUETOOTH_CONN:
                                      pbluetti_dev_state[BLUETOOTH_CONN].p_f_value = (void*) &BLUBLUETOOTH_CONN;
                                  break;
                                case  AUTO_SLEEP_MODE:
                                      pbluetti_dev_state[AUTO_SLEEP_MODE].p_f_value = (void*) &BLUAUTO_SLEEP_MODE;
                                  break;
                                case  LED_CONTROL:
                                      pbluetti_dev_state[LED_CONTROL].p_f_value = (void*) &BLULED_CONTROL;
                                  break;
                            }
                          //S2VAL(" init ", DEVICE_F_NAMES[i], (uint32_t) pbluetti_dev_state[i].p_f_value);
                          topBLU[i] = topDevice + DEVICE_F_NAMES[i];
                          S2VAL(" make topBLU ", DEVICE_F_NAMES[i], topBLU[i]);
                          errMQTT = (int8_t) mqtt.subscribe(topBLU[i].c_str());
                          sprintf(cout, "MQTT subscribe %s\n", DEVICE_F_NAMES[i]);
                          soutMQTTerr(cout, errMQTT);
                        }
                    #endif
                  #if (USE_BME280_I2C > OFF) // 1
                      topBME280t = topDevice + topBME280t;
                      errMQTT = (int8_t) mqtt.subscribe(topBME280t.c_str());
                          soutMQTTerr(" MQTT subscribe BME280t", errMQTT);

                      topBME280p = topDevice + topBME280p;
                      errMQTT = (int8_t) mqtt.subscribe(topBME280p.c_str());
                          soutMQTTerr(" MQTT subscribe BME280p", errMQTT);

                      topBME280h = topDevice + topBME280h;
                      errMQTT = (int8_t) mqtt.subscribe(topBME280h.c_str());
                          soutMQTTerr(" MQTT subscribe BME280h", errMQTT);
                    #endif
                  #if (USE_RGBLED_PWM > OFF)
                      topRGBBright = topDevice + topRGBBright;
                      errMQTT = (int8_t) mqtt.subscribe(topRGBBright.c_str());
                          soutMQTTerr(" MQTT subscribe LEDBright ", errMQTT);

                      topRGBCol = topDevice + topRGBCol;
                      errMQTT = (int8_t) mqtt.subscribe(topRGBCol.c_str());
                          soutMQTTerr(" MQTT subscribe LEDCol ", errMQTT);
                    #endif
                }
            }
          void soutMQTTerr(String text, int8_t errMQTT)
            {
              if (errMQTT == -3)    //  no err, AlreadyConnected = no err
                {
                  errMQTT == MD_OK;
                }
              //if (errMQTT == MD_OK)
                //{
                  //errMQTTold = MD_OK;
                //}
              else
                {
                  if ((errMQTT != errMQTTold)) // display new error
                    {
                      S2VAL(text, errMQTT, cerrMQTT[(-1) * errMQTT]);
                      errMQTTold = errMQTT;
                      if (errMQTT != -7) // reset error except NotConnected
                        { errMQTT = MD_OK; }

                    }
                }
            }

          void readMQTTmsg()
            {
              if (errMQTT != MD_OK) // not connected
                {
                  connectMQTT();
                }

              if (errMQTT == MD_OK) // connected
                {
                  char* ptopic = NULL;
                  while (anzMQTTmsg > 0)
                    {
                      ptopic = pMQTTRd->topic + strlen(MQTT_TOPDEV); // remove device ID
                          //S3VAL(" readMQTT pMQTTRd ptopic payload ", (uint32_t) pMQTTRd->topic, ptopic, pMQTTRd->payload);
                          //S3VAL(" readMQTT Bright  result ", topRGBBright, pMQTTRd->topic, topRGBBright.equals(pMQTTRd->topic));
                          //S3VAL(" readMQTT Color   result ", topRGBCol,    pMQTTRd->topic, topRGBCol.equals(pMQTTRd->topic));
                          //S3VAL(" readMQTT testLED result ", toptestLED,   pMQTTRd->topic, toptestLED.equals(pMQTTRd->topic));
                      #if (USE_RGBLED_PWM > OFF)
                          //if (strcmp(ptopic, topRGBBright.c_str())) // RGB LED bright
                          if (topRGBBright.equals(pMQTTRd->topic)) // RGB LED bright
                            {
                              RGBLED->bright(atoi(pMQTTRd->payload));
                              //S2VAL(" readMQTT RGBLED new bright payload ", RGBLED->bright(), pMQTTRd->payload);
                            }
                          else if (topRGBCol.equals(pMQTTRd->topic)) // RGB LED bright
                            {
                              tmpMQTT = pMQTTRd->payload;
                              sscanf(tmpMQTT.c_str(), "%x", &tmpval32);
                              RGBLED->col24(tmpval32);
                              //SHEXVAL(" readMQTT RGBLED new color  payload ", RGBLED->col24());
                            }
                          else {}
                        #endif
                      #if (USE_GEN_DIG_OUT > OFF)
                          if (toptestLED.equals(pMQTTRd->topic)) // test-led
                            {
                              if (strcmp(pMQTTRd->payload, "false") == 0)
                                { testLED = OFF;}
                              else
                                { testLED = ON; }
                              //SVAL(" readMQTT testLED new val ", testLED);
                            }
                        #endif
                      pMQTTRd = (MQTTmsg_t*) pMQTTRd->pNext;
                      anzMQTTmsg--;
                    }
                }
            }
        #endif

