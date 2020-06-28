// Based on https://raw.githubusercontent.com/tobozo/M5Stack-PacketMonitor/master/M5Stack-PacketMonitor.ino

#include "freertos/FreeRTOS.h"
#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string>
#include <cstddef>
#include <Wire.h>
#include <Preferences.h>
#include <M5SysBase.h>
#include "FS.h"
#include "SD.h"
#include "Buffer.h"

using namespace           std;

#define MAX_CH            14                          // 1 - 14 channels (1-11 for US, 1-13 for EU and 1-14 for Japan)
#define SNAP_LEN          2324                        // max len of each received packet
#define BG_COLOR          NAVY
#define TXT_COLOR         WHITE
#define GRAPH_COLOR       WHITE
#define DRAW_DELAY        1000

#define RESERVE_TOP       40
#define RESERVE_BOT       28
#define MAX_X             320
#define MAX_Y             (240 - RESERVE_TOP - RESERVE_BOT)
#define RUNNING_CORE      1

#define LEFT_EDGE         0
#define RIGHT_EDGE        MAX_X
#define TOP_UPPER_PADDING 6
#define TOP_LEFT_MARGIN   8
#define TOP_SEP_LINE      27
#define TOP_SEP_LINE_1    87
#define TOP_SEP_LINE_2    208
#define BOT_SEP_LINE      212
#define BOT_UPPER_PADDING 220
#define BUT_A_TEXT_POS    53
#define BUT_B_TEXT_POS    148
#define BUT_C_TEXT_POS    230


M5SysBase                 m5sys;
Buffer                    sdBuffer;
Preferences               preferences;
bool                      saveEnabled         = false;
bool                      useSD               = false;
bool                      buttonPressed       = false;
bool                      buttonEnabled       = true;
int                       buttonId            = 0;
uint32_t                  lastDrawTime;
uint32_t                  lastButtonTime;
uint32_t                  tmpPacketCounter;
uint32_t                  pkts[MAX_X];                // here the packets per second will be saved
uint32_t                  deauths             = 0;    // deauth frames per second
unsigned int              ch                  = 1;    // current 802.11 channel
int                       rssiSum;


esp_err_t event_handler(void* ctx, system_event_t* event) {
  // No logging in RTOS event handlers
  return ESP_OK;
}


void wifi_promiscuous(void* buf, wifi_promiscuous_pkt_type_t type) {
  VERBOSE("wifi_promiscuous(buff, type=%d\n", type);
  wifi_promiscuous_pkt_t* pkt           = (wifi_promiscuous_pkt_t*)buf;
  wifi_pkt_rx_ctrl_t      ctrl          = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;
  uint32_t                packetLength  = ctrl.sig_len;

  if (type == WIFI_PKT_MGMT && (pkt->payload[0] == 0xA0 || pkt->payload[0] == 0xC0 )) deauths++;

  if (type == WIFI_PKT_MISC)   return;           // wrong packet type
  if (ctrl.sig_len > SNAP_LEN) return;           // packet too long

  if (type == WIFI_PKT_MGMT) packetLength -= 4;  // fix for known bug in the IDF https://github.com/espressif/esp-idf/issues/886

  //Serial.print(".");
  tmpPacketCounter++;
  rssiSum += ctrl.rssi;

  if (useSD) sdBuffer.addPacket(pkt->payload, packetLength);
}


double getMultiplicator() {
  VERBOSE("getMultiplicator()\n");
  uint32_t maxVal = 1;
  for (int i = 0; i < MAX_X; i++) {
    if (pkts[i] > maxVal) maxVal = pkts[i];
  }
  if (maxVal > MAX_Y)
    return (double)MAX_Y / (double)maxVal;
  else
    return 1;
}


void setChannel(int newChannel) {
  VERBOSE("setChannel(%d)\n", newChannel);
  ch = newChannel;
  if (ch > MAX_CH || ch < 1) ch = 1;
  DEBUG("Channel set to %d\n", ch);

  preferences.begin("packetmonitor32", false);  // read-write
  preferences.putUInt("channel", ch);
  preferences.end();

  esp_wifi_set_promiscuous(false);
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous_rx_cb(&wifi_promiscuous);
  esp_wifi_set_promiscuous(true);
}


