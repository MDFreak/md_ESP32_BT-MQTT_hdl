
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
        static String    BLUDEVICE_TYPE           = "";
        static String    BLUSERIAL_NUMBER         = "";
        static float     BLUARM_VERSION           = 0.0;
        static float     BLUDSP_VERSION           = 0.0;
        static uint16_t  BLUDC_INPUT_POWER        = 0;
        static uint16_t  BLUAC_INPUT_POWER        = 0;
        static uint16_t  BLUAC_OUTPUT_POWER       = 0;
        static uint16_t  BLUDC_OUTPUT_POWER       = 0;
        static float     BLUPOWER_GENERATION      = 0;
        static uint16_t  BLUTOTAL_BATT_PERC       = 0;
        static bool      BLUAC_OUTPUT_ON          = 0;
        static bool      BLUDC_OUTPUT_ON          = 0;
      // INFO internal
        static uint16_t  BLUAC_OUTPUT_MODE        = 0;
        static float     BLUINTERN_AC_VOLT        = 0.0;
        static float     BLUINTERN_CURR_1         = 0.0;
        static uint16_t  BLUINTERN_POWER_1        = 0;
        static float     BLUINTERN_AC_FREQ        = 0.0;
        static float     BLUINTERN_CURR_2         = 0.0;
        static uint16_t  BLUINTERN_POWER_2        = 0;
        static float     BLUAC_INPUT_VOLT         = 0.0;
        static float     BLUINTERN_CURR_3         = 0.0;
        static uint16_t  BLUINTERN_POWER_3        = 0;
        static float     BLUAC_INPUT_FREQ         = 0.0;
        static float     BLUINT_DC_INP_VOLT       = 0.0;
        static uint16_t  BLUINT_DC_INP_POW        = 0;
        static float     BLUINT_DC_INP_CURR       = 0.0;
        static uint16_t  BLUPACK_NUM_MAX          = 0;
        static float     BLUTOTAL_BATT_VOLT       = 0.0;
        static float     BLUTOTAL_BATT_CURR       = 0.0;
        static uint16_t  BLUPACK_NUM              = 0;
        static uint16_t  BLUPACK_STATUS           = 0;
        static float     BLUPACK_VOLTAGE          = 0.0;
        static uint16_t  BLUPACK_BATT_PERC        = 0;
        //static uint16_t  BLUCELL_VOTAGES        = 0;
        static uint16_t  BLUPACK_BMS_VERSION      = 0;
      // CONTROL elements
        static uint16_t  BLUUPS_MODE              = 0;
        static uint16_t  BLUSPLIT_PHASE_ON        = 0;
        static uint16_t  BLUSPLIT_PH_MACH_MODE    = 0;
        static uint16_t  BLUSET_PACK_NUM          = 0;
        static uint16_t  BLUSET_AC_OUT_ON         = 0;
        static uint16_t  BLUSET_DC_OUT_ON         = 0;
        static bool      BLUGRID_CHANGE_ON        = 0;
        static uint16_t  BLUTIME_CTRL_ON          = 0;
        static uint16_t  BLUBATT_RANGE_START      = 0;
        static uint16_t  BLUBATT_RANGE_END        = 0;
        static uint16_t  BLUBLUETOOTH_CONN        = 0;
        static uint16_t  BLUAUTO_SLEEP_MODE       = 0;
        static uint16_t  BLULED_CONTROL           = 0;
        static uint16_t  BLUFIELD_UNDEFINED       = 0;
    // --- old value
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
        anzUsCycles++;
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
            handleBluetooth();
          #endif
      // --- standard output cycle ---
        #ifdef USE_OUTPUT_CYCLE
            if (outpT.TOut())
              {
                outpT.startT();
                //SVAL(" outputcycle idx ", outpIdx);
                if (errMQTT == MD_OK)
                  {
                    switch(outpIdx)
                      {
                        // INFO device offset sort
                          case DEVICE_TYPE:         // 0
                              if (BLUDEVICE_TYPE != BLUDEVICE_TYPEold)
                                {
                                  valBLUDEVICE_TYPE = BLUDEVICE_TYPE;
                                  //errMQTT = (int8_t) mqtt.publish(topBLUDEVICE_TYPE.c_str(), (uint8_t*) valBLUDEVICE_TYPE.c_str(), valBLUDEVICE_TYPE.length());
                                  errMQTT = (int8_t) mqtt.publish(topBLU[FIELD_IDX_MAX].c_str(), (uint8_t*) valBLUDEVICE_TYPE.c_str(), valBLUDEVICE_TYPE.length());
                                  soutMQTTerr(valBLUDEVICE_TYPE.c_str(), errMQTT);
                                      //SVAL(topBLUDEVICE_TYPE, valBLUDEVICE_TYPE.c_str());
                                      SVAL(topBLUDEVICE_TYPE, valBLUDEVICE_TYPE.c_str());
                                  BLUDEVICE_TYPEold = BLUDEVICE_TYPE;
                                }
                            break;
                          case SERIAL_NUMBER:       // 1
                              if (BLUSERIAL_NUMBER != BLUSERIAL_NUMBERold)
                                {
                                  valBLUSERIAL_NUMBER = BLUSERIAL_NUMBER;
                                  errMQTT = (int8_t) mqtt.publish(topBLUSERIAL_NUMBER.c_str(), (uint8_t*) valBLUSERIAL_NUMBER.c_str(), valBLUSERIAL_NUMBER.length());
                                  soutMQTTerr(valBLUSERIAL_NUMBER.c_str(), errMQTT);
                                      SVAL(topBLUSERIAL_NUMBER, valBLUSERIAL_NUMBER.c_str());
                                  BLUSERIAL_NUMBERold = BLUSERIAL_NUMBER;
                                }
                            break;
                          case ARM_VERSION:         // 2
                              if (BLUARM_VERSION != BLUARM_VERSIONold)
                                {
                                  valBLUARM_VERSION = BLUARM_VERSION;
                                  errMQTT = (int8_t) mqtt.publish(topBLUARM_VERSION.c_str(), (uint8_t*) valBLUARM_VERSION.c_str(), valBLUARM_VERSION.length());
                                  soutMQTTerr(valBLUDC_OUTPUT_POWER.c_str(), errMQTT);
                                      SVAL(topBLUARM_VERSION, valBLUARM_VERSION.c_str());
                                  BLUARM_VERSIONold = BLUARM_VERSION;
                                }
                            break;
                          case DSP_VERSION:         // 3
                              if (BLUDSP_VERSION != BLUDSP_VERSION)
                                {
                                  valBLUDSP_VERSION = BLUDSP_VERSION;
                                  errMQTT = (int8_t) mqtt.publish(topBLUDSP_VERSION.c_str(), (uint8_t*) valBLUDSP_VERSION.c_str(), valBLUDSP_VERSION.length());
                                  soutMQTTerr(valBLUDC_OUTPUT_POWER.c_str(), errMQTT);
                                      SVAL(topBLUDSP_VERSION, valBLUDSP_VERSION.c_str());
                                  BLUDSP_VERSION = BLUDSP_VERSION;
                                }
                            break;
                          case DC_INPUT_POWER:      // 4
                              if (BLUDC_INPUT_POWER != BLUDC_INPUT_POWERold)
                                {
                                  valBLUDC_INPUT_POWER = BLUDC_INPUT_POWER;
                                  errMQTT = (int8_t) mqtt.publish(topBLUDC_INPUT_POWER.c_str(), (uint8_t*) valBLUDC_INPUT_POWER.c_str(), valBLUDC_INPUT_POWER.length());
                                  soutMQTTerr(valBLUDC_INPUT_POWER.c_str(), errMQTT);
                                      SVAL(topBLUDC_INPUT_POWER, valBLUDC_INPUT_POWER.c_str());
                                  BLUDC_INPUT_POWERold = BLUDC_INPUT_POWER;
                                }
                            break;
                          case AC_INPUT_POWER:      // 5
                              if (BLUAC_INPUT_POWER != BLUAC_INPUT_POWERold)
                                {
                                  valBLUAC_INPUT_POWER = BLUAC_INPUT_POWER;
                                  errMQTT = (int8_t) mqtt.publish(topBLUAC_INPUT_POWER.c_str(), (uint8_t*) valBLUAC_INPUT_POWER.c_str(), valBLUAC_INPUT_POWER.length());
                                  soutMQTTerr(valBLUAC_INPUT_POWER.c_str(), errMQTT);
                                      SVAL(topBLUAC_INPUT_POWER, valBLUAC_INPUT_POWER.c_str());
                                  BLUAC_INPUT_POWERold = BLUAC_INPUT_POWER;
                                }
                            break;
                          case AC_OUTPUT_POWER:     // 6
                              if (BLUAC_OUTPUT_POWER != BLUAC_OUTPUT_POWERold)
                                {
                                  valBLUDC_OUTPUT_POWER = BLUDC_OUTPUT_POWER;
                                  errMQTT = (int8_t) mqtt.publish(topBLUDC_OUTPUT_POWER.c_str(), (uint8_t*) valBLUAC_OUTPUT_POWER.c_str(), valBLUAC_OUTPUT_POWER.length());
                                  soutMQTTerr(valBLUAC_OUTPUT_POWER.c_str(), errMQTT);
                                      SVAL(topBLUDC_OUTPUT_POWER, valBLUAC_OUTPUT_POWER.c_str());
                                  BLUAC_OUTPUT_POWERold = BLUAC_OUTPUT_POWER;
                                }
                            break;
                          case DC_OUTPUT_POWER:     // 7
                              if (BLUDC_OUTPUT_POWER != BLUDC_OUTPUT_POWERold)
                                {
                                  valBLUDC_OUTPUT_POWER = BLUDC_OUTPUT_POWER;
                                  errMQTT = (int8_t) mqtt.publish(topBLUDC_OUTPUT_ON.c_str(), (uint8_t*) valBLUDC_OUTPUT_POWER.c_str(), valBLUDC_OUTPUT_POWER.length());
                                  soutMQTTerr(valBLUDC_OUTPUT_POWER.c_str(), errMQTT);
                                      SVAL(topBLUDC_OUTPUT_ON, valBLUDC_OUTPUT_POWER.c_str());
                                  BLUDC_OUTPUT_POWERold = BLUDC_OUTPUT_POWER;
                                }
                            break;
                          case POWER_GENERATION:    // 8
                              if (BLUAC_OUTPUT_POWER != BLUAC_OUTPUT_POWERold)
                                {
                                    valBLUAC_OUTPUT_POWER = BLUAC_OUTPUT_POWER;
                                    errMQTT = (int8_t) mqtt.publish(topBLUPOWER_GENERATION.c_str(), (uint8_t*) valBLUAC_OUTPUT_POWER.c_str(), valBLUAC_OUTPUT_POWER.length());
                                    soutMQTTerr(valBLUAC_OUTPUT_POWER.c_str(), errMQTT);
                                        SVAL(topBLUPOWER_GENERATION, valBLUAC_OUTPUT_POWER.c_str());
                                  BLUAC_OUTPUT_POWERold = BLUAC_OUTPUT_POWER;
                                }
                            break;
                          case TOTAL_BATT_PERC:     // 9
                              if (BLUTOTAL_BATT_PERC != BLUTOTAL_BATT_PERCold)
                                {
                                  valBLUTOTAL_BATT_PERC = BLUTOTAL_BATT_PERC;
                                  errMQTT = (int8_t) mqtt.publish(topBLUTOTAL_BATT_PERC.c_str(), (uint8_t*) valBLUTOTAL_BATT_PERC.c_str(), valBLUTOTAL_BATT_PERC.length());
                                  soutMQTTerr(valBLUTOTAL_BATT_PERC.c_str(), errMQTT);
                                      SVAL(topBLUTOTAL_BATT_PERC, valBLUTOTAL_BATT_PERC.c_str());
                                  BLUTOTAL_BATT_PERCold = BLUTOTAL_BATT_PERC;
                                }
                            break;
                          case AC_OUTPUT_ON:        // 10
                              if (BLUAC_OUTPUT_POWER != BLUARM_VERSIONold)
                                {
                                  valBLUARM_VERSION = BLUARM_VERSIONold;
                                  errMQTT = (int8_t) mqtt.publish(topBLUAC_OUTPUT_POWER.c_str(), (uint8_t*) valBLUARM_VERSION.c_str(), valBLUARM_VERSION.length());
                                  soutMQTTerr(valBLUARM_VERSION.c_str(), errMQTT);
                                      SVAL(topBLUAC_OUTPUT_POWER, valBLUARM_VERSION.c_str());
                                  BLUARM_VERSIONold = BLUARM_VERSION;
                                }
                            break;
                          case DC_OUTPUT_ON:        // 11
                              if (BLUAC_OUTPUT_ON != BLUAC_OUTPUT_ONold)
                                {
                                  valBLUAC_OUTPUT_ON = BLUAC_OUTPUT_ON;
                                  errMQTT = (int8_t) mqtt.publish(topBLUAC_OUTPUT_ON.c_str(), (uint8_t*) valBLUAC_OUTPUT_ON.c_str(), valBLUAC_OUTPUT_ON.length());
                                  soutMQTTerr(valBLUAC_OUTPUT_ON.c_str(), errMQTT);
                                      SVAL(topBLUAC_OUTPUT_ON, valBLUAC_OUTPUT_ON.c_str());
                                  BLUAC_OUTPUT_ONold = BLUAC_OUTPUT_ON;
                                }
                            break;
                        // INFO internal
                          case AC_OUTPUT_MODE:      // 12
                              if (BLUAC_OUTPUT_MODE != BLUAC_OUTPUT_MODEold)
                                {
                                  valBLUAC_OUTPUT_MODE = BLUAC_OUTPUT_MODE;
                                  errMQTT = (int8_t) mqtt.publish(topBLUAC_OUTPUT_MODE.c_str(), (uint8_t*) valBLUAC_OUTPUT_MODE.c_str(), valBLUAC_OUTPUT_MODE.length());
                                  soutMQTTerr(valBLUAC_OUTPUT_MODE.c_str(), errMQTT);
                                      SVAL(topBLUPACK_VOLTAGE, valBLUAC_OUTPUT_MODE.c_str());
                                  BLUAC_OUTPUT_MODEold = AC_OUTPUT_MODE;
                                }
                            break;
                          case INTERN_AC_VOLT:      // 13
                              if (BLUINTERN_AC_VOLT != BLUINTERN_AC_VOLTold)
                                {
                                  valBLUINTERN_AC_VOLT = BLUINTERN_AC_VOLT;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINTERN_AC_VOLT.c_str(), (uint8_t*) valBLUINTERN_AC_VOLT.c_str(), valBLUINTERN_AC_VOLT.length());
                                  soutMQTTerr(valBLUINTERN_AC_VOLT.c_str(), errMQTT);
                                      SVAL(topBLUINTERN_AC_VOLT, valBLUINTERN_AC_VOLT.c_str());
                                  BLUINTERN_AC_VOLTold = BLUINTERN_AC_VOLT;
                                }
                            break;
                          case INTERN_CURR_1:       // 14
                              if (BLUINTERN_CURR_1 != BLUINTERN_CURR_1old)
                                {
                                  valBLUINTERN_CURR_1 = BLUINTERN_CURR_1;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINTERN_CURR_1.c_str(), (uint8_t*) valBLUINTERN_CURR_1.c_str(), valBLUINTERN_CURR_1.length());
                                  soutMQTTerr(valBLUINTERN_CURR_1.c_str(), errMQTT);
                                      SVAL(topBLUINTERN_CURR_1, valBLUINTERN_CURR_1.c_str());
                                  BLUINTERN_CURR_1old = BLUINTERN_CURR_1;
                                }
                            break;
                          case INTERN_POWER_1:  // 15
                              if (BLUINTERN_POWER_1 != BLUINTERN_POWER_1old)
                                {
                                  valBLUINTERN_POWER_1 = BLUINTERN_POWER_1;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINTERN_POWER_1.c_str(), (uint8_t*) valBLUINTERN_POWER_1.c_str(), valBLUINTERN_POWER_1.length());
                                  soutMQTTerr(valBLUINTERN_POWER_1.c_str(), errMQTT);
                                      SVAL(topBLUINTERN_POWER_1, valBLUINTERN_POWER_1.c_str());
                                  BLUINTERN_POWER_1old = BLUINTERN_POWER_1;
                                }
                            break;
                          case INTERN_AC_FREQ:  // 16
                              if (BLUINTERN_AC_FREQ != BLUINTERN_AC_FREQold)
                                {
                                  valBLUINTERN_AC_FREQ = BLUINTERN_AC_FREQ;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINTERN_AC_FREQ.c_str(), (uint8_t*) valBLUINTERN_AC_FREQ.c_str(), valBLUINTERN_AC_FREQ.length());
                                  soutMQTTerr(valBLUINTERN_AC_FREQ.c_str(), errMQTT);
                                      SVAL(topBLUINTERN_AC_FREQ, valBLUINTERN_AC_FREQ.c_str());
                                  BLUINTERN_AC_FREQold = BLUINTERN_AC_FREQ;
                                }
                            break;
                          case INTERN_CURR_2:  // 17
                              if (BLUINTERN_CURR_2 != BLUINTERN_CURR_2old)
                                {
                                  valBLUINTERN_CURR_2 = BLUINTERN_CURR_2;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINTERN_CURR_2.c_str(), (uint8_t*) valBLUINTERN_CURR_2.c_str(), valBLUINTERN_CURR_2.length());
                                  soutMQTTerr(valBLUINTERN_CURR_2.c_str(), errMQTT);
                                      SVAL(topBLUINTERN_CURR_2, valBLUINTERN_CURR_2.c_str());
                                  BLUINTERN_CURR_2old = BLUINTERN_CURR_2;
                                }
                            break;
                          case INTERN_POWER_2:  // 18
                              if (BLUINTERN_POWER_2 != BLUINTERN_POWER_2old)
                                {
                                  valBLUINTERN_POWER_2 = BLUINTERN_POWER_2;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINTERN_POWER_2.c_str(), (uint8_t*) valBLUINTERN_POWER_2.c_str(), valBLUINTERN_POWER_2.length());
                                  soutMQTTerr(valBLUINTERN_POWER_2.c_str(), errMQTT);
                                      SVAL(topBLUINTERN_POWER_2, valBLUINTERN_POWER_2.c_str());
                                  BLUINTERN_POWER_2old = BLUINTERN_POWER_2;
                                }
                            break;
                          case AC_INPUT_VOLT:  // 19
                              if (BLUAC_INPUT_VOLT != BLUAC_INPUT_VOLTold)
                                {
                                  valBLUAC_INPUT_VOLT = BLUAC_INPUT_VOLT;
                                  errMQTT = (int8_t) mqtt.publish(topBLUAC_INPUT_VOLT.c_str(), (uint8_t*) valBLUAC_INPUT_VOLT.c_str(), valBLUAC_INPUT_VOLT.length());
                                  soutMQTTerr(valBLUAC_INPUT_VOLT.c_str(), errMQTT);
                                      SVAL(topBLUAC_INPUT_VOLT, valBLUAC_INPUT_VOLT.c_str());
                                  BLUAC_INPUT_VOLTold = BLUAC_INPUT_VOLT;
                                }
                            break;
                          case INTERN_CURR_3:  // 20
                              if (BLUINTERN_CURR_3 != BLUINTERN_CURR_3old)
                                {
                                  valBLUINTERN_CURR_3 = BLUINTERN_CURR_3;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINTERN_CURR_3.c_str(), (uint8_t*) valBLUINTERN_CURR_3.c_str(), valBLUINTERN_CURR_3.length());
                                  soutMQTTerr(valBLUINTERN_CURR_3.c_str(), errMQTT);
                                      SVAL(topBLUINTERN_CURR_3, valBLUINTERN_CURR_3.c_str());
                                  BLUINTERN_CURR_3old = BLUINTERN_CURR_3;
                                }
                            break;
                          case INTERN_POWER_3:  // 21
                              if (BLUINTERN_POWER_3 != BLUINTERN_POWER_3old)
                                {
                                  valBLUINTERN_POWER_3 = BLUINTERN_POWER_3;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINTERN_POWER_3.c_str(), (uint8_t*) valBLUINTERN_POWER_3.c_str(), valBLUINTERN_POWER_3.length());
                                  soutMQTTerr(valBLUINTERN_POWER_3.c_str(), errMQTT);
                                      SVAL(topBLUINTERN_POWER_3, valBLUINTERN_POWER_3.c_str());
                                  BLUINTERN_POWER_3old = BLUINTERN_POWER_3;
                                }
                            break;
                          case AC_INPUT_FREQ:  // 22
                              if (BLUAC_INPUT_FREQ != BLUAC_INPUT_FREQold)
                                {
                                  valBLUAC_INPUT_FREQ = BLUAC_INPUT_FREQ;
                                  errMQTT = (int8_t) mqtt.publish(topBLUAC_INPUT_FREQ.c_str(), (uint8_t*) valBLUAC_INPUT_FREQ.c_str(), valBLUAC_INPUT_FREQ.length());
                                  soutMQTTerr(valBLUAC_INPUT_FREQ.c_str(), errMQTT);
                                      SVAL(topBLUAC_INPUT_FREQ, valBLUAC_INPUT_FREQ.c_str());
                                  BLUAC_INPUT_FREQold = BLUAC_INPUT_FREQ;
                                }
                            break;
                          case INT_DC_INP_VOLT:  // 23
                              if (BLUINT_DC_INP_VOLT != BLUINT_DC_INP_VOLTold)
                                {
                                  valBLUINT_DC_INP_VOLT = BLUINT_DC_INP_VOLT;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINT_DC_INP_VOLT.c_str(), (uint8_t*) valBLUINT_DC_INP_VOLT.c_str(), valBLUINT_DC_INP_VOLT.length());
                                  soutMQTTerr(valBLUINT_DC_INP_VOLT.c_str(), errMQTT);
                                      SVAL(topBLUINT_DC_INP_VOLT, valBLUINT_DC_INP_VOLT.c_str());
                                  BLUINT_DC_INP_VOLTold = BLUINT_DC_INP_VOLT;
                                }
                            break;
                          case INT_DC_INP_POW:  // 24
                              if (BLUINT_DC_INP_POW != BLUINT_DC_INP_POWold)
                                {
                                  valBLUINT_DC_INP_POW = BLUINT_DC_INP_POW;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINT_DC_INP_POW.c_str(), (uint8_t*) valBLUINT_DC_INP_POW.c_str(), valBLUINT_DC_INP_POW.length());
                                  soutMQTTerr(valBLUINT_DC_INP_POW.c_str(), errMQTT);
                                      SVAL(topBLUINT_DC_INP_POW, valBLUINT_DC_INP_POW.c_str());
                                  BLUINT_DC_INP_POWold = BLUINT_DC_INP_POW;
                                }
                            break;
                          case INT_DC_INP_CURR:  // 25
                              if (BLUINT_DC_INP_CURR != BLUINT_DC_INP_CURRold)
                                {
                                  valBLUINT_DC_INP_CURR = BLUINT_DC_INP_CURR;
                                  errMQTT = (int8_t) mqtt.publish(topBLUINT_DC_INP_CURR.c_str(), (uint8_t*) valBLUINT_DC_INP_CURR.c_str(), valBLUINT_DC_INP_CURR.length());
                                  soutMQTTerr(valBLUINT_DC_INP_CURR.c_str(), errMQTT);
                                      SVAL(topBLUINT_DC_INP_CURR, valBLUINT_DC_INP_CURR.c_str());
                                  BLUINT_DC_INP_CURRold = BLUINT_DC_INP_CURR;
                                }
                            break;
                          case PACK_NUM_MAX:  // 26
                              if (BLUPACK_NUM_MAX != BLUPACK_NUM_MAXold)
                                {
                                  valBLUPACK_NUM_MAX = BLUPACK_NUM_MAX;
                                  errMQTT = (int8_t) mqtt.publish(topBLUPACK_NUM_MAX.c_str(), (uint8_t*) valBLUPACK_NUM_MAX.c_str(), valBLUPACK_NUM_MAX.length());
                                  soutMQTTerr(valBLUPACK_NUM_MAX.c_str(), errMQTT);
                                      SVAL(topBLUPACK_NUM_MAX, valBLUPACK_NUM_MAX.c_str());
                                  BLUPACK_NUM_MAXold = BLUPACK_NUM_MAX;
                                }
                            break;
                          case TOTAL_BATT_VOLT:  // 27
                              if (BLUTOTAL_BATT_VOLT != BLUTOTAL_BATT_VOLTold)
                                {
                                  valBLUTOTAL_BATT_VOLT = BLUTOTAL_BATT_VOLT;
                                  errMQTT = (int8_t) mqtt.publish(topBLUTOTAL_BATT_VOLT.c_str(), (uint8_t*) valBLUTOTAL_BATT_VOLT.c_str(), valBLUTOTAL_BATT_VOLT.length());
                                  soutMQTTerr(valBLUTOTAL_BATT_VOLT.c_str(), errMQTT);
                                      SVAL(topBLUTOTAL_BATT_VOLT, valBLUTOTAL_BATT_VOLT.c_str());
                                  BLUTOTAL_BATT_VOLTold = BLUTOTAL_BATT_VOLT;
                                }
                            break;
                          case TOTAL_BATT_CURR:  // 28
                              if (BLUTOTAL_BATT_CURR != BLUTOTAL_BATT_CURRold)
                                {
                                  valBLUTOTAL_BATT_CURR = BLUTOTAL_BATT_CURR;
                                  errMQTT = (int8_t) mqtt.publish(topBLUTOTAL_BATT_CURR.c_str(), (uint8_t*) valBLUTOTAL_BATT_CURR.c_str(), valBLUTOTAL_BATT_CURR.length());
                                  soutMQTTerr(valBLUTOTAL_BATT_CURR.c_str(), errMQTT);
                                      SVAL(topBLUTOTAL_BATT_CURR, valBLUTOTAL_BATT_CURR.c_str());
                                  BLUTOTAL_BATT_CURRold = BLUTOTAL_BATT_CURR;
                                }
                            break;
                          case PACK_NUM:  // 29
                              if (BLUPACK_NUM != BLUPACK_NUMold)
                                {
                                  valBLUPACK_NUM = BLUPACK_NUM;
                                  errMQTT = (int8_t) mqtt.publish(topBLUPACK_NUM.c_str(), (uint8_t*) valBLUPACK_NUM.c_str(), valBLUPACK_NUM.length());
                                  soutMQTTerr(valBLUPACK_NUM.c_str(), errMQTT);
                                      SVAL(topBLUPACK_NUM, valBLUPACK_NUM.c_str());
                                  BLUPACK_NUMold = BLUPACK_NUM;
                                }
                            break;
                          case PACK_STATUS:  // 30
                              if (BLUPACK_STATUS != BLUPACK_STATUSold)
                                {
                                  valBLUPACK_STATUS = BLUPACK_STATUS;
                                  errMQTT = (int8_t) mqtt.publish(topBLUPACK_STATUS.c_str(), (uint8_t*) valBLUPACK_STATUS.c_str(), valBLUPACK_STATUS.length());
                                  soutMQTTerr(valBLUPACK_STATUS.c_str(), errMQTT);
                                      SVAL(topBLUPACK_STATUS, valBLUPACK_STATUS.c_str());
                                  BLUPACK_STATUSold = BLUPACK_STATUS;
                                }
                            break;
                          case PACK_VOLTAGE:  // 31
                              if (BLUPACK_VOLTAGE != BLUPACK_VOLTAGEold)
                                {
                                  valBLUPACK_VOLTAGE = BLUPACK_VOLTAGE;
                                  errMQTT = (int8_t) mqtt.publish(topBLUPACK_VOLTAGE.c_str(), (uint8_t*) valBLUPACK_VOLTAGE.c_str(), valBLUPACK_VOLTAGE.length());
                                  soutMQTTerr(valBLUPACK_VOLTAGE.c_str(), errMQTT);
                                      SVAL(topBLUPACK_VOLTAGE, valBLUPACK_VOLTAGE.c_str());
                                  BLUPACK_VOLTAGEold = BLUPACK_VOLTAGE;
                                }
                            break;
                          case PACK_BATT_PERC:  // 32
                              if (BLUPACK_BATT_PERC != BLUPACK_BATT_PERCold)
                                {
                                  valBLUPACK_BATT_PERC = BLUPACK_BATT_PERC;
                                  errMQTT = (int8_t) mqtt.publish(topBLUPACK_BATT_PERC.c_str(), (uint8_t*) valBLUPACK_BATT_PERC.c_str(), valBLUPACK_BATT_PERC.length());
                                  soutMQTTerr(valBLUPACK_BATT_PERC.c_str(), errMQTT);
                                      SVAL(topBLUPACK_BATT_PERC, valBLUPACK_BATT_PERC.c_str());
                                  BLUPACK_BATT_PERCold = BLUPACK_BATT_PERC;
                                }
                            break;
                          //case CELL_VOTAGES // 33
                              /*
                                if (BLUCELL_VOTAGES != BLUCELL_VOTAGESold)
                                  {
                                    valBLUCELL_VOTAGES = BLUCELL_VOTAGES;
                                    errMQTT = (int8_t) mqtt.publish(topBLUCELL_VOTAGES.c_str(), (uint8_t*) valBLUCELL_VOTAGES.c_str(), valBLUCELL_VOTAGES.length());
                                    soutMQTTerr(valBLUCELL_VOTAGES.c_str(), errMQTT);
                                        SVAL(topBLUCELL_VOTAGES, valBLUCELL_VOTAGES.c_str());
                                    BLUCELL_VOTAGESold = BLUCELL_VOTAGES;
                                  }
                                break;
                               */
                          case PACK_BMS_VERSION:  // 34
                              if (BLUPACK_BMS_VERSION != BLUPACK_BMS_VERSIONold)
                                {
                                  valBLUPACK_BMS_VERSION = BLUPACK_BMS_VERSION;
                                  errMQTT = (int8_t) mqtt.publish(topBLUPACK_BMS_VERSION.c_str(), (uint8_t*) valBLUPACK_BMS_VERSION.c_str(), valBLUPACK_BMS_VERSION.length());
                                  soutMQTTerr(valBLUPACK_BMS_VERSION.c_str(), errMQTT);
                                      SVAL(topBLUPACK_BMS_VERSION, valBLUPACK_BMS_VERSION.c_str());
                                  BLUPACK_BMS_VERSIONold = BLUPACK_BMS_VERSION;
                                }
                            break;
                        // CONTROL elements
                          case UPS_MODE:  // 35
                              if (BLUUPS_MODE != BLUUPS_MODEold)
                                {
                                  valBLUUPS_MODE = BLUUPS_MODE;
                                  errMQTT = (int8_t) mqtt.publish(topBLUUPS_MODE.c_str(), (uint8_t*) valBLUUPS_MODE.c_str(), valBLUUPS_MODE.length());
                                  soutMQTTerr(valBLUUPS_MODE.c_str(), errMQTT);
                                      SVAL(topBLUUPS_MODE, valBLUUPS_MODE.c_str());
                                  BLUUPS_MODEold = BLUUPS_MODE;
                                }
                            break;
                          case SPLIT_PHASE_ON:  // 36
                              if (BLUSPLIT_PHASE_ON != BLUSPLIT_PHASE_ONold)
                                {
                                  valBLUSPLIT_PHASE_ON = BLUSPLIT_PHASE_ON;
                                  errMQTT = (int8_t) mqtt.publish(topBLUSPLIT_PHASE_ON.c_str(), (uint8_t*) valBLUSPLIT_PHASE_ON.c_str(), valBLUSPLIT_PHASE_ON.length());
                                  soutMQTTerr(valBLUSPLIT_PHASE_ON.c_str(), errMQTT);
                                      SVAL(topBLUSPLIT_PHASE_ON, valBLUSPLIT_PHASE_ON.c_str());
                                  BLUSPLIT_PHASE_ONold = BLUSPLIT_PHASE_ON;
                                }
                            break;
                          case SPLIT_PH_MACH_MODE:  // 37
                              if (BLUSPLIT_PH_MACH_MODE != BLUSPLIT_PH_MACH_MODEold)
                                {
                                  valBLUSPLIT_PH_MACH_MODE = BLUSPLIT_PH_MACH_MODE;
                                  errMQTT = (int8_t) mqtt.publish(topBLUSPLIT_PH_MACH_MODE.c_str(), (uint8_t*) valBLUSPLIT_PH_MACH_MODE.c_str(), valBLUSPLIT_PH_MACH_MODE.length());
                                  soutMQTTerr(valBLUSPLIT_PH_MACH_MODE.c_str(), errMQTT);
                                      SVAL(topBLUSPLIT_PH_MACH_MODE, valBLUSPLIT_PH_MACH_MODE.c_str());
                                  BLUSPLIT_PH_MACH_MODEold = BLUSPLIT_PH_MACH_MODE;
                                }
                            break;
                          case SET_PACK_NUM:  // 38
                              if (BLUSET_PACK_NUM != BLUSET_PACK_NUMold)
                                {
                                  valBLUSET_PACK_NUM = BLUSET_PACK_NUM;
                                  errMQTT = (int8_t) mqtt.publish(topBLUSET_PACK_NUM.c_str(), (uint8_t*) valBLUSET_PACK_NUM.c_str(), valBLUSET_PACK_NUM.length());
                                  soutMQTTerr(valBLUSET_PACK_NUM.c_str(), errMQTT);
                                      SVAL(topBLUSET_PACK_NUM, valBLUSET_PACK_NUM.c_str());
                                  BLUSET_PACK_NUMold = BLUSET_PACK_NUM;
                                }
                            break;
                          case SET_AC_OUT_ON:  // 39
                              if (BLUSET_AC_OUT_ON != BLUSET_AC_OUT_ONold)
                                {
                                  valBLUSET_AC_OUT_ON = BLUSET_AC_OUT_ON;
                                  errMQTT = (int8_t) mqtt.publish(topBLUSET_AC_OUT_ON.c_str(), (uint8_t*) valBLUSET_AC_OUT_ON.c_str(), valBLUSET_AC_OUT_ON.length());
                                  soutMQTTerr(valBLUSET_AC_OUT_ON.c_str(), errMQTT);
                                      SVAL(topBLUSET_AC_OUT_ON, valBLUSET_AC_OUT_ON.c_str());
                                  BLUSET_AC_OUT_ONold = BLUSET_AC_OUT_ON;
                                }
                            break;
                          case SET_DC_OUT_ON:  // 40
                              if (BLUSET_DC_OUT_ON != BLUSET_DC_OUT_ONold)
                                {
                                  valBLUSET_DC_OUT_ON = BLUSET_DC_OUT_ON;
                                  errMQTT = (int8_t) mqtt.publish(topBLUSET_DC_OUT_ON.c_str(), (uint8_t*) valBLUSET_DC_OUT_ON.c_str(), valBLUSET_DC_OUT_ON.length());
                                  soutMQTTerr(valBLUSET_DC_OUT_ON.c_str(), errMQTT);
                                      SVAL(topBLUSET_DC_OUT_ON, valBLUSET_DC_OUT_ON.c_str());
                                  BLUSET_DC_OUT_ONold = BLUSET_DC_OUT_ON;
                                }
                            break;
                          case GRID_CHANGE_ON:  // 41
                              if (BLUGRID_CHANGE_ON != BLUGRID_CHANGE_ONold)
                                {
                                  valBLUGRID_CHANGE_ON = BLUGRID_CHANGE_ON;
                                  errMQTT = (int8_t) mqtt.publish(topBLUGRID_CHANGE_ON.c_str(), (uint8_t*) valBLUGRID_CHANGE_ON.c_str(), valBLUGRID_CHANGE_ON.length());
                                  soutMQTTerr(valBLUDC_OUTPUT_POWER.c_str(), errMQTT);
                                      SVAL(topBLUGRID_CHANGE_ON, valBLUGRID_CHANGE_ON.c_str());
                                  BLUGRID_CHANGE_ONold = BLUGRID_CHANGE_ON;
                                }
                            break;
                          case TIME_CTRL_ON:  // 42
                              if (BLUTIME_CTRL_ON != BLUTIME_CTRL_ONold)
                                {
                                  valBLUTIME_CTRL_ON = BLUTIME_CTRL_ON;
                                  errMQTT = (int8_t) mqtt.publish(topBLUTIME_CTRL_ON.c_str(), (uint8_t*) valBLUTIME_CTRL_ON.c_str(), valBLUTIME_CTRL_ON.length());
                                  soutMQTTerr(valBLUTIME_CTRL_ON.c_str(), errMQTT);
                                      SVAL(topBLUTIME_CTRL_ON, valBLUTIME_CTRL_ON.c_str());
                                  BLUTIME_CTRL_ONold = BLUTIME_CTRL_ON;
                                }
                            break;
                          case BATT_RANGE_START:  // 43
                              if (BLUBATT_RANGE_START != BLUBATT_RANGE_STARTold)
                                {
                                  valBLUBATT_RANGE_START = BLUBATT_RANGE_START;
                                  errMQTT = (int8_t) mqtt.publish(topBLUBATT_RANGE_START.c_str(), (uint8_t*) valBLUBATT_RANGE_START.c_str(), valBLUBATT_RANGE_START.length());
                                  soutMQTTerr(valBLUBATT_RANGE_START.c_str(), errMQTT);
                                      SVAL(topBLUBATT_RANGE_START, valBLUBATT_RANGE_START.c_str());
                                  BLUBATT_RANGE_STARTold = BLUBATT_RANGE_START;
                                }
                            break;
                          case BATT_RANGE_END:  // 44
                              if (BLUBATT_RANGE_END != BLUBATT_RANGE_ENDold)
                                {
                                  valBLUBATT_RANGE_END = BLUBATT_RANGE_END;
                                  errMQTT = (int8_t) mqtt.publish(topBLUBATT_RANGE_END.c_str(), (uint8_t*) valBLUBATT_RANGE_END.c_str(), valBLUBATT_RANGE_END.length());
                                  soutMQTTerr(valBLUBATT_RANGE_END.c_str(), errMQTT);
                                      SVAL(topBLUBATT_RANGE_END, valBLUBATT_RANGE_END.c_str());
                                  BLUBATT_RANGE_ENDold = BLUBATT_RANGE_END;
                                }
                            break;
                          case BLUETOOTH_CONN:  // 45
                              if (BLUBLUETOOTH_CONN != BLUBLUETOOTH_CONNold)
                                {
                                  valBLUBLUETOOTH_CONN = BLUBLUETOOTH_CONN;
                                  errMQTT = (int8_t) mqtt.publish(topBLUBLUETOOTH_CONN.c_str(), (uint8_t*) valBLUBLUETOOTH_CONN.c_str(), valBLUBLUETOOTH_CONN.length());
                                  soutMQTTerr(valBLUBLUETOOTH_CONN.c_str(), errMQTT);
                                      SVAL(topBLUBLUETOOTH_CONN, valBLUBLUETOOTH_CONN.c_str());
                                  BLUBLUETOOTH_CONNold = BLUBLUETOOTH_CONN;
                                }
                            break;
                          case AUTO_SLEEP_MODE:  // 46
                              if (BLUAUTO_SLEEP_MODE != BLUAUTO_SLEEP_MODEold)
                                {
                                  valBLUAUTO_SLEEP_MODE = BLUAUTO_SLEEP_MODE;
                                  errMQTT = (int8_t) mqtt.publish(topBLUAUTO_SLEEP_MODE.c_str(), (uint8_t*) valBLUAUTO_SLEEP_MODE.c_str(), valBLUAUTO_SLEEP_MODE.length());
                                  soutMQTTerr(valBLUAUTO_SLEEP_MODE.c_str(), errMQTT);
                                      SVAL(topBLUAUTO_SLEEP_MODE, valBLUAUTO_SLEEP_MODE.c_str());
                                  BLUAUTO_SLEEP_MODEold = BLUAUTO_SLEEP_MODE;
                                }
                            break;
                          case LED_CONTROL:  // 47
                              if (BLULED_CONTROL != BLULED_CONTROLold)
                                {
                                  valBLULED_CONTROL = BLULED_CONTROL;
                                  errMQTT = (int8_t) mqtt.publish(topBLULED_CONTROL.c_str(), (uint8_t*) valBLULED_CONTROL.c_str(), valBLULED_CONTROL.length());
                                  soutMQTTerr(valBLULED_CONTROL.c_str(), errMQTT);
                                      SVAL(topBLULED_CONTROL, valBLULED_CONTROL.c_str());
                                  BLULED_CONTROLold = BLULED_CONTROL;
                                }
                            break;
                          case FIELD_UNDEFINED:  // 48
                              if (BLUFIELD_UNDEFINED != BLUFIELD_UNDEFINEDold)
                                {
                                  valBLUFIELD_UNDEFINED = BLUFIELD_UNDEFINED;
                                  errMQTT = (int8_t) mqtt.publish(topBLUFIELD_UNDEFINED.c_str(), (uint8_t*) valBLUFIELD_UNDEFINED.c_str(), valBLUFIELD_UNDEFINED.length());
                                  soutMQTTerr(valBLUFIELD_UNDEFINED.c_str(), errMQTT);
                                      SVAL(topBLUFIELD_UNDEFINED, valBLUFIELD_UNDEFINED.c_str());
                                  BLUFIELD_UNDEFINEDold = BLUFIELD_UNDEFINED;
                                }
                            break;
                          default:
                            break;
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
              String taskMessage = "loop task running on core ";
              taskMessage = taskMessage + xPortGetCoreID();
              STXT(taskMessage);
              usLast = micros();
              firstrun = false;
            }
        anzUsCycles++;
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
              SVAL(" MQTT connect... errMQTT", errMQTT);
              errMQTT = (int8_t) mqtt.connectTo(MQTT_HOST, MQTT_PORT);
              soutMQTTerr(" MQTT connect", errMQTT);
              if (errMQTT == MD_OK)
                {
                  #if (BLUETTI_TYPE == BLUETTI_AC300)
                      // INFO device offset sort
                        // DEVICE_TYPE
                          pbluetti_dev_state[DEVICE_TYPE].p_f_value = (void*) &BLUDEVICE_TYPE;
                          topBLUDEVICE_TYPE = topDevice + topBLUDEVICE_TYPE;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUDEVICE_TYPE.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_DEV_TYPE", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_DEV_TYPE", errMQTT, (uint32_t) &BLUDEVICE_TYPE, (uint32_t) &(pbluetti_dev_state[DEVICE_TYPE]).p_f_value);

                        // SERIAL_NUMBER
                          pbluetti_dev_state[SERIAL_NUMBER].p_f_value = (void*) &BLUSERIAL_NUMBER;
                          topBLUSERIAL_NUMBER = topDevice + topBLUSERIAL_NUMBER;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUSERIAL_NUMBER.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_SER_NUM", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_SER_NUM", errMQTT, (uint32_t) &BLUSERIAL_NUMBER, (uint32_t) &(pbluetti_dev_state[SERIAL_NUMBER]).p_f_value);

                        // ARM_VERSION
                          pbluetti_dev_state[ARM_VERSION].p_f_value = (void*) &BLUARM_VERSION;
                          topBLUARM_VERSION = topDevice + topBLUARM_VERSION;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUARM_VERSION.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_ARM_VERS", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_ARM_VERS", errMQTT, (uint32_t) &BLUARM_VERSION, (uint32_t) &(pbluetti_dev_state[ARM_VERSION]).p_f_value);

                        // DSP_VERSION
                          pbluetti_dev_state[DSP_VERSION].p_f_value = (void*) &BLUDSP_VERSION;
                          topBLUDSP_VERSION = topDevice + topBLUDSP_VERSION;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUDSP_VERSION.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_DSP_VERS", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_DSP_VERS", errMQTT, (uint32_t) &BLUDSP_VERSION, (uint32_t) &(pbluetti_dev_state[DSP_VERSION]).p_f_value);

                        // DC_INPUT_POWER
                          pbluetti_dev_state[DC_INPUT_POWER].p_f_value = (void*) &BLUDC_INPUT_POWER;
                          topBLUDC_INPUT_POWER = topDevice + topBLUDC_INPUT_POWER;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUDC_INPUT_POWER.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_DCIN_POW", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_DCIN_POW", errMQTT, (uint32_t) &BLUDC_INPUT_POWER, (uint32_t) &(pbluetti_dev_state[DC_INPUT_POWER]).p_f_value);

                        // AC_INPUT_POWER
                          pbluetti_dev_state[AC_INPUT_POWER].p_f_value = (void*) &BLUAC_INPUT_POWER;
                          topBLUAC_INPUT_POWER = topDevice + topBLUAC_INPUT_POWER;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUAC_INPUT_POWER.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_ACIN_POW", errMQTT);
                              S3HEXVAL(" MQTT subscribe MQTT_BLU_ACIN_POW", errMQTT, (uint32_t) &BLUAC_INPUT_POWER, (uint32_t) &(pbluetti_dev_state[AC_INPUT_POWER]).p_f_value);

                        // AC_OUTPUT_POWER
                          (pbluetti_dev_state[AC_OUTPUT_POWER]).p_f_value = (void*) &BLUAC_OUTPUT_POWER;
                          topBLUDC_OUTPUT_POWER = topDevice + topBLUDC_OUTPUT_POWER;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUDC_OUTPUT_POWER.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_ACOUT_POW", errMQTT);
                              //S3HEXVAL(" MQTT subscribe MQTT_BLU_ACOUT_POW", errMQTT, (uint32_t) &BLUAC_OUTPUT_POWER, (uint32_t) &(pbluetti_dev_state[AC_OUTPUT_POWER]).p_f_value);

                        // DC_OUTPUT_POWER
                          //S4HEXVAL(" conn MQTT idx pblu pblu[idx] size ", DC_OUTPUT_POWER, (uint32_t) pbluetti_dev_state,
                          //                                              (uint32_t) &(pbluetti_dev_state[DC_OUTPUT_POWER]), sizeof(device_field_data_t) );
                          pbluetti_dev_state[DC_OUTPUT_POWER].p_f_value = (void*) &BLUDC_OUTPUT_POWER;
                          topBLUDC_OUTPUT_ON = topDevice + topBLUDC_OUTPUT_ON;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUDC_OUTPUT_ON.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_DCOUT_POW", errMQTT);
                              //S3HEXVAL(" MQTT subscribe MQTT_BLU_DCOUT_POW", DC_OUTPUT_POWER, (uint32_t) &BLUDC_OUTPUT_POWER, (uint32_t) &(pbluetti_dev_state[DC_OUTPUT_POWER].p_f_value));

                        // POWER_GENERATION
                          pbluetti_dev_state[POWER_GENERATION].p_f_value = (void*) &BLUAC_OUTPUT_POWER;
                          topBLUPOWER_GENERATION = topDevice + topBLUPOWER_GENERATION;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUPOWER_GENERATION.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_POWER_GENERATION", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_POWER_GENERATION", errMQTT, (uint32_t) &BLUPOWER_GENERATION, (uint32_t) &(pbluetti_dev_state[POWER_GENERATION]).p_f_value);

                        // TOTAL_BATT_PERC
                          pbluetti_dev_state[TOTAL_BATT_PERC].p_f_value = (void*) &BLUTOTAL_BATT_PERC;
                          topBLUTOTAL_BATT_PERC = topDevice + topBLUTOTAL_BATT_PERC;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUTOTAL_BATT_PERC.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_TOTAL_BATT_PERC", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_TOTAL_BATT_PERC", errMQTT, (uint32_t) &BLUTOTAL_BATT_PERC, (uint32_t) &(pbluetti_dev_state[TOTAL_BATT_PERC]).p_f_value);

                        // AC_OUTPUT_ON
                          //S4HEXVAL(" conn MQTT idx pblu pblu[idx] size ", AC_OUTPUT_ON, (uint32_t) pbluetti_dev_state,
                          //                                              (uint32_t) &(pbluetti_dev_state[AC_OUTPUT_ON]), sizeof(device_field_data_t) );
                          pbluetti_dev_state[AC_OUTPUT_ON].p_f_value = (void*) &BLUARM_VERSION;
                          topBLUAC_OUTPUT_POWER = topDevice + topBLUAC_OUTPUT_POWER;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUAC_OUTPUT_POWER.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_AC_OUTPUT_ON", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_AC_OUTPUT_ON", errMQTT, (uint32_t) &BLUAC_OUTPUT_ON, (uint32_t) &(pbluetti_dev_state[AC_OUTPUT_ON]).p_f_value);

                        // DC_OUTPUT_ON
                          pbluetti_dev_state[DC_OUTPUT_ON].p_f_value = (void*) &BLUDC_OUTPUT_ON;
                          topBLUDC_OUTPUT_ON = topDevice + topBLUDC_OUTPUT_ON;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUDC_OUTPUT_ON.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_DC_OUTPUT_ON", errMQTT);
                              //S3HEXVAL(" MQTT subscribe MQTT_BLU_DC_OUTPUT_ON", DC_OUTPUT_ON, (uint32_t) &BLUDC_OUTPUT_ON, (uint32_t) &(pbluetti_dev_state[DC_OUTPUT_ON]).p_f_value);

                      // INFO internal
                        // AC_OUTPUT_MODE
                          pbluetti_dev_state[AC_OUTPUT_MODE].p_f_value = (void*) &BLUAC_OUTPUT_MODE;
                          topBLUAC_OUTPUT_MODE = topDevice + topBLUAC_OUTPUT_MODE;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUAC_OUTPUT_MODE.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLUAC_OUTPUT_MODE", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLUAC_OUTPUT_MODE", errMQTT, (uint32_t) &BLUAC_OUTPUT_MODE, (uint32_t) &(pbluetti_dev_state[BLUAC_OUTPUT_MODE]).p_f_value);

                        // INTERN_AC_VOLT
                          pbluetti_dev_state[INTERN_AC_VOLT].p_f_value = (void*) &BLUINTERN_AC_VOLT;
                          topBLUINTERN_AC_VOLT = topDevice + topBLUINTERN_AC_VOLT;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINTERN_AC_VOLT.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_AC_VOLT", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_INT_AC_VOLT", errMQTT, (uint32_t) &BLUINTERN_AC_VOLT, (uint32_t) &(pbluetti_dev_state[INTERN_AC_VOLT]).p_f_value);

                        // INTERN_CURR_1
                          pbluetti_dev_state[INTERN_CURR_1].p_f_value = (void*) &BLUINTERN_CURR_1;
                          topBLUINTERN_CURR_1 = topDevice + topBLUINTERN_CURR_1;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINTERN_CURR_1.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_CURR1", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_INT_CURR1", errMQTT, (uint32_t) &BLUINTERN_CURR_1, (uint32_t) &(pbluetti_dev_state[INTERN_CURR_1]).p_f_value);

                        // INTERN_POWER_1
                          pbluetti_dev_state[INTERN_POWER_1].p_f_value = (void*) &BLUINTERN_POWER_1;
                          topBLUINTERN_POWER_1 = topDevice + topBLUINTERN_POWER_1;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINTERN_POWER_1.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_INTERN_POWER_1", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTTINTERN_POWER_1", errMQTT, (uint32_t) &BLUINTERN_POWER_1, (uint32_t) &(pbluetti_dev_state[INTERN_POWER_1]).p_f_value);

                        // INTERN_AC_FREQ
                          pbluetti_dev_state[INTERN_AC_FREQ].p_f_value = (void*) &BLUINTERN_AC_FREQ;
                          topBLUINTERN_AC_FREQ = topDevice + topBLUINTERN_AC_FREQ;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINTERN_AC_FREQ.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLUINTERN_AC_FREQ", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLUINTERN_AC_FREQ", errMQTT, (uint32_t) &BLUINTERN_AC_FREQ, (uint32_t) &(pbluetti_dev_state[INTERN_AC_FREQ]).p_f_value);

                        // INTERN_CURR_2
                          pbluetti_dev_state[INTERN_CURR_2].p_f_value = (void*) &BLUINTERN_CURR_2;
                          topBLUINTERN_CURR_2 = topDevice + topBLUINTERN_CURR_2;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINTERN_CURR_2.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_CURR2", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_INT_CURR2", errMQTT, (uint32_t) &BLUINTERN_CURR_2, (uint32_t) &(pbluetti_dev_state[INTERN_CURR_2]).p_f_value);

                        // INTERN_POWER_2
                          pbluetti_dev_state[INTERN_POWER_2].p_f_value = (void*) &BLUINTERN_POWER_2;
                          topBLUINTERN_POWER_2 = topDevice + topBLUINTERN_POWER_2;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINTERN_POWER_2.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_INTERN_POWER_2", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_INTERN_POWER_2", errMQTT, (uint32_t) &BLUINTERN_POWER_2, (uint32_t) &(pbluetti_dev_state[INTERN_POWER_2]).p_f_value);

                        // AC_INPUT_VOLT
                          pbluetti_dev_state[AC_INPUT_VOLT].p_f_value = (void*) &BLUAC_INPUT_VOLT;
                          topBLUAC_INPUT_VOLT = topDevice + topBLUAC_INPUT_VOLT;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUAC_INPUT_VOLT.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_AC_INPUT_VOLT", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_AC_INPUT_VOLT", errMQTT, (uint32_t) &BLUAC_INPUT_VOLT, (uint32_t) &(pbluetti_dev_state[AC_INPUT_VOLT]).p_f_value);

                        // INTERN_CURR_3
                          pbluetti_dev_state[INTERN_CURR_3].p_f_value = (void*) &BLUINTERN_CURR_3;
                          topBLUINTERN_CURR_3 = topDevice + topBLUINTERN_CURR_3;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINTERN_CURR_3.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_INTERN_CURR_3", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_INTERN_CURR_3", errMQTT, (uint32_t) &BLUINTERN_CURR_3, (uint32_t) &(pbluetti_dev_state[INTERN_CURR_3]).p_f_value);

                        // INTERN_POWER_3
                          pbluetti_dev_state[INTERN_POWER_3].p_f_value = (void*) &BLUINTERN_POWER_3;
                          topBLUINTERN_POWER_3 = topDevice + topBLUINTERN_POWER_3;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINTERN_POWER_3.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_INTERN_POWER_3", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_INTERN_POWER_3", errMQTT, (uint32_t) &BLUINTERN_POWER_3, (uint32_t) &(pbluetti_dev_state[INTERN_POWER_3]).p_f_value);

                        // AC_INPUT_FREQ
                          pbluetti_dev_state[AC_INPUT_FREQ].p_f_value = (void*) &BLUAC_INPUT_FREQ;
                          topBLUAC_INPUT_FREQ = topDevice + topBLUAC_INPUT_FREQ;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUAC_INPUT_FREQ.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_AC_INPUT_FREQ", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_AC_INPUT_FREQ", errMQTT, (uint32_t) &BLUAC_INPUT_FREQ, (uint32_t) &(pbluetti_dev_state[AC_INPUT_FREQ]).p_f_value);

                        // INT_DC_INP_VOLT
                          pbluetti_dev_state[INT_DC_INP_VOLT].p_f_value = (void*) &BLUINT_DC_INP_VOLT;
                          topBLUINT_DC_INP_VOLT = topDevice + topBLUINT_DC_INP_VOLT;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINT_DC_INP_VOLT.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_DC_INP_VOLT", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_INT_DC_INP_VOLT", errMQTT, (uint32_t) &BLUINT_DC_INP_VOLT, (uint32_t) &(pbluetti_dev_state[INT_DC_INP_VOLT]).p_f_value);

                        // INT_DC_INP_POW
                          pbluetti_dev_state[INT_DC_INP_POW].p_f_value = (void*) &BLUINT_DC_INP_POW;
                          topBLUINT_DC_INP_POW = topDevice + topBLUINT_DC_INP_POW;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINT_DC_INP_POW.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_DC_INP_POW", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_INT_DC_INP_POW", errMQTT, (uint32_t) &BLUINT_DC_INP_POW, (uint32_t) &(pbluetti_dev_state[INT_DC_INP_POW]).p_f_value);

                        // INT_DC_INP_CURR
                          pbluetti_dev_state[INT_DC_INP_CURR].p_f_value = (void*) &BLUINT_DC_INP_CURR;
                          topBLUINT_DC_INP_CURR = topDevice + topBLUINT_DC_INP_CURR;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUINT_DC_INP_CURR.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_DC_INP_CURR", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_INT_DC_INP_CURR", errMQTT, (uint32_t) &BLUINT_DC_INP_CURR, (uint32_t) &(pbluetti_dev_state[INT_DC_INP_CURR]).p_f_value);

                        // PACK_NUM_MAX
                          pbluetti_dev_state[PACK_NUM_MAX].p_f_value = (void*) &BLUPACK_NUM_MAX;
                          topBLUPACK_NUM_MAX = topDevice + topBLUPACK_NUM_MAX;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUPACK_NUM_MAX.c_str());
                              soutMQTTerr(" MQTT subscribe PACK_NUM_MAX", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe PACK_NUM_MAX", errMQTT, (uint32_t) &BLUPACK_NUM_MAX, (uint32_t) &(pbluetti_dev_state[PACK_NUM_MAX]).p_f_value);

                        // TOTAL_BATT_VOLT
                          pbluetti_dev_state[TOTAL_BATT_VOLT].p_f_value = (void*) &BLUTOTAL_BATT_VOLT;
                          topBLUTOTAL_BATT_VOLT = topDevice + topBLUTOTAL_BATT_VOLT;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUTOTAL_BATT_VOLT.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_TOTAL_BATT_VOLT", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_TOTAL_BATT_VOLT", errMQTT, (uint32_t) &BLUTOTAL_BATT_VOLT, (uint32_t) &(pbluetti_dev_state[TOTAL_BATT_VOLT]).p_f_value);

                        // TOTAL_BATT_CURR
                          pbluetti_dev_state[TOTAL_BATT_CURR].p_f_value = (void*) &BLUTOTAL_BATT_CURR;
                          topBLUTOTAL_BATT_CURR = topDevice + topBLUTOTAL_BATT_CURR;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUTOTAL_BATT_CURR.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_TOTAL_BATT_CURR", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_TOTAL_BATT_CURR", errMQTT, (uint32_t) &BLUTOTAL_BATT_CURR, (uint32_t) &(pbluetti_dev_state[TOTAL_BATT_CURR]).p_f_value);

                        // PACK_NUM
                          pbluetti_dev_state[PACK_NUM].p_f_value = (void*) &BLUPACK_NUM;
                          topBLUPACK_NUM = topDevice + topBLUPACK_NUM;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUPACK_NUM.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_PACK_NUM", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_PACK_NUM", errMQTT, (uint32_t) &BLUPACK_NUM, (uint32_t) &(pbluetti_dev_state[PACK_NUM]).p_f_value);

                        // PACK_STATUS
                          pbluetti_dev_state[PACK_STATUS].p_f_value = (void*) &BLUPACK_STATUS;
                          topBLUPACK_STATUS = topDevice + topBLUPACK_STATUS;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUPACK_STATUS.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_PACK_STATUS", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_PACK_STATUS", errMQTT, (uint32_t) &BLUPACK_STATUS, (uint32_t) &(pbluetti_dev_state[PACK_STATUS]).p_f_value);

                        // PACK_VOLTAGE
                          pbluetti_dev_state[PACK_VOLTAGE].p_f_value = (void*) &BLUPACK_VOLTAGE;
                          topBLUPACK_VOLTAGE = topDevice + topBLUPACK_VOLTAGE;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUPACK_VOLTAGE.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_PACK_U", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_PACK_U", errMQTT, (uint32_t) &BLUPACK_VOLTAGE, (uint32_t) &(pbluetti_dev_state[PACK_VOLTAGE]).p_f_value);

                        // PACK_BATT_PERC
                          pbluetti_dev_state[PACK_BATT_PERC].p_f_value = (void*) &BLUPACK_BATT_PERC;
                          topBLUPACK_BATT_PERC = topDevice + topBLUPACK_BATT_PERC;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUPACK_BATT_PERC.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_PACK_BATT_PERC", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_PACK_BATT_PERC", errMQTT, (uint32_t) &BLUPACK_BATT_PERC, (uint32_t) &(pbluetti_dev_state[PACK_BATT_PERC]).p_f_value);

                        // CELL_VOTAGES
                          /*
                            pbluetti_dev_state[CELL_VOTAGES].p_f_value = (void*) &BLUCELL_VOTAGES;
                            topBLUCELL_VOTAGES = topDevice + topBLUCELL_VOTAGES;
                            errMQTT = (int8_t) mqtt.subscribe(topBLUCELL_VOTAGES.c_str());
                                soutMQTTerr(" MQTT subscribe MQTT_BLU_CELL_VOTAGES", errMQTT);
                            //    S3HEXVAL(" MQTT subscribe MQTT_BLU_CELL_VOTAGES", errMQTT, (uint32_t) &BLUCELL_VOTAGES, (uint32_t) &(pbluetti_dev_state[CELL_VOTAGES]).p_f_value);
                           */
                        // PACK_BMS_VERSION
                          pbluetti_dev_state[PACK_BMS_VERSION].p_f_value = (void*) &BLUPACK_BMS_VERSION;
                          topBLUPACK_BMS_VERSION = topDevice + topBLUPACK_BMS_VERSION;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUPACK_BMS_VERSION.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_PACK_BMS_VERSION", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_PACK_BMS_VERSION", errMQTT, (uint32_t) &BLUPACK_BMS_VERSION, (uint32_t) &(pbluetti_dev_state[PACK_BMS_VERSION]).p_f_value);

                      // CONTROL elements
                        // UPS_MODE
                          pbluetti_dev_state[UPS_MODE].p_f_value = (void*) &BLUUPS_MODE;
                          topBLUUPS_MODE = topDevice + topBLUUPS_MODE;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUUPS_MODE.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_UPS_MODE", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_UPS_MODE", errMQTT, (uint32_t) &BLUUPS_MODE, (uint32_t) &(pbluetti_dev_state[UPS_MODE]).p_f_value);

                        // SPLIT_PHASE_ON
                          pbluetti_dev_state[SPLIT_PHASE_ON].p_f_value = (void*) &BLUSPLIT_PHASE_ON;
                          topBLUSPLIT_PHASE_ON = topDevice + topBLUSPLIT_PHASE_ON;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUSPLIT_PHASE_ON.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_UPS_MODESPLIT_PHASE_ON", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_SPLIT_PHASE_ON", errMQTT, (uint32_t) &BLUSPLIT_PHASE_ON, (uint32_t) &(pbluetti_dev_state[SPLIT_PHASE_ON]).p_f_value);

                        // SPLIT_PH_MACH_MODE
                          pbluetti_dev_state[SPLIT_PH_MACH_MODE].p_f_value = (void*) &BLUSPLIT_PH_MACH_MODE;
                          topBLUSPLIT_PH_MACH_MODE = topDevice + topBLUSPLIT_PH_MACH_MODE;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUUPS_MODE.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_SPLIT_PH_MACH_MODE", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_SPLIT_PH_MACH_MODE", errMQTT, (uint32_t) &BLUSPLIT_PH_MACH_MODE, (uint32_t) &(pbluetti_dev_state[SPLIT_PH_MACH_MODE]).p_f_value);

                        // SET_PACK_NUM
                          pbluetti_dev_state[SET_PACK_NUM].p_f_value = (void*) &BLUSET_PACK_NUM;
                          topBLUSET_PACK_NUM = topDevice + topBLUSET_PACK_NUM;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUSET_PACK_NUM.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_SET_PACK_NUM", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_SET_PACK_NUM", errMQTT, (uint32_t) &BLUSET_PACK_NUM, (uint32_t) &(pbluetti_dev_state[SET_PACK_NUM]).p_f_value);

                        // SET_AC_OUT_ON
                          pbluetti_dev_state[SET_AC_OUT_ON].p_f_value = (void*) &BLUSET_AC_OUT_ON;
                          topBLUSET_AC_OUT_ON = topDevice + topBLUSET_AC_OUT_ON;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUSET_AC_OUT_ON.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_SET_AC_OUT_ON", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_SET_AC_OUT_ON", errMQTT, (uint32_t) &BLUSET_AC_OUT_ON, (uint32_t) &(pbluetti_dev_state[SET_AC_OUT_ON]).p_f_value);

                        // SET_DC_OUT_ON
                          pbluetti_dev_state[SET_DC_OUT_ON].p_f_value = (void*) &BLUSET_DC_OUT_ON;
                          topBLUSET_DC_OUT_ON = topDevice + topBLUSET_DC_OUT_ON;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUUPS_MODE.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_UPS_MODE", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_SET_DC_OUT_ON", errMQTT, (uint32_t) &BLUSET_DC_OUT_ON, (uint32_t) &(pbluetti_dev_state[SET_DC_OUT_ON]).p_f_value);

                        // GRID_CHANGE_ON
                          pbluetti_dev_state[GRID_CHANGE_ON].p_f_value = (void*) &BLUGRID_CHANGE_ON;
                          topBLUGRID_CHANGE_ON = topDevice + topBLUGRID_CHANGE_ON;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUGRID_CHANGE_ON.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_GRID_CHANGE_ON", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_GRID_CHANGE_ON", errMQTT, (uint32_t) &BLUGRID_CHANGE_ON, (uint32_t) &(pbluetti_dev_state[GRID_CHANGE_ON]).p_f_value);

                        // TIME_CTRL_ON
                          pbluetti_dev_state[TIME_CTRL_ON].p_f_value = (void*) &BLUTIME_CTRL_ON;
                          topBLUTIME_CTRL_ON = topDevice + topBLUTIME_CTRL_ON;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUTIME_CTRL_ON.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_TIME_CTRL_ON", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_TIME_CTRL_ON", errMQTT, (uint32_t) &BLUTIME_CTRL_ON, (uint32_t) &(pbluetti_dev_state[TIME_CTRL_ON]).p_f_value);

                        // BATT_RANGE_START
                          pbluetti_dev_state[BATT_RANGE_START].p_f_value = (void*) &BLUBATT_RANGE_START;
                          topBLUBATT_RANGE_START = topDevice + topBLUBATT_RANGE_START;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUBATT_RANGE_START.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_BATT_RANGE_START", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_BATT_RANGE_START", errMQTT, (uint32_t) &BLUBATT_RANGE_START, (uint32_t) &(pbluetti_dev_state[BATT_RANGE_START]).p_f_value);

                        // BATT_RANGE_END
                          pbluetti_dev_state[BATT_RANGE_END].p_f_value = (void*) &BLUBATT_RANGE_END;
                          topBLUBATT_RANGE_END = topDevice + topBLUBATT_RANGE_END;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUBATT_RANGE_END.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_BATT_RANGE_END", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_BATT_RANGE_END", errMQTT, (uint32_t) &BLUBATT_RANGE_END, (uint32_t) &(pbluetti_dev_state[BATT_RANGE_END]).p_f_value);

                        // BLUETOOTH_CONN
                          pbluetti_dev_state[BLUETOOTH_CONN].p_f_value = (void*) &BLUBLUETOOTH_CONN;
                          topBLUBLUETOOTH_CONN = topDevice + topBLUBLUETOOTH_CONN;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUBLUETOOTH_CONN.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_BLUETOOTH_CONN", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_BLUETOOTH_CONN", errMQTT, (uint32_t) &BLBLUETOOTH_CONN, (uint32_t) &(pbluetti_dev_state[BLUETOOTH_CONN]).p_f_value);

                        // AUTO_SLEEP_MODE
                          pbluetti_dev_state[AUTO_SLEEP_MODE].p_f_value = (void*) &BLUAUTO_SLEEP_MODE;
                          topBLUAUTO_SLEEP_MODE = topDevice + topBLUAUTO_SLEEP_MODE;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUAUTO_SLEEP_MODE.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_AUTO_SLEEP_MODE", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_AUTO_SLEEP_MODE", errMQTT, (uint32_t) &BLUAUTO_SLEEP_MODE, (uint32_t) &(pbluetti_dev_state[AUTO_SLEEP_MODE]).p_f_value);

                        // LED_CONTROL
                          pbluetti_dev_state[LED_CONTROL].p_f_value = (void*) &BLULED_CONTROL;
                          topBLULED_CONTROL = topDevice + topBLULED_CONTROL;
                          errMQTT = (int8_t) mqtt.subscribe(topBLULED_CONTROL.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_LED_CONTROL", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_LED_CONTROL", errMQTT, (uint32_t) &BLULED_CONTROL, (uint32_t) &(pbluetti_dev_state[LED_CONTROL]).p_f_value);
                        // FIELD_UNDEFINED
                          pbluetti_dev_state[FIELD_UNDEFINED].p_f_value = (void*) &BLUFIELD_UNDEFINED;
                          topBLUFIELD_UNDEFINED = topDevice + topBLUFIELD_UNDEFINED;
                          errMQTT = (int8_t) mqtt.subscribe(topBLUFIELD_UNDEFINED.c_str());
                              soutMQTTerr(" MQTT subscribe MQTT_BLU_FIELD_UNDEFINED", errMQTT);
                          //    S3HEXVAL(" MQTT subscribe MQTT_BLU_FIELD_UNDEFINED", errMQTT, (uint32_t) &BLUFIELD_UNDEFINED, (uint32_t) &(pbluetti_dev_state[FIELD_UNDEFINED]).p_f_value);
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
              if (errMQTT == MD_OK)
                {
                  errMQTTold = MD_OK;
                }
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

