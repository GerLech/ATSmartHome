//Version 0.5
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
#include <AT_LanguageGE.h>


#include <Arduino.h>
#include <AT_Database.h>
#include "AT_Webserver.h"
#include "WebServer.h"

const PROGMEM char AT_WIDGET_TEMPLATE[] =
"<button type='submit' name='widget' value=%i style=\"border:none;background-color:#%06x;border-color:#%06x;color:#%06x;font-size:%ipt;font-weight:%s;text-align:center;width:%i%%;float:%s;\">\n"
"<div style='padding:5px;'>\n";

const PROGMEM char HTML_HEADER[] =
"<!DOCTYPE HTML>\n"
"<html>\n"
"<head>\n"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0>\">\n"
"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">\n"
"<title>Smart Home</title>\n"
"<style>\n"
"body { background-color: #d2f3eb; font-family: Arial, Helvetica, Sans-Serif; Color: #000000;font-size:12pt;width:800px; }\n"
"th { background-color: #b6c0db; color: #050ed2;font-weight:lighter;font-size:10pt;}\n"
"table, th, td {border: 1px solid black;}\n"
".titel {font-size:18pt;font-weight:bold;text-align:center;} \n"
"</style>\n";

const PROGMEM char HTML_HEADER_END[] =
"</head>\n"
"<body><div id='main_div' style='margin-left:15px;margin-right:15px;'>\n";
const PROGMEM char HTML_END[] =
"</body></html>\n";
const PROGMEM char HTML_SCRIPT[] =
"<script language=\"javascript\">\n"
"function reload() {\n"
"document.location=\"http://%s\";}\n"
"function setWidth() {\n"
  "var x = screen.width;\n"
  "if (x<800) document.getElementById('main_div').style.width = (x-50) + 'px';}\n"
"</script>\n";
const PROGMEM char HTML_END_RELOAD[] =
"</div><script language=\"javascript\">setWidth(); setTimeout(reload, %i);</script></body>\n"
"</html>\n";



AT_Webserver::AT_Webserver(WebServer * srv, AT_Database *database) {
  _server = srv;
  _database = database;
}

void AT_Webserver::begin() {
  _server->begin();
}

void AT_Webserver::handleClient(){
  _server->handleClient();
}

void AT_Webserver::handleRoot(uint8_t refresh){
  //first check arguments to perform actions
  if (_server->hasArg("widget")){
    uint16_t id = _server->arg("widget").toInt();
    ATCURVALUES res = _database->getResult(id);
    if (res.type == ATTYPE_SWITCHOUT) {
      _database->toggleResult(id);
      if (_onResultChange) _onResultChange(id);
    }
  }
  char tmp1[20];
  char htmlbuf[256];
  _server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  //Header
  _server->send(200, "text/html",HTML_HEADER);
  //IP Adresse for reload script
  WiFi.localIP().toString().toCharArray(tmp1,20);
  sprintf(htmlbuf,HTML_SCRIPT,tmp1);
  _server->sendContent(htmlbuf);
  _server->sendContent(HTML_HEADER_END);
  _server->sendContent("<div class=\"titel\">ArduiTouch Smart&nbsp;Home</div>\n");

  sendResults();
  //Das Formular mit den Eingabefeldern wird mit den aktuellen Werten befüllt
  //sprintf(htmlbuf,HTML_CONFIG,txtSSID,txtPassword,txtUser,txtPwd,txtId);
  //und an den Browsewr gesendet
  //server->sendContent("<h1>SMART Home</h1>");
  sprintf(htmlbuf,HTML_END_RELOAD,refresh*1000);
  _server->sendContent(htmlbuf);

}

void AT_Webserver::handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += _server->uri();
  message += "\nMethod: ";
  message += (_server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += _server->args();
  message += "\n";
  for (uint8_t i = 0; i < _server->args(); i++) {
    message += " " + _server->argName(i) + ": " + _server->arg(i) + "\n";
  }
  _server->send(404, "text/plain", message);
}

uint32_t AT_Webserver::convertColor(uint16_t color565) {
  uint8_t r = (color565 >> 11)*8;
  uint8_t g = ((color565 >> 5) & 0x3f)*4;
  uint8_t b = (color565 & 0x1f)*8;
  return r*65536+g*256+b;
}

//show a list of results with custom widgets
void AT_Webserver::sendResults(){
  String msg;
  uint16_t x,y;
  x = 0; y = 0;
  ATDISPLAYPAGE pg;
  ATDISPLAYWIDGET wdg;
  _server->sendContent("<form method='post'>\n");
  for (uint8_t p = 0; p<ATMAXPAGES; p++) {
    pg = _database->getPage(p);
    for (uint8_t i = 0;i<ATWIDGETSPERPAGE;i++) {
      wdg = pg.widgets[i];
      if (wdg.status == ATWIDGET_USED) {
        switch (wdg.type) {
          case ATWIDGET_SIMPLE: sendSimpleWidget(wdg); break;
        }
      }
    }
  }
  _server->sendContent("</form>\n");

}

//show a simple widget
void AT_Webserver::sendSimpleWidget(ATDISPLAYWIDGET wdg){
  char htmlbuf[256];
  const char * flo[] = {"inline-start","left","right"};
  const char * weight[] = {"normal","bold"};
  String val;
  String msg;
  uint16_t fill;
  uint8_t width;
  uint8_t iflo;
  uint8_t iwgt;
  fill = wdg.bgcolor;
  if (_database->isSwitchOut(wdg.source)) {
    val = ATTXTOFF;
    if (_database->getBooleanValue(wdg.source)==0) {
      fill = wdg.bgcolorOn;
      val = ATTXTON;
    }
  } else {
    val = _database->getValueString(wdg.source,wdg.precision,true);
  }
  iflo = 0;
  iwgt = (wdg.size == ATWIDGET_SMALL)?0:1;
  width = 100;
  switch (wdg.size) {
    case ATWIDGET_LEFT: iflo=1; width = 50; break;
    case ATWIDGET_RIGHT: iflo=2; width = 50; break;
  }
  sprintf(htmlbuf,AT_WIDGET_TEMPLATE,wdg.source,convertColor(fill),convertColor(wdg.bgcolor),convertColor(wdg.fontcolor),15,weight[iwgt],width,flo[iflo]);
  _server->sendContent(htmlbuf);
  if (wdg.size == ATWIDGET_SMALL) {
    msg = wdg.label + " " + val;
    _server->sendContent(msg);
  } else
  {
    _server->sendContent(wdg.label+"<br>");
    String msg = wdg.label+" "+val;
    _server->sendContent(val);
  }
  _server->sendContent("</div></button>\n");
}
//register a callback function result change event
void AT_Webserver::registerOnResultChange(void (*callback)(uint16_t index)){
  _onResultChange = callback;
}