bool setupSD() {
  VERBOSE("setupSD()\n");
  if(saveEnabled) {
    if (!SD.begin()) {
      Serial.println("Card Mount Failed");
      return false;
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    INFO("SD Card Size: %lluMB\n", cardSize);
    return true;
  }
  return false;
}


void draw() {
  VERBOSE("draw()\n");
  double multiplicator = getMultiplicator();
  int len;
  int rssi;

  if (pkts[MAX_X - 1] > 0)
    rssi = rssiSum / (int)pkts[MAX_X - 1];
  else
    rssi = rssiSum;
  
  // draw the graph
  for (int i = 0; i < MAX_X; i++) {
    len = pkts[i] * multiplicator;
    if (i < MAX_X - 1) {
      pkts[i] = pkts[i + 1];
      int newlen = pkts[i] * multiplicator;
      int oldpos = (239 - RESERVE_BOT) - (len > MAX_Y ? MAX_Y : len);
      int newpos = (239 - RESERVE_BOT) - (newlen > MAX_Y ? MAX_Y : newlen);
      if(newlen - len != 0) {
        if(newlen > len) {
          DEBUG("Drawing %d, %d, %d, %d\n", i, oldpos, i, newpos);
          M5.Lcd.drawLine(i, oldpos, i, newpos, GRAPH_COLOR);      
        }
        else {
          DEBUG("Erasing %d, %d, %d, %d\n", i, oldpos, i, newpos);
          M5.Lcd.drawLine(i, oldpos, i, newpos, BG_COLOR);      
        }
      }
    }
  }
  M5.Lcd.setTextColor(TXT_COLOR, BG_COLOR);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(TOP_LEFT_MARGIN, TOP_UPPER_PADDING);
  M5.Lcd.print("Ch# ");
  M5.Lcd.print(ch);
  M5.Lcd.print(" ");
  M5.Lcd.drawLine(TOP_SEP_LINE_1, LEFT_EDGE, TOP_SEP_LINE_1, TOP_SEP_LINE, TXT_COLOR);

  M5.Lcd.setCursor(TOP_SEP_LINE_1 + TOP_LEFT_MARGIN, TOP_UPPER_PADDING);
  M5.Lcd.print("RSSI ");
  M5.Lcd.print(rssi);
  M5.Lcd.print("  ");
  M5.Lcd.drawLine(TOP_SEP_LINE_2, LEFT_EDGE, TOP_SEP_LINE_2, TOP_SEP_LINE, TXT_COLOR);

  M5.Lcd.setCursor(TOP_SEP_LINE_2 + TOP_LEFT_MARGIN, TOP_UPPER_PADDING);
  M5.Lcd.print("Pkts ");
  M5.Lcd.print(tmpPacketCounter);
  M5.Lcd.print("  ");

  M5.Lcd.drawLine(LEFT_EDGE, TOP_SEP_LINE, RIGHT_EDGE, TOP_SEP_LINE, TXT_COLOR);
  M5.Lcd.drawLine(LEFT_EDGE, BOT_SEP_LINE, RIGHT_EDGE, BOT_SEP_LINE, TXT_COLOR);

  M5.Lcd.setCursor(BUT_A_TEXT_POS, BOT_UPPER_PADDING);
  M5.Lcd.print("Ch+");
  M5.Lcd.setCursor(BUT_B_TEXT_POS, BOT_UPPER_PADDING);
  M5.Lcd.print("Ch-");
  M5.Lcd.setCursor(BUT_C_TEXT_POS, BOT_UPPER_PADDING);
  M5.Lcd.print(saveEnabled ? "Stop" : "Save");  // Print the opposite, the action to perform
}


void coreTask( void * p ) {
  VERBOSE("coreTask()\n");
  uint32_t currentTime;
  while (true) {
    currentTime = millis();
    M5.update();
    if(M5.BtnA.wasReleased()) {
      INFO("Button A\n");
      if(++ch > MAX_CH) ch = 1; 
      setChannel(ch);
    }
    if(M5.BtnB.wasReleased()) {
      INFO("Button B\n");
      if(--ch < 1) ch = MAX_CH;
      setChannel(ch);
    }
    if(M5.BtnC.wasReleased()) {
      INFO("Button C\n");
      if(saveEnabled) {
        if (useSD) {
          useSD = false;
          sdBuffer.close(&SD);
        }
      }
      saveEnabled = !saveEnabled;
      INFO("Setting SC save mode to %s\n", saveEnabled ? "on" : "off");
      if(saveEnabled) {
        if (setupSD())
          sdBuffer.open(&SD);
      }
      M5.Lcd.setCursor(BUT_C_TEXT_POS, BOT_UPPER_PADDING);
      M5.Lcd.print(saveEnabled ? "Stop" : "Save");  // Print the opposite, the action to perform
    }
    if (useSD) sdBuffer.save(&SD);  // save buffer to SD

    // draw Display
    if (currentTime - lastDrawTime > DRAW_DELAY) {
      lastDrawTime = currentTime;
      VERBOSE("Free RAM %u %u\n", heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT), heap_caps_get_minimum_free_size(MALLOC_CAP_32BIT));
      pkts[MAX_X - 1] = tmpPacketCounter;
      draw();
      DEBUG("coreTask: packet size was %d\n", pkts[MAX_X - 1]);

      tmpPacketCounter  = 0;
      deauths           = 0;
      rssiSum           = 0;
    }
  }
}


void setup() {
  m5sys.begin("PacketMonitor", NETWORK_CONNECTION_AUTO);
  preferences.begin("packetmonitor32");   // read-only
  ch = preferences.getUInt("channel", 1);
  preferences.end();

  // System & WiFi
  nvs_flash_init();
  tcpip_adapter_init();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
  ESP_ERROR_CHECK(esp_wifi_start());
  esp_wifi_set_channel(ch, WIFI_SECOND_CHAN_NONE);

  // SD card
  sdBuffer = Buffer();

  if (setupSD())
    sdBuffer.open(&SD);
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(BG_COLOR);

  // second core
  xTaskCreatePinnedToCore(
    coreTask,               /* Function to implement the task */
    "coreTask",             /* Name of the task */
    2500,                   /* Stack size in words */
    NULL,                   /* Task input parameter */
    0,                      /* Priority of the task */
    NULL,                   /* Task handle. */
    RUNNING_CORE);          /* Core where the task should run */

  // start Wifi sniffer
  esp_wifi_set_promiscuous_rx_cb(&wifi_promiscuous);
  esp_wifi_set_promiscuous(true);
}


void loop() {
  VERBOSE("loop()\n");
  vTaskDelay(portMAX_DELAY);
}
