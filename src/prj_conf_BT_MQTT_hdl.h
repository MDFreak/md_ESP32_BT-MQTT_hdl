#ifndef _PRJ_CONF_BT_MQTT_HDL_H_
  #define _PRJ_CONF_BT_MQTT_HDL_H_

  #include <Arduino.h>
  #include <md_defines.h>
  // ******************************************
  // --- project
    #define UTC_SEASONTIME UTC_SUMMERTIME
    #define USE_MQTT       ON
    #define USE_BTOOTH     ON
    #define USE_WIFI       ON
    #define USE_FLASH_MEM  ON
    #define USE_NTP_SERVER ON
    #define USE_BLUETTI    ON
    #define USE_VOLTCRAFT  ON

  // --- system
    #define USE_OUTPUT_CYCLE
    #define USE_INPUT_CYCLE
    #define INPUT_CYCLE_MS          10
    #define OUTPUT_CYCLE_MS         21
  // --- bluetti
    #if (USE_BLUETTI > OFF)
        #define AC300               1
        #define AC200M              2     // not implemented
        #define EP500               3     // not implemented
        #define EB3A                4     // not implemented
        #define EP500P              5     // not implemented
        #define AC500               6     // not implemented

        #define USE_BLUETTI_AC300   ON
        #define USE_BLUETTI_AC200M  OFF   // not implemented
        #define USE_BLUETTI_EP500   OFF   // not implemented
        #define USE_BLUETTI_EB3A    OFF   // not implemented
        #define USE_BLUETTI_EP500P  OFF   // not implemented
        #define USE_BLUETTI_AC500   OFF   // not implemented

        #define BT_QUERY_MSG_DELAY  3000
        #define MAX_DISCONN_TIME_TO_REBOOT  5 //device will reboot when wlan/BT/MQTT is not connectet within x Minutes
        #define SLEEP_TIME_ON_BT_NOT_AVAIL  2 //device will sleep x minutes if restarted is triggered by bluetooth error
                                              //set to 0 to disable
        #if (USE_BLUETTI_AC300 > OFF)
            //#define DEVICE_NAME = "BLUETTI_AC300"
            #include <Bluetti_AC300.h>
          #endif
        #if (USE_BLUETTI_AC200 > OFF)
            #include <Bluetti_AC200.h>
          #endif
        #if (USE_BLUETTI_EP500 > OFF)
            #include <Bluetti_EP500.h>
          #endif
        #if (USE_BLUETTI_EB3A > OFF)
            #include <Bluetti_EB3A.h>
          #endif
        #if (USE_BLUETTI_AC500 > OFF)
            #include <Bluetti_AC500.h>
          #endif
        #if (USE_MQTT > OFF)
            #define MQTT_DEVICE           "bluetti"
            #define MQTT_TOPDEV           "bluetti/"
          #endif
      #endif
  // voltcraft (conrad)
    #if (USE_VOLTCRAFT > OFF)
        #define SEM6000             1

        #define VOLTCRAFT_SEM6000   ON
      #endif
  // --- network
    // wifi
      #if (USE_WIFI > OFF)
          #define WIFI_MAX_LEN    20
          #define WIFI_ANZ_LOGIN  8
          #define WIFI_IS_DUTY    ON
          #define WIFI_SSID2      "HS-HomeG"    // WLAN Am Jungberg 9
          #define WIFI_SSID2_PW   "ElaNanniRalf3"
          #define WIFI_SSID3      "WL-Fairnetz" //Weltladen
          #define WIFI_SSID3_PW   "WL&Fair2Live#"
          #define WIFI_SSID4      "MachNet"     //machQuadrat
          #define WIFI_SSID4_PW   "!Machquadrat1"
          #define WIFI_SSID5      "MD_KingKong" //Hotspot Martin
          #define WIFI_SSID5_PW   "ElaNanniRalf3"
          #define WIFI_SSID6      "CDWiFi"      //OEBB Raijet
          #define WIFI_SSID6_PW   ""
          #define WIFI_SSID7      "xWIFI@DB"    //DB ICE
          #define WIFI_SSID7_PW   ""
          #define WIFI_SSID8      ""    // ?
          #define WIFI_SSID8_PW   ""
          #define WIFI_SSID9      ""    // ?
          #define WIFI_SSID9_PW   ""
          #define WIFI_CONN_DELAY 500000ul // Scan-Abstand [us]
          #define WIFI_CONN_REP   5        // Anzahle der Connect-Schleifen
          #define WIFI_CONN_CYCLE 4000ul   // Intervallzeit fuer Recoonect [us]
          #define NTPSERVER_CYCLE 1000ul   // Intervallzeit [us]

          #define WIFI_ANZ_LOCIP  WIFI_ANZ_LOGIN
          // Router Bauwagen 192.168.0.1
            #define WIFI_FIXIP0     0x1F00A8C0ul // 192.168.0.31   lowest first
            #define WIFI_SSID0      "MAMD-mobil"   // Bauwagen
            #define WIFI_SSID0_PW   "M&M2KsR&N#"
            #define WIFI_GATEWAY0   0x0100A8C0ul // 192.168.0.1      // Bauwagen
          // Router Moosgraben 192.168.0.1
            #define WIFI_FIXIP1     0x1F00000Aul // 10.0.0.231
            #define WIFI_SSID1      "MAMD-HomeG"  // WLAN Moosgrabenstrasse 26
            #define WIFI_SSID1_PW   "ElaNanniRalf3"
            #define WIFI_GATEWAY1   0x8B00000Aul // 10.0.0.139      // Moosgraben
            #define WIFI_GATEWAY2   0x8B00000Aul // 10.0.0.139      // Jungberg
            #define WIFI_FIXIP2     0x1800000Aul // 10.0.0.24
            #define WIFI_GATEWAY3   0x8a00000Aul // 10.0.0.138      // Weltladen
            #define WIFI_FIXIP3     0x1600000Aul // 10.0.0.22
            #define WIFI_GATEWAY4   0x01250D0Aul // 10.0.0.1        // machquadrat
            #define WIFI_FIXIP4     0x6F250D0Aul // 10.0.0.22
            #define WIFI_GATEWAY5   0x012BA8C0ul // 192.168.43.154  // hotspot KingKong
            #define WIFI_FIXIP5     0x162BA8C0ul // 192.168.43.22
            #define WIFI_GATEWAY6   0x0926A8C0ul // 192.168.32.1    // OEBB Railjet
            #define WIFI_FIXIP6     0x1620A8C0ul // 192.168.32.22
            #define WIFI_GATEWAY7   0x01AE12ACul // 172.18.0.1      // DB ICE
            #define WIFI_FIXIP7     0x16AE12ACul // 172.18.174.22
            #define WIFI_GATEWAY8   0x0100000Aul // 10.0.0.10       // ?
            #define WIFI_FIXIP8     0x1600000Aul // 10.0.0.22       // ?
            #define WIFI_GATEWAY9   0x0100000Aul // 10.0.0.1        // ?
            #define WIFI_FIXIP9     0x1600000Aul // 10.0.0.22       // ?
            #define WIFI_SUBNET     0x00FFFFFFul // 255.255.255.0
        #endif
    // --- bluetooth
      #define DEV_STATE_UPDATE_S          5 //time in sec
      #define MSG_VIEWER_ENTRY_COUNT      20 //number of lines for web message viewer
      #define MSG_VIEWER_REFRESH_CYC_S    5 //refresh time for website data in seconds

      #if (USE_BLUETTI_AC300 > OFF)
              // The remote Bluetti service we wish to connect to.
          #define BLUETTI_UUID_SERVICE  "0000ff00-0000-1000-8000-00805f9b34fb"
              // The characteristics of Bluetti Devices
          #define BLUETTI_UUID_NOTIFY   "0000ff01-0000-1000-8000-00805f9b34fb"
          #define BLUETTI_UUID_WRITE    "0000ff02-0000-1000-8000-00805f9b34fb"
        #endif
    #if (USE_VOLTCRAFT > OFF)
          // The remote Voltcraft service we wish to connect to
            // device name
            // handle: 0x0002u, char properties: 0x02u, char value handle: 0x0003u
          #define VOLTCRAFT_UUID_SERVICE  "00002a00-0000-1000-8000-00805f9b34fb"
            // device vendor, hardware, firmware
            // handle: 0x0024u, char properties: 0x06u, char value handle: 0x0025u
          #define VOLTCRAFT_UUID_NAME     "0000ff01-0000-1000-8000-00805f9b34fb"
            // ?
            // handle: 0x0027u, char properties: 0x02u, char value handle: 0x0028u
          #define VOLTCRAFT_UUID_UNDEF   "0000ff02-0000-1000-8000-00805f9b34fb"
            // device vendor, hardware, firmware
            // handle: 0x002au, char properties: 0x04u, char value handle: 0x002bu
          #define VOLTCRAFT_UUID_WRITE    "0000ff03-0000-1000-8000-00805f9b34fb"
            // notifcations
            // handle: 0x002du, char properties: 0x10u, char value handle: 0x002eu
          #define VOLTCRAFT_UUID_NOTIFY   "0000ff04-0000-1000-8000-00805f9b34fb"
      #endif
    // --- MQTT Mosquitto client
    #if (USE_MQTT > OFF)
        #define MQTT_HOST             "10.0.0.203"
        //#define MQTT_HOST             "10.13.37.27"
        #define MQTT_PORT             1883
        #define MQTT_SECURE           OFF
        #define MQTT_TOPIC_MAXLEN     20
        #define MQTT_PAYLOAD_MAXLEN   20
        #define MQTT_MSG_MAXANZ       30
        typedef struct // MQTT_MSG
          {
            char  topic[MQTT_TOPIC_MAXLEN];
            char  payload[MQTT_PAYLOAD_MAXLEN];
            void* pNext;
          } MQTTmsg_t;
        #if(MQTT_SECURE > OFF)
            #define MQTT_BROKER_USER  "<user>"
            #define MQTT_BROKER_PASS  "<pass>"
          #endif
      #endif
    // ******************************************
#endif // _PRJ_CONF_BT_MQTT_HDL_H_
