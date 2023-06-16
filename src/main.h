#ifndef _MAIN_H_
  #define _MAIN_H_

  // --- includes
    #include <Arduino.h>
    #include <string.h>
    #include <stdio.h>                                                        // Biblioteca STDIO
    #include <iostream>
    #include <md_time.hpp>
    #include <md_defines.h>
    #include <md_util.h>
    #include <prj_conf_BT_MQTT_hdl.h>
    #include <BTooth.h>
        //#include <driver\gpio.h>
        //#include <driver\adc.h>
        //#include "freertos/task.h"
        //#include "freertos/queue.h"
        //#include "driver/ledc.h"
        //#include "driver/mcpwm.h"
    //#include "driver/pcnt.h"
        //#include "esp_attr.h"
        //#include "esp_log.h"
  // --- system components
  // --- memory
    #if (USE_FLASH_MEM > OFF)
        #include <FS.h>
        //#include <SPIFFS.h>
        #include <md_spiffs.h>
      #endif
    #if (USE_SD_SPI > OFF)
        //#include "sdmmc_cmd.h"
        #include <FS.h>
        #include <sd.h>
        #include <spi.h>
      #endif
  // --- network
    #if (USE_WIFI > OFF)
        #include <AsyncTCP.h>
        #include <ESPAsyncWebServer.h>
        #include <md_webserver.h>
        #include <ip_list.hpp>
        #if (USE_MQTT > OFF)
            #include <Network/Clients/MQTT.hpp>
          #endif
      #endif

  // ---------------------------------------
  // --- prototypes
    // ------ system -------------------------
      // --- heap ------------------------
        void heapFree(const char* text);
    // ------ user interface -----------------
      // --- user output
      // --- user input
        // vcc measure
          #if (USE_VCC50_ANA > OFF)
              void initVCC50();
            #endif
          #if (USE_VCC33_ANA > OFF)
              void initVCC33();
            #endif
    // ----- memory ---------------------------
          #if (USE_FLASH_MEM > OFF)
              void startFlash();
              void testFlash();
            #endif
    // ------ network -------------------------
      // --- WIFI
        #if (USE_WIFI > OFF)
            uint8_t startWIFI(bool startup);
            #if (USE_NTP_SERVER > OFF)
                void    initNTPTime();
              #endif
          #endif
        #if (USE_BTOOTH > OFF)

          #endif
      // --- MQTT
        #if (USE_MQTT > OFF)
            void startMQTT();
            void connectMQTT();
            void soutMQTTerr(String text, int8_t errMQTT);
            void readMQTTmsg();
          #endif
    // ---------
#endif // _MAIN_H_
