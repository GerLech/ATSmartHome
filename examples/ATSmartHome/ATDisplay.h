/* subfile to ATSmartHome
 *  has all functions and global data 
 *  associated with display and touch screen
 */
#include <Arduino.h>
//libraries for display
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>

//fonts from the ArduiTouch library
#include "fonts/AT_Bold12pt7b.h" 
#include "fonts/AT_Standard9pt7b.h" 


//used pins
#define TFT_CS   5      //diplay chip select
#define TFT_DC   4      //display d/c
#define TFT_MOSI 23     //diplay MOSI
#define TFT_CLK  18     //display clock
#define TFT_RST  22     //display reset
#define TFT_MISO 19     //display MISO
#define TFT_LED  15     //display background LED


#define TOUCH_CS 14     //touch screen chip select
#define TOUCH_IRQ 2     //touch screen interrupt


//prepare driver for display and touch screen
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
TouchEvent tevent(touch);

//global variables
int16_t currentPage = 0; //current page on display positive numbers for result pages
                        //-1 = System setup -2 = device setup -3 = widget setup
uint8_t currentWidget; //widget to be used in widget setup
String error = "";

//predefine showPage() function to use it before definition
void showPage(int16_t page);

//handle double click events if system setup page is displayed
void sysSetupDbl(TS_Point p) {
  if (tevent.isInArea(p,20,60,220,100)) {
    SPIFFS.format();
    Serial.println("Format SPIFFS");
    showPage(0);
  }
  if (tevent.isInArea(p,20,120,220,160)) {
    showPage(0);
  }

}

//handle double click events if device setup page is displayed
void devSetupDbl(TS_Point p) {
  if (tevent.isInArea(p,20,90,220,130)) {
    database.registerDev(newdevice);
    Serial.println("Device registered");
    if (!database.writeDevices(ATDEVICEFILE)) Serial.print("Cannot write devices");
    newdevice = "";
    showPage(0);
  }
  if (tevent.isInArea(p,20,150,220,190)) {
    showPage(0);
  }
  
}

//callback for double click events
void onDblClick(TS_Point p) {
  if (currentPage >= 0) {
    //double click on top bar opens system setup page
    if (p.y < 40) {
      showPage(-1);
    } 
    //double click on bottom bar opens device setup page
    else if ((p.y > 280) && (newdevice.length() > 0)) {
      showPage(-2);
      
    }
  } else {
    switch (currentPage) {
      case -1 : sysSetupDbl(p); break;
      case -2 : devSetupDbl(p); break;
      default: showPage(0);
    }
  }
}

//prepare display and register callback for touch events
void initDisplay() {
  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);    // switch display on
  #ifdef ARDUITOUCH0102
    digitalWrite(TFT_LED, LOW);  
  #endif
  //start drivers
  tft.begin();
  touch.begin(); 
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(0); 
  tevent.setResolution(tft.width(),tft.height());
  tevent.setDblClick(200);
  tevent.registerOnTouchDblClick(onDblClick);

}

//convert Strings from UTF8 into codetable for AT_ fonts
//do allow german umlaut and degree sign
String toDisplay(String text) {
  String res = "";
  uint8_t i = 0;
  char c;
  while (i<text.length()) {
    c=text[i];
    if (c==195) { //UTF8 Zeichen Umlaute
      i++;
      switch (text[i]) {
        case 164: c=130; break; //ä
        case 182: c=131; break; //ö
        case 188: c=132; break; //ü
        case 159: c=133; break; //ß
        case 132: c=127; break; //Ä 
        case 150: c=128; break; //Ö
        case 156: c=129; break; //Ü
        default: c=0;
      }
    } else if (c == 194) { //UTF8 Zeichen Grad Symbol
      i++;
      if (text[i] == 176) c=134; else c=0;
    } else if (c > 128) { //normale Zeichen bleiben unverändert
      c=0;
    }
    if (c>0) res.concat(c);
    i++;
  }
  return res;
}

//call the print function with converted string
void printTft(String text) {
  tft.print(toDisplay(text));
}

//draw a rounded filled rectangle with centered label
void drawButton(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bc, uint16_t rc, String lbl) {
  uint16_t w1,h1;
  int16_t x1,y1,hx;
  tft.fillRoundRect(x,y,w,h,6,bc);
  tft.drawRoundRect(x,y,w,h,6,rc);
  tft.drawRoundRect(x+1,y+1,w-2,h-2,5,rc);
  tft.getTextBounds(lbl,x,y,&x1,&y1,&w1,&h1);
  hx = y-y1;
  tft.setCursor(x+(w-w1)/2,y+(h-hx)/2+hx);
  printTft(lbl);
}

