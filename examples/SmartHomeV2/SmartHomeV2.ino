
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include "TouchEvent.h"
#include "FS.h"
#include "SPIFFS.h"
#include "WebServer.h"
//ArduiTouch libraries by Gerald Lechner from github
#include "AT_Database.h"
#include "AT_MessageBuffer.h"
#include "TouchEvent.h"
#include <AT_Display.h>
#include "AT_Webserver.h"

#include <ESPiLight.h>


//used pins
#define TFT_CS   5      //diplay chip select
#define TFT_DC   4      //display d/c
#define TFT_RST  22     //display reset
#define TFT_LED  15     //display background LED
#define ARDUITOUCH_VERSION 0 //0 for older 1 for new with PNP Transistor

#define TOUCH_CS 14     //touch screen chip select
#define TOUCH_IRQ 2     //touch screen interrupt

//file names used with SPIFFS do not remove the starting slash!

#define ATDEVICEFILE "/devices.txt"
#define ATCONFIGFILE "/config.txt"

#define RCPIN 16

//data structures for database and message buffer
AT_Database database(ATDEVICEFILE,ATCONFIGFILE);
AT_MessageBuffer msg;
ATSETUP * stp;
Adafruit_ILI9341 tft(TFT_CS,TFT_DC,TFT_RST);
XPT2046_Touchscreen touch(TOUCH_CS,TOUCH_IRQ);
TouchEvent tevent(touch);

AT_Display dsp(&tft,&database,TFT_LED,ARDUITOUCH_VERSION);
WebServer srv(80);
AT_Webserver webserver(&srv,&database);

//Shoud be defined if a Ardui-Touch version 01-02 or higher is used
//#define ARDUITOUCHB

ESPiLight rf(-1);

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

void handleRoot() {
  webserver.handleRoot(stp->refresh);
}

void handleNotFound() {
  webserver.handleNotFound();
}

//this function is called if system setup has changed
void systemChanged() {
  ATSETUP * stp=database.getSetup();
  if (stp->useWlan) {
    connectWlan(stp->SSID,stp->password,stp->NTPserver);
  } else {
    disconnectWlan();
  }
}

// callback function. It is called on successfully received and parsed rc signal
void rfCallback(const String &protocol, const String &message, int status,
                size_t repeats, const String &deviceID) {
  /*
  Serial.print("RF signal arrived [");
  Serial.print(protocol);  // protocoll used to parse
  Serial.print("][");
  Serial.print(deviceID);  // value of id key in json message
  Serial.print("] (");
  Serial.print(status);  // status of message, depending on repeat, either:
                         // FIRST   - first message of this protocoll within the
                         //           last 0.5 s
                         // INVALID - message repeat is not equal to the
                         //           previous message
                         // VALID   - message is equal to the previous message
                         // KNOWN   - repeat of a already valid message
  Serial.print(") ");
  Serial.print(message);  // message in json format
  Serial.println();
  */
  // check if message is valid and process it
  if (status == FIRST) {
    Serial.println("**************************************************");
    Serial.print("Valid message: [");
    Serial.print(protocol);
    Serial.print("] ");
    Serial.print(message);
    Serial.print(" ");
    Serial.print(deviceID);
    Serial.println();
    Serial.println("**************************************************");
  }
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
  webserver.registerOnResultChange(sendData);
  dsp.registerOnSystemChanged(systemChanged);
  //start flash file system
  if (SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) Serial.println(F("SPIFFS loaded"));
  initWiFi();
  //connectWlan("","");
  delay(100);
  if (!initESPNow()) dsp.setStatus("ESP NOW failed");
  
  if (!database.readDevices()) Serial.println("Can not read devices");
  if (!database.readConfig()) Serial.println("Can not read widget config");
  if (!database.readSetup()) Serial.println("Can not read setup");
  stp = database.getSetup();
  if (stp->useWlan) connectWlan(stp->SSID, stp->password, stp->NTPserver);
  dsp.showCurrentPage();
  srv.on("/",handleRoot);
  srv.onNotFound(handleNotFound);
  webserver.begin();
  // set callback funktion
  rf.setCallback(rfCallback);
  // inittilize receiver
  rf.initReceiver(RCPIN);
  rf.limitProtocols("[\"arctech_switch\"]");

}

void loop() {
  tevent.pollTouchScreen();
  dsp.updateDisplay();
  webserver.handleClient();
  rf.loop();
}
