
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include "TouchEvent.h"
#include "FS.h"
#include "ESP8266WiFi.h"
//ArduiTouch libraries by Gerald Lechner from github
#include "AT_Database.h"
#include "AT_MessageBuffer.h"
#include "TouchEvent.h"
#include <AT_Display.h>


//used pins
#define TFT_CS   D1      //diplay chip select
#define TFT_DC   D2      //display d/c
#define TFT_LED  15     //display background LED
#define ARDUITOUCH_VERSION 1 //0 for older 1 for new with PNP Transistor

#define TOUCH_CS 0    //touch screen chip select
#define TOUCH_IRQ 2     //touch screen interrupt

//file names used with SPIFFS do not remove the starting slash!

#define ATDEVICEFILE "/devices.txt"
#define ATCONFIGFILE "/config.txt"

//data structures for database and message buffer
AT_Database database(ATDEVICEFILE,ATCONFIGFILE);
AT_MessageBuffer msg;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046_Touchscreen touch(TOUCH_CS,TOUCH_IRQ);
TouchEvent tevent(touch);

AT_Display dsp(&tft,&database,TFT_LED,ARDUITOUCH_VERSION);



//sub files for this scetch do not change the order
#include "MyWifi.h"     //all associated to WiFi

//Format flash file system if not already done
#define FORMAT_SPIFFS_IF_FAILED true

void onClick(TS_Point p) {
  Serial.println("Click");
  dsp.onClick(p);
}
void onLong(TS_Point p) {
  dsp.onLongClick(p);
}
void onSwipe(uint8_t direction) {
  dsp.onSwipe(direction);
}

void setup() {
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("Start");
  pinMode(TFT_LED,OUTPUT);
  int heap = ESP.getFreeHeap();
  Serial.printf("RAM available %i\n",heap);
  touch.begin();
  tevent.setResolution(tft.width(),tft.height());
  tevent.setDblClick(200);
  tevent.registerOnTouchLong(onLong);
  tevent.registerOnTouchClick(onClick);
  tevent.registerOnTouchSwipe(onSwipe);
  dsp.begin(1);
  dsp.display(true);
  dsp.registerOnResultChange(sendData);
  //start flash file system
  if (SPIFFS.begin()) Serial.println("SPIFFS loaded");
  initWiFi();
  //connectWlan("","");
  delay(100);
  if (!initESPNow()) dsp.setStatus("ESP NOW failed");
  
  if (!database.readDevices()) Serial.println("Can not read devices");
  if (!database.readConfig()) Serial.println("Can not read widget config");
  dsp.showCurrentPage();
  
}

void loop() {
  yield();
  tevent.pollTouchScreen();
  dsp.updateDisplay();
}
