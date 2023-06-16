
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
      static bool      BLUacouton     = 0;
      static bool      BLUdcouton     = 0;
      static uint16_t  BLUdcoutp      = 0;
      static uint16_t  BLUacoutp      = 0;
      static float     BLUpowgen      = 0.0;
      static uint16_t  BLUtotbatt     = 0;
      static uint16_t  BLUdcinp       = 0;
      static uint16_t  BLUacinp       = 0;
      static float     BLUpaku        = 0.0;
      static String    BLUsernum      = "";
      static float     BLUarmvers     = 0.0;
      static float     BLUdspvers     = 0.0;
      static String    BLUdevtype     = "";
      static float     BLUintacu      = 0.0;
      static float     BLUintcurr1    = 0.0;
      static uint16_t  BLUpackmax     = 0;
      static uint16_t  BLUupsmode     = 0;
      static uint16_t  BLUslpmode     = 0;
      static bool      BLUgrdchon     = 0;
      static uint16_t  BLUfieldund    = 0;
    // --- old value
      static bool      BLUacoutonold  = 1000;
      static bool      BLUdcoutonold  = 1000;
      static uint16_t  BLUdcoutpold   = 1000;
      static uint16_t  BLUacoutpold   = 1000;
      static float     BLUpowgenold   = 1000.0;
      static uint16_t  BLUtotbattold  = 1000;
      static uint16_t  BLUdcinpold    = 1000;
      static uint16_t  BLUacinpold    = 1000;
      static float     BLUpakuold     = 1000.0;
      static String    BLUsernumold   = "---";
      static float     BLUarmversold  = 1000.0;
      static float     BLUdspversold  = 1000.0;
      static String    BLUdevtypeold  = "---";
      static float     BLUintacuold   = 1000.0;
      static float     BLUintcurr1old = 1000.0;
      static uint16_t  BLUpackmaxold  = 1000;
      static uint16_t  BLUupsmodeold  = 1000;
      static uint16_t  BLUslpmodeold  = 1000;
      static bool      BLUgrdchonold  = 1000;
      static uint16_t  BLUfieldundold = 1000;
    // --- MQTT value as string
      static String    valBLUacouton  = "";
      static String    valBLUdcouton  = "";
      static String    valBLUdcoutp   = "";
      static String    valBLUacoutp   = "";
      static String    valBLUpowgen   = "";
      static String    valBLUtotbatt  = "";
      static String    valBLUdcinp    = "";
      static String    valBLUacinp    = "";
      static String    valBLUpaku     = "";
      static String    valBLUsernum   = "";
      static String    valBLUarmvers  = "";
      static String    valBLUdspvers  = "";
      static String    valBLUdevtype  = "";
      static String    valBLUintacu   = "";
      static String    valBLUintcurr1 = "";
      static String    valBLUpackmax  = "";
      static String    valBLUupsmode  = "";
      static String    valBLUslpmode  = "";
      static String    valBLUgrdchon  = "";
      static String    valBLUfieldund = "";
    // --- MQTT publish flag
      static int8_t    pubBLUacouton  = 0;
      static int8_t    pubBLUdcouton  = 0;
      static int8_t    pubBLUdcoutp   = 0;
      static int8_t    pubBLUacoutp   = 0;
      static int8_t    pubBLUpowgen   = 0;
      static int8_t    pubBLUtotbatt  = 0;
      static int8_t    pubBLUdcinp    = 0;
      static int8_t    pubBLUacinp    = 0;
      static int8_t    pubBLUpaku     = 0;
      static int8_t    pubBLUsernum   = 0;
      static int8_t    pubBLUarmvers  = 0;
      static int8_t    pubBLUdspvers  = 0;
      static int8_t    pubBLUdevtype  = 0;
      static int8_t    pubBLUupsmode  = 0;
      static int8_t    pubBLUslpmode  = 0;
      static int8_t    pubBLUgrdchon  = 0;
      static int8_t    pubBLUfieldund = 0;
      static int8_t    pubBLUintacu   = 0;
      static int8_t    pubBLUintcurr1 = 0;
      static int8_t    pubBLUpackmax  = 0;
    #if (USE_MQTT > OFF) // MQTT topic
        static String topBLUdcoutp   = DEVICE_F_NAMES[DC_OUTPUT_ON]; // "dcoutp"
        static String topBLUdcouton  = DEVICE_F_NAMES[AC_OUTPUT_ON] ; // "dcouton"
        static String topBLUacoutp   = DEVICE_F_NAMES[DC_OUTPUT_POWER]; // "acoutp"
        static String topBLUacouton  = DEVICE_F_NAMES[AC_OUTPUT_POWER] ; // "acouton"
        static String topBLUpowgen   = DEVICE_F_NAMES[POWER_GENERATION]  ; // "powgen"
        static String topBLUtotbatt  = DEVICE_F_NAMES[TOTAL_BATTERY_PERCENT] ; // "totbatt"
        static String topBLUdcinp    = DEVICE_F_NAMES[DC_INPUT_POWER] ; // "dcinp"
        static String topBLUacinp    = DEVICE_F_NAMES[AC_INPUT_POWER] ; // "acinp"
        static String topBLUpaku     = DEVICE_F_NAMES[PACK_VOLTAGE]   ; // "paku"
        static String topBLUsernum   = DEVICE_F_NAMES[SERIAL_NUMBER]  ; // "sernum"
        static String topBLUarmvers  = DEVICE_F_NAMES[ARM_VERSION] ; // "armvers"
        static String topBLUdspvers  = DEVICE_F_NAMES[DSP_VERSION] ; // "dspvers"
        static String topBLUdevtype  = DEVICE_F_NAMES[DEVICE_TYPE] ; // "devtype"
        static String topBLUintacu   = DEVICE_F_NAMES[INTERNAL_AC_VOLTAGE] ; // "intacu"
        static String topBLUintcurr1 = DEVICE_F_NAMES[INTERNAL_CURRENT_ONE]; // "intcurr1"
        static String topBLUpackmax  = DEVICE_F_NAMES[PACK_NUM_MAX] ; // "packmax"
        static String topBLUupsmode  = DEVICE_F_NAMES[UPS_MODE] ; // "upsmode"
        static String topBLUslpmode  = DEVICE_F_NAMES[AUTO_SLEEP_MODE] ; // "sleepm"
        static String topBLUgrdchon  = DEVICE_F_NAMES[GRID_CHANGE_ON]; // "grdchon"
        static String topBLUfieldund = DEVICE_F_NAMES[FIELD_UNDEFINED]; // "fldundef"
      #endif
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
                outpIdx++;
                outpT.startT();
                //SVAL(" outputcycle idx ", outpIdx);
                switch(outpIdx)
                  {
                    case 1:  // DC_OUTPU_POWER
                        if (BLUdcoutp != BLUdcoutpold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUdcoutp = BLUdcoutp;
                                errMQTT = (int8_t) mqtt.publish(topBLUdcoutp.c_str(), (uint8_t*) valBLUdcoutp.c_str(), valBLUdcoutp.length());
                                soutMQTTerr(valBLUdcoutp.c_str(), errMQTT);
                                    SVAL(topBLUdcoutp, valBLUdcoutp.c_str());
                              }
                            BLUdcoutpold = BLUdcoutp;
                          }
                      break;
                    case 2:  // DC_OUTPUT_ON
                        if (BLUdcouton != BLUdcoutonold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUdcouton = BLUdcouton;
                                errMQTT = (int8_t) mqtt.publish(topBLUdcouton.c_str(), (uint8_t*) valBLUdcouton.c_str(), valBLUdcouton.length());
                                soutMQTTerr(valBLUdcouton.c_str(), errMQTT);
                                    SVAL(topBLUdcouton, valBLUdcouton.c_str());
                              }
                            BLUdcoutonold = BLUdcouton;
                          }
                      break;
                    case 3:  // AC_OUTPUT_POWER
                        if (BLUacoutp != BLUacoutpold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUdcoutp = BLUdcoutp;
                                errMQTT = (int8_t) mqtt.publish(topBLUacoutp.c_str(), (uint8_t*) valBLUacoutp.c_str(), valBLUacoutp.length());
                                soutMQTTerr(valBLUacoutp.c_str(), errMQTT);
                                    SVAL(topBLUacoutp, valBLUacoutp.c_str());
                              }
                            BLUacoutpold = BLUacoutp;
                          }
                      break;
                    case 4:  // AC_OUTPUT_ON
                        if (BLUacoutp != BLUacoutonold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUacouton = BLUacoutonold;
                                errMQTT = (int8_t) mqtt.publish(topBLUacouton.c_str(), (uint8_t*) valBLUacouton.c_str(), valBLUacouton.length());
                                soutMQTTerr(valBLUacouton.c_str(), errMQTT);
                                    SVAL(topBLUacouton, valBLUacouton.c_str());
                              }
                            BLUacoutonold = BLUacouton;
                          }
                      break;
                    case 5:  // POWER_GENERATION
                        if (BLUpowgen != BLUpowgenold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUpowgen = BLUpowgen;
                                errMQTT = (int8_t) mqtt.publish(topBLUpowgen.c_str(), (uint8_t*) valBLUpowgen.c_str(), valBLUpowgen.length());
                                soutMQTTerr(valBLUpowgen.c_str(), errMQTT);
                                    SVAL(topBLUpowgen, valBLUpowgen.c_str());
                              }
                            BLUpowgenold = BLUpowgen;
                          }
                      break;
                    case 6:  // TOTAL_BATTERY_PERCENT
                        if (BLUtotbatt != BLUtotbattold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUtotbatt = BLUtotbatt;
                                errMQTT = (int8_t) mqtt.publish(topBLUtotbatt.c_str(), (uint8_t*) valBLUtotbatt.c_str(), valBLUtotbatt.length());
                                soutMQTTerr(valBLUtotbatt.c_str(), errMQTT);
                                    SVAL(topBLUtotbatt, valBLUtotbatt.c_str());
                              }
                            BLUtotbattold = BLUtotbatt;
                          }
                      break;
                    case 7:  // DC_INPUT_POWER
                        if (BLUdcinp != BLUdcinpold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUdcinp = BLUdcinp;
                                errMQTT = (int8_t) mqtt.publish(topBLUdcinp.c_str(), (uint8_t*) valBLUdcinp.c_str(), valBLUdcinp.length());
                                soutMQTTerr(valBLUdcinp.c_str(), errMQTT);
                                    SVAL(topBLUdcinp, valBLUdcinp.c_str());
                              }
                            BLUdcinpold = BLUdcinp;
                          }
                      break;
                    case 8:  // AC_INPUT_POWER
                        if (BLUacinp != BLUacinpold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUacinp = BLUacinp;
                                errMQTT = (int8_t) mqtt.publish(topBLUacinp.c_str(), (uint8_t*) valBLUacinp.c_str(), valBLUacinp.length());
                                soutMQTTerr(valBLUacinp.c_str(), errMQTT);
                                    SVAL(topBLUacinp, valBLUacinp.c_str());
                              }
                            BLUacinpold = BLUacinp;
                          }
                      break;
                    case 9:  // PACK_VOLTAGE
                        if (BLUpaku != BLUpakuold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUpaku = BLUpaku;
                                errMQTT = (int8_t) mqtt.publish(topBLUpaku.c_str(), (uint8_t*) valBLUpaku.c_str(), valBLUpaku.length());
                                soutMQTTerr(valBLUpaku.c_str(), errMQTT);
                                    SVAL(topBLUpaku, valBLUpaku.c_str());
                              }
                            BLUpakuold = BLUpaku;
                          }
                      break;
                    case 10:  // SERIAL_NUMBER
                        if (BLUsernum != BLUsernumold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUsernum = BLUsernum;
                                errMQTT = (int8_t) mqtt.publish(topBLUsernum.c_str(), (uint8_t*) valBLUsernum.c_str(), valBLUsernum.length());
                                soutMQTTerr(valBLUsernum.c_str(), errMQTT);
                                    SVAL(topBLUsernum, valBLUsernum.c_str());
                              }
                            BLUsernumold = BLUsernum;
                          }
                      break;
                    case 11:  // ARM_VERSION
                        if (BLUarmvers != BLUarmversold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUarmvers = BLUarmvers;
                                errMQTT = (int8_t) mqtt.publish(topBLUarmvers.c_str(), (uint8_t*) valBLUarmvers.c_str(), valBLUarmvers.length());
                                soutMQTTerr(valBLUdcoutp.c_str(), errMQTT);
                                    SVAL(topBLUarmvers, valBLUarmvers.c_str());
                              }
                            BLUarmversold = BLUarmvers;
                          }
                      break;
                    case 12:  // DSP_VERSION
                        if (BLUdspvers != BLUdspvers)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUdspvers = BLUdspvers;
                                errMQTT = (int8_t) mqtt.publish(topBLUdspvers.c_str(), (uint8_t*) valBLUdspvers.c_str(), valBLUdspvers.length());
                                soutMQTTerr(valBLUdcoutp.c_str(), errMQTT);
                                    SVAL(topBLUdspvers, valBLUdspvers.c_str());
                              }
                            BLUdspvers = BLUdspvers;
                          }
                      break;
                    case 13:  // DEVICE_TYPE
                        if (BLUdevtype != BLUdevtypeold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUdevtype = BLUdevtype;
                                errMQTT = (int8_t) mqtt.publish(topBLUdevtype.c_str(), (uint8_t*) valBLUdevtype.c_str(), valBLUdevtype.length());
                                soutMQTTerr(valBLUdevtype.c_str(), errMQTT);
                                    SVAL(topBLUdevtype, valBLUdevtype.c_str());
                              }
                            BLUdevtypeold = BLUdevtype;
                          }
                      break;
                    case 14:  // INTERNAL_AC_VOLTAGE
                        if (BLUintacu != BLUintacuold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUintacu = BLUintacu;
                                errMQTT = (int8_t) mqtt.publish(topBLUintacu.c_str(), (uint8_t*) valBLUintacu.c_str(), valBLUintacu.length());
                                soutMQTTerr(valBLUintacu.c_str(), errMQTT);
                                    SVAL(topBLUintacu, valBLUintacu.c_str());
                              }
                            BLUintacuold = BLUintacu;
                          }
                      break;
                    case 15:  // INTERNAL_CURRENT_ONE
                        if (BLUintcurr1 != BLUintcurr1old)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUintcurr1 = BLUintcurr1;
                                errMQTT = (int8_t) mqtt.publish(topBLUintcurr1.c_str(), (uint8_t*) valBLUintcurr1.c_str(), valBLUintcurr1.length());
                                soutMQTTerr(valBLUintcurr1.c_str(), errMQTT);
                                    SVAL(topBLUintcurr1, valBLUdcoutp.c_str());
                              }
                            BLUintcurr1old = BLUintcurr1;
                          }
                      break;
                    case 16:  // PACK_NUM_MAX
                        if (BLUpackmax != BLUpackmaxold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUpackmax = BLUpackmax;
                                errMQTT = (int8_t) mqtt.publish(topBLUpackmax.c_str(), (uint8_t*) valBLUpackmax.c_str(), valBLUpackmax.length());
                                soutMQTTerr(valBLUpackmax.c_str(), errMQTT);
                                    SVAL(topBLUpackmax, valBLUpackmax.c_str());
                              }
                            BLUpackmaxold = BLUpackmax;
                          }
                      break;
                    case 17:  // UPS_MODE
                        if (BLUupsmode != BLUupsmodeold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUupsmode = BLUupsmode;
                                errMQTT = (int8_t) mqtt.publish(topBLUupsmode.c_str(), (uint8_t*) valBLUupsmode.c_str(), valBLUupsmode.length());
                                soutMQTTerr(valBLUupsmode.c_str(), errMQTT);
                                    SVAL(topBLUupsmode, valBLUupsmode.c_str());
                              }
                            BLUupsmodeold = BLUupsmode;
                          }
                      break;
                    case 18:  // AUTO_SLEEP_MODE
                        if (BLUslpmode != BLUslpmodeold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUslpmode = BLUslpmode;
                                errMQTT = (int8_t) mqtt.publish(topBLUslpmode.c_str(), (uint8_t*) valBLUslpmode.c_str(), valBLUslpmode.length());
                                soutMQTTerr(valBLUslpmode.c_str(), errMQTT);
                                    SVAL(topBLUslpmode, valBLUslpmode.c_str());
                              }
                            BLUslpmodeold = BLUslpmode;
                          }
                      break;
                    case 19:  // GRID_CHANGE_ON
                        if (BLUgrdchon != BLUgrdchonold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUgrdchon = BLUgrdchon;
                                errMQTT = (int8_t) mqtt.publish(topBLUgrdchon.c_str(), (uint8_t*) valBLUgrdchon.c_str(), valBLUgrdchon.length());
                                soutMQTTerr(valBLUdcoutp.c_str(), errMQTT);
                                    SVAL(topBLUgrdchon, valBLUgrdchon.c_str());
                              }
                            BLUgrdchonold = BLUgrdchon;
                          }
                      break;
                    case 20:  // FIELD_UNDEFINED
                        if (BLUfieldund != BLUfieldundold)
                          {
                            if (errMQTT == MD_OK)
                              {
                                valBLUfieldund = BLUfieldund;
                                errMQTT = (int8_t) mqtt.publish(topBLUfieldund.c_str(), (uint8_t*) valBLUfieldund.c_str(), valBLUfieldund.length());
                                soutMQTTerr(valBLUfieldund.c_str(), errMQTT);
                                    SVAL(topBLUfieldund, valBLUfieldund.c_str());
                              }
                            BLUfieldundold = BLUfieldund;
                          }
                      break;
                    default:
                        outpIdx = 0;
                      break;
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
                  #if (USE_BLUETTI_AC300 > OFF)
                      // AC_OUTPUT_ON
                        //S4HEXVAL(" conn MQTT idx pblu pblu[idx] size ", AC_OUTPUT_ON, (uint32_t) pbluetti_dev_state,
                        //                                              (uint32_t) &(pbluetti_dev_state[AC_OUTPUT_ON]), sizeof(device_field_data_t) );
                        pbluetti_dev_state[AC_OUTPUT_ON].p_f_value = (void*) &BLUacouton;
                        topBLUacouton = topDevice + topBLUacouton;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUacouton.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_ACOUT_ON", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_ACOUT_ON", errMQTT, (uint32_t) &BLUacouton, (uint32_t) &(pbluetti_dev_state[AC_OUTPUT_ON]).p_f_value);

                      // DC_OUTPUT_ON
                        pbluetti_dev_state[DC_OUTPUT_ON].p_f_value = (void*) &BLUdcouton;
                        topBLUdcouton = topDevice + topBLUdcouton;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUdcouton.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_DCOUT_ON", errMQTT);
                            //S3HEXVAL(" MQTT subscribe MQTT_BLU_DCOUT_ON", DC_OUTPUT_ON, (uint32_t) &BLUdcouton, (uint32_t) &(pbluetti_dev_state[DC_OUTPUT_ON]).p_f_value);

                      // DC_OUTPUT_POWER
                        //S4HEXVAL(" conn MQTT idx pblu pblu[idx] size ", DC_OUTPUT_POWER, (uint32_t) pbluetti_dev_state,
                        //                                              (uint32_t) &(pbluetti_dev_state[DC_OUTPUT_POWER]), sizeof(device_field_data_t) );
                        pbluetti_dev_state[DC_OUTPUT_POWER].p_f_value = (void*) &BLUdcoutp;
                        topBLUdcoutp = topDevice + topBLUdcoutp;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUdcoutp.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_DCOUT_POW", errMQTT);
                            //S3HEXVAL(" MQTT subscribe MQTT_BLU_DCOUT_POW", DC_OUTPUT_POWER, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[DC_OUTPUT_POWER].p_f_value));

                      // AC_OUTPUT_POWER
                        (pbluetti_dev_state[AC_OUTPUT_POWER]).p_f_value = (void*) &BLUacoutp;
                        topBLUacoutp = topDevice + topBLUacoutp;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUacoutp.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_ACOUT_POW", errMQTT);
                            //S3HEXVAL(" MQTT subscribe MQTT_BLU_ACOUT_POW", errMQTT, (uint32_t) &BLUacoutp, (uint32_t) &(pbluetti_dev_state[AC_OUTPUT_POWER]).p_f_value);

                      // POWER_GENERATION
                        pbluetti_dev_state[POWER_GENERATION].p_f_value = (void*) &BLUpowgen;
                        topBLUpowgen = topDevice + topBLUpowgen;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUpowgen.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_POW_GEN", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_POW_GEN", errMQTT, (uint32_t) &BLUpowgen, (uint32_t) &(pbluetti_dev_state[POWER_GENERATION]).p_f_value);

                      // TOTAL_BATTERY_PERCENT
                        pbluetti_dev_state[TOTAL_BATTERY_PERCENT].p_f_value = (void*) &BLUtotbatt;
                        topBLUtotbatt = topDevice + topBLUtotbatt;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUtotbatt.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_TOT_BATT", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_TOT_BATT", errMQTT, (uint32_t) &BLUtotbatt, (uint32_t) &(pbluetti_dev_state[TOTAL_BATTERY_PERCENT]).p_f_value);

                      // DC_INPUT_POWER
                        pbluetti_dev_state[DC_INPUT_POWER].p_f_value = (void*) &BLUdcinp;
                        topBLUdcinp = topDevice + topBLUdcinp;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUdcinp.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_DCIN_POW", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_DCIN_POW", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[DC_INPUT_POWER]).p_f_value);

                      // AC_INPUT_POWER
                        pbluetti_dev_state[AC_INPUT_POWER].p_f_value = (void*) &BLUacinp;
                        topBLUacinp = topDevice + topBLUacinp;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUacinp.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_ACIN_POW", errMQTT);
                            S3HEXVAL(" MQTT subscribe MQTT_BLU_ACIN_POW", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[AC_INPUT_POWER]).p_f_value);

                      // PACK_VOLTAGE
                        pbluetti_dev_state[PACK_VOLTAGE].p_f_value = (void*) &BLUpaku;
                        topBLUpaku = topDevice + topBLUpaku;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUpaku.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_PACK_U", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_PACK_U", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[PACK_VOLTAGE]).p_f_value);

                      // SERIAL_NUMBER
                        pbluetti_dev_state[SERIAL_NUMBER].p_f_value = (void*) &BLUsernum;
                        topBLUsernum = topDevice + topBLUsernum;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUsernum.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_SER_NUM", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_SER_NUM", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[SERIAL_NUMBER]).p_f_value);

                      // ARM_VERSION
                        pbluetti_dev_state[ARM_VERSION].p_f_value = (void*) &BLUarmvers;
                        topBLUarmvers = topDevice + topBLUarmvers;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUarmvers.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_ARM_VERS", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_ARM_VERS", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[ARM_VERSION]).p_f_value);

                      // DSP_VERSION
                        pbluetti_dev_state[DSP_VERSION].p_f_value = (void*) &BLUdspvers;
                        topBLUdspvers = topDevice + topBLUdspvers;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUdspvers.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_DSP_VERS", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_DSP_VERS", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[DSP_VERSION]).p_f_value);

                      // DEVICE_TYPE
                        pbluetti_dev_state[DEVICE_TYPE].p_f_value = (void*) &BLUdevtype;
                        topBLUdevtype = topDevice + topBLUdevtype;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUdevtype.c_str());

                            soutMQTTerr(" MQTT subscribe MQTT_BLU_DEV_TYPE", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_DEV_TYPE", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[DEVICE_TYPE]).p_f_value);

                      // INTERNAL_AC_VOLTAGE
                        pbluetti_dev_state[INTERNAL_AC_VOLTAGE].p_f_value = (void*) &BLUintacu;
                        topBLUintacu = topDevice + topBLUintacu;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUintacu.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_AC_VOLT", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_INT_AC_VOLT", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[INTERNAL_AC_VOLTAGE]).p_f_value);

                      // INTERNAL_CURRENT_ONE
                        pbluetti_dev_state[INTERNAL_CURRENT_ONE].p_f_value = (void*) &BLUintcurr1;
                        topBLUintcurr1 = topDevice + topBLUintcurr1;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUintcurr1.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_CURR1", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe MQTT_BLU_INT_CURR1", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[INTERNAL_CURRENT_ONE]).p_f_value);

                      // PACK_NUM_MAX
                        pbluetti_dev_state[PACK_NUM_MAX].p_f_value = (void*) &BLUpackmax;
                        topBLUpackmax = topDevice + topBLUpackmax;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUpackmax.c_str());
                            soutMQTTerr(" MQTT subscribe PACK_NUM_MAX", errMQTT);
                        //    S3HEXVAL(" MQTT subscribe PACK_NUM_MAX", errMQTT, (uint32_t) &BLUdcoutp, (uint32_t) &(pbluetti_dev_state[PACK_NUM_MAX]).p_f_value);

                      // UPS_MODE
                        pbluetti_dev_state[UPS_MODE].p_f_value = (void*) &BLUupsmode;
                        topBLUupsmode = topDevice + topBLUupsmode;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUupsmode.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_UPS_MODE", errMQTT);

                      // AUTO_SLEEP_MODE
                        pbluetti_dev_state[AUTO_SLEEP_MODE].p_f_value = (void*) &BLUslpmode;
                        topBLUslpmode = topDevice + topBLUslpmode;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUslpmode.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_SLP_MODE", errMQTT);

                      // GRID_CHANGE_ON
                        pbluetti_dev_state[GRID_CHANGE_ON].p_f_value = (void*) &BLUgrdchon;
                        topBLUgrdchon = topDevice + topBLUgrdchon;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUgrdchon.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_FIELD_UND", errMQTT);

                      // FIELD_UNDEFINED
                        pbluetti_dev_state[FIELD_UNDEFINED].p_f_value = (void*) &BLUfieldund;
                        topBLUfieldund = topDevice + topBLUfieldund;
                        errMQTT = (int8_t) mqtt.subscribe(topBLUfieldund.c_str());
                            soutMQTTerr(" MQTT subscribe MQTT_BLU_INT_AC_U", errMQTT);
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
                      SVAL(text, cerrMQTT[(-1) * errMQTT]);
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

