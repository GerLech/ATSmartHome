#include "FS.h"
#include "SPIFFS.h"
//ArduiTouch libraries by Gerald Lechner from github
#include "AT_Database.h"
#include "AT_MessageBuffer.h"
#include "TouchEvent.h"

//data structures for database and message buffer
AT_Database database;
AT_MessageBuffer msg;

//file names used with SPIFFS do not remove the starting slash!

#define ATDEVICEFILE "/devices.txt"
#define ATCONFIGFILE "/config.txt"

//sub files for this scetch do not change the order
#include "ATWiFi.h"     //all associated to WiFi
#include "ATDisplay.h"  //all associated to display and touch screen


//Format flash file system if not already done
#define FORMAT_SPIFFS_IF_FAILED true

//main setup
void setup() {
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("Start");
  initDisplay(); //call setup routine in the display subfile
  //start flash file system
  if (SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) Serial.println(F("SPIFFS loaded"));
  initWiFi();
  //connectWlan("","");
  delay(100);
  if (!initESPNow()) error = "ESP NOW failed";
  
  if (!database.readDevices(ATDEVICEFILE)) Serial.println("Can not read devices");
  if (!database.readConfig(ATCONFIGFILE)) Serial.println("Can not read widget config");
  //set the display to the firs result page
  currentPage = 0;
}

uint32_t tdisplay = 0; //time for the next display update 

void loop() {
  tevent.pollTouchScreen(); //check for touch events
  if (millis() > tdisplay) { //if time was reached (1s) update display
    showCurrentPage();
    tdisplay = millis()+1000;
  }
}