//draw a filled rectangle with centered label
void drawWidget(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bc, uint16_t rc, String lbl) {
  uint16_t w1,h1;
  int16_t x1,y1;
  tft.fillRect(x,y,w,h,bc);
  tft.drawRect(x,y,w,h,rc);
  tft.drawRect(x+1,y+1,w-2,h-2,rc);
  tft.getTextBounds(lbl,x,y,&x1,&y1,&w1,&h1);
  tft.setCursor(x+(w-w1)/2,y1+h/2 +h1);
  printTft(lbl);
}

//draw horizontal centered text
//centered on w with offset x
void centerText(uint16_t x, uint16_t y, uint16_t w, String text) {
  uint16_t w1,h1;
  int16_t x1,y1;
  tft.getTextBounds(text,x,y,&x1,&y1,&w1,&h1);
  tft.setCursor(x+(w-w1)/2,y);
  printTft(text);
}

//show the top bar on result pages
void showTimeBar() {
  tft.setFont(&AT_Bold12pt7b);
  tft.fillRect(0,0,240,40,ILI9341_BLUE);
  tft.setTextColor(ILI9341_YELLOW,ILI9341_BLUE);
  tft.setCursor(10,25);
  printTft("AT Smart Home");
  //printTft(AT_GetLocalTime());  

}

//show the bottom bar on result pages 
void showStatusBar() {
  tft.setFont(&AT_Bold12pt7b);
  tft.fillRect(0,280,240,39,ILI9341_BLUE);
  tft.setCursor(10,280+25);
  if (newdevice.length() > 0) {
      tft.setTextColor(ILI9341_YELLOW,ILI9341_BLUE);
      printTft(newdevice);
  }
  else if (error.length() > 0) {
      tft.setTextColor(ILI9341_WHITE,ILI9341_BLUE);
      printTft(error);
  }
}

//show result page
void showResults() {
  ATCURVALUES val;
  String msg;
  uint16_t x = 0;
  uint16_t y = 40;
  tft.setFont(&AT_Standard9pt7b);
  tft.setTextColor(ILI9341_BLACK,ILI9341_YELLOW);
  for (uint16_t i = 0; i<ATMAXCHANNELS; i++) {
    val = database.getResult(i);
    if ((val.valid == 1) && (val.step == 1)) {
      msg = "Sensor "+String(i)+" "+String(AT_GetFloat(val.value))+" °C";
      drawWidget(x,y,240,30,ILI9341_YELLOW,ILI9341_YELLOW,msg);
      database.setStep(0,i);
      y+=30;
    }
    
  }
  
}

//show setup page for system setups
void showSystemSetup() {
  tft.setFont(&AT_Bold12pt7b);
  tft.setTextColor(ILI9341_YELLOW,ILI9341_BLACK);
  centerText(0,30,240,"System");
  tft.setTextColor(ILI9341_BLACK,ILI9341_YELLOW);
  drawButton(20,60,200,40,ILI9341_YELLOW,ILI9341_BLUE,"Format FS");
  drawButton(20,120,200,40,ILI9341_YELLOW,ILI9341_BLUE,"Zurück");
}

//show setup page for device setups
void showDeviceSetup() {
  tft.setFont(&AT_Bold12pt7b);
  tft.setTextColor(ILI9341_YELLOW,ILI9341_BLACK);
  centerText(0,30,240,"Neues Gerät");
  centerText(0,60,240,newdevice);
  tft.setTextColor(ILI9341_BLACK,ILI9341_YELLOW);
  drawButton(20,90,200,40,ILI9341_YELLOW,ILI9341_BLUE,"Registrieren");
  drawButton(20,150,200,40,ILI9341_YELLOW,ILI9341_BLUE,"Zurück");
}

void showWidgetSetup(uint8_t widget) {
  
}


void showCurrentWidgetSetup() {
  showWidgetSetup(currentWidget);
}

//show certain page
void showPage(int16_t page) {
  //if different page clear all
  if (page != currentPage) {
    tft.fillScreen(ILI9341_BLACK);
    currentPage = page;
  }
  if (currentPage < 0) {
    switch (currentPage) {
      case -1 : showSystemSetup(); break;
      case -2 : showDeviceSetup(); break;
      case -3 : showCurrentWidgetSetup(); break;
    }
  }else{
    showTimeBar();
    showResults();
    showStatusBar();
  }
}


//show current page
void showCurrentPage() {
  showPage(currentPage);
}
 
