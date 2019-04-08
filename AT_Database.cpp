//Version 0.4
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include <Arduino.h>
#include <AT_MessageBuffer.h>
#include "AT_Database.h"
#include "FS.h"
#if defined(ESP32)
  #include "SPIFFS.h"
  #define ATUSEESP32
#else
  #include "ESP8266WiFi.h"
  #define FILE_READ       "r"
  #define FILE_WRITE      "w"
  #define FILE_APPEND     "a"
#endif

#define CONFVERSION "V1.0"

AT_Database::AT_Database(String deviceFile, String confFile) {
  //init the database
  uint16_t i;
  _deviceFile = deviceFile;
  _confFile = confFile;
  for (i = 0; i<ATMAXCHANNELS; i++) _results[i].valid = 0;
  for (i = 0; i<ATMAXDEVICE; i++) _devices[i].activ = 0;
  for (i = 0; i<ATMAXPAGES; i++) {
    for (uint8_t j = 0; j<ATWIDGETSPERPAGE; j++) _pages[i].widgets[j].status = ATWIDGET_UNUSED;
  }
}
boolean AT_Database::readConfig() {
  return readConfig(_confFile);
}

boolean AT_Database::readConfig(String fileName) {
  uint16_t i = 0;
  uint8_t j = 0;
  boolean old = false;
  String tmp;
  String version;
  char hex[3];
  if (!SPIFFS.exists(fileName)) {
    //does not exist create
    return writeConfig(fileName);
  }
  File f = SPIFFS.open(fileName, "r");
  if (!f) {
    return false;
  }
  version = f.readStringUntil('\n');
  if (version != CONFVERSION) old = true;
  while (f.available() && (i<ATMAXPAGES)) {
    j=0;
    while (f.available() && (j<ATWIDGETSPERPAGE)) {
      if (old){
        //we have a config without version info
        tmp=version;
        old = false;
      } else {
        tmp = f.readStringUntil('\n');
      }
      _pages[i].widgets[j].status = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].source = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].type = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].size = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].bgcolor = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].bgcolorOn = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].fontcolor = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].image = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].precision = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].label = tmp;
      if (version == CONFVERSION) {
        tmp = f.readStringUntil('\n');
        _pages[i].widgets[j].val1 = tmp.toFloat();
        tmp = f.readStringUntil('\n');
        _pages[i].widgets[j].color1 = tmp.toInt();
        tmp = f.readStringUntil('\n');
        _pages[i].widgets[j].val2 = tmp.toFloat();
        tmp = f.readStringUntil('\n');
        _pages[i].widgets[j].color2 = tmp.toInt();
        tmp = f.readStringUntil('\n');
        _pages[i].widgets[j].val3 = tmp.toFloat();
        tmp = f.readStringUntil('\n');
        _pages[i].widgets[j].color3 = tmp.toInt();
      } else {
        _pages[i].widgets[j].val1 = 0;
        _pages[i].widgets[j].color1 = 0;
        _pages[i].widgets[j].val2 = 0;
        _pages[i].widgets[j].color2 = 0;
        _pages[i].widgets[j].val3 = 0;
        _pages[i].widgets[j].color3 = 0;
      }

      j++;
    }
    i++;
  }
  return true;

}

boolean AT_Database::writeConfig() {
  return writeConfig(_confFile);
}

boolean AT_Database::writeConfig(String fileName) {
  File f = SPIFFS.open(fileName, FILE_WRITE);
  if (!f) return false;
  f.print("V1.0");f.print('\n');
  for (uint16_t i = 0; i<ATMAXPAGES; i++) {
    for (uint8_t j = 0; j<ATWIDGETSPERPAGE; j++) {
      f.print(_pages[i].widgets[j].status);f.print('\n');
      f.print(_pages[i].widgets[j].source);f.print('\n');
      f.print(_pages[i].widgets[j].type);f.print('\n');
      f.print(_pages[i].widgets[j].size);f.print('\n');
      f.print(_pages[i].widgets[j].bgcolor);f.print('\n');
      f.print(_pages[i].widgets[j].bgcolorOn);f.print('\n');
      f.print(_pages[i].widgets[j].fontcolor);f.print('\n');
      f.print(_pages[i].widgets[j].image);f.print('\n');
      f.print(_pages[i].widgets[j].precision);f.print('\n');
      f.print(_pages[i].widgets[j].label);f.print('\n');
      f.print(_pages[i].widgets[j].val1);f.print('\n');
      f.print(_pages[i].widgets[j].color1);f.print('\n');
      f.print(_pages[i].widgets[j].val2);f.print('\n');
      f.print(_pages[i].widgets[j].color2);f.print('\n');
      f.print(_pages[i].widgets[j].val3);f.print('\n');
      f.print(_pages[i].widgets[j].color3);f.print('\n');
    }
  }
  return true;

}

boolean AT_Database::readDevices() {
  return readDevices(_deviceFile);
}

boolean AT_Database::readDevices(String fileName) {
  uint16_t i = 0;
  String tmp;
  char hex[3];
  if (!SPIFFS.exists(fileName)) {
    //does not exist create
    Serial.println("Create device file");
    return writeDevices(fileName);
  }
  File f = SPIFFS.open(fileName, "r");
  if (!f) {
    return false;
  }
  while (f.available() && (i<ATMAXDEVICE)) {
    tmp = f.readStringUntil('\n');
    _devices[i].activ = tmp.toInt();
    tmp = f.readStringUntil('\n');
    _devices[i].service = tmp.toInt();
    tmp = f.readStringUntil('\n');
    Serial.println(tmp);
    for (uint8_t j=0; j<6; j++){
      hex[0]=tmp[j*3];
      hex[1]=tmp[j*3+1];
      hex[2]=0;
      _devices[i].id[j]= (byte) strtol(hex,NULL,16);
    }
    tmp = f.readStringUntil('\n');
    _devices[i].name = tmp;
    tmp = f.readStringUntil('\n');
    _devices[i].last = tmp;
    i++;
  }
  return true;
}

boolean AT_Database::writeDevices() {
  return writeDevices(_deviceFile);
}

boolean AT_Database::writeDevices(String fileName) {
  File f = SPIFFS.open(fileName, FILE_WRITE);
  if (!f) return false;
  for (uint16_t i = 0; i<ATMAXDEVICE; i++) {
    f.print(_devices[i].activ);f.print('\n');
    if (_devices[i].activ) {
      f.print(_devices[i].service);f.print('\n');
      Serial.println(AT_GetId(_devices[i].id));
      f.print(AT_GetId(_devices[i].id));f.print('\n');
      f.print(_devices[i].name);f.print('\n');
      f.print(_devices[i].last);f.print('\n');
    } else {
      f.printf("0\n00:00:00:00:00:00\n-\n-\n");
    }
  }
  return true;
}

int16_t AT_Database::findDevice(uint8_t id[6]) {
  uint8_t j;
  int16_t i = 0;
  boolean found = false;
  do {
    j = 0;
    if (_devices[i].activ == 0) {
      i++;
    } else {
      while ((j < 6) && (id[j] == _devices[i].id[j])) {j++;}
      found = (j == 6);
      if (!found) i++;
    }
  } while ((i<ATMAXDEVICE) && (!found));
  if (found) {return i;} else {return -1;}

}

void AT_Database::setResult(uint8_t index, ATDATAPACKET data) {
  _results[index].valid = 1;
  _results[index].step = 1;
  _results[index].type = data.type;
  _results[index].unit = data.unit;
  for (uint8_t j = 0; j<4; j++) _results[index].value[j] = data.value[j];
}

int8_t AT_Database::registerDev() {
  return registerDev(_newDevice, _newMsg);
}

int8_t AT_Database::registerDev(String deviceId, AT_MessageBuffer msg) {
  ATMESSAGEHEADER mh = msg.getHeader();
  uint16_t i = 0;
  uint16_t page, slot, dev;
  uint16_t channels, source;
  ATDISPLAYWIDGET * wdg;
  ATDATAPACKET dp;
  Serial.println("Start search");
  while ((i<ATMAXDEVICE) && (_devices[i].activ == 1)) i++;
  if (i >= ATMAXDEVICE) return -1;
  //save the device
  for (uint8_t j = 0; j<6; j++) _devices[i].id[j] = mh.id[j];
  _devices[i].activ = 1;
  _devices[i].service = 0;
  _devices[i].devicebits = msg.getDeviceBits();
  //setup a default widget for all channels
  dev = i; //devicenummer
  page = 0;
  channels = msg.getPackets();
  if (channels >= ATMAXDEVCHAN) channels = ATMAXDEVCHAN - 1;
  for (i=0; i<channels; i++) {
    Serial.printf("Try channel %i \n",i);
    dp = msg.getData(i);
    source = dev * ATMAXDEVCHAN + dp.channel;
    setResult(source,dp);
    do {
      slot = getFreeSlot(page, ATWIDGET_SMALL);
      if (slot<0) {
        page++;
      } else {
        Serial.printf("Slot %i get result %i\n",slot,source);
        wdg = &_pages[page].widgets[slot];
        wdg->status = ATWIDGET_USED;
        wdg->source = source;
        wdg->size = ATWIDGET_SMALL;
        wdg->type = ATWIDGET_SIMPLE;
        wdg->bgcolorOn = 0x878f;
        wdg->fontcolor = 0x0000;
        wdg->precision = 2;
        if (dp.type == ATTYPE_SWITCHOUT) {
          wdg->bgcolor = 0xd699;
          wdg->label="Switch "+String(i);
        } else {
          wdg->bgcolor = 0xFFF2;
          wdg->label="Channel "+String(i);
        }
        wdg->val1 = 0;
        wdg->color1=0;
        wdg->val2 = 0;
        wdg->color2=0;
        wdg->val3 = 0;
        wdg->color3=0;
        wdg->image = 0;
      }
    } while ((slot < 0) && (page < ATMAXPAGES));
  }
  _newDevice = ""; //signal it was registered
  return dev;
}

int8_t AT_Database::getResponse(int16_t device, uint8_t * buffer, uint8_t * size){
  uint8_t max = * size;
  ATMSGPACKET head;
  ATDATAPACKET data;
  uint8_t sz = 0;
  uint8_t i,j;
  uint16_t base;
  //first we copy the header for the device
  for (i = 0; i< 6; i++) head.id[i] = _devices[device].id[i];
  head.packets = 0;
  head.devicebits = _devices[device].devicebits;
  sz = sizeof(head);
  //now we check if we have channels with data to send
  base = device * ATMAXDEVCHANNELS;
  for (i=0; i<ATMAXDEVCHANNELS; i++) {
    if ((_results[base+i].valid == 1) && (_results[base+i].step == 2) &&
      ((_results[base+i].type == ATTYPE_ANALOGOUT) ||
      (_results[base+i].type == ATTYPE_DIGITALOUT))) {
      head.packets ++;
      data.type = _results[base+i].type;
      data.unit = _results[base+i].unit;
      data.channel = i;
      for (j=0; j<4; j++) data.value[j] = _results[base+i].value[j];
      memcpy(buffer+sz,&data,sizeof(data));
      sz += sizeof(data);
    }
  }
  //finally we copy the header with the correct packet number
  memcpy(buffer,&head,sizeof(head));
  * size = sz;
  return head.packets;
}

ATCURVALUES AT_Database::getResult(uint16_t index) {
  if (index<ATMAXCHANNELS) return _results[index];
}

void AT_Database::setStep(uint8_t step, uint16_t index) {
  if (index<ATMAXCHANNELS) _results[index].step = step;
}

String AT_GetId(uint8_t id[6]){
  String stid;
  char tmp[4];
  sprintf(tmp,"%02x",id[0]);
  stid=tmp;
  for (uint8_t j = 1; j<6; j++) {
    sprintf(tmp,":%02x",id[j]);
    stid = stid += tmp ;
  }
  return stid;
}

String AT_GetLocalTime() {
  char sttime[20] = "No Time available!";
  #ifdef ATUSEESP32
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      return sttime;
    }
    strftime(sttime, sizeof(sttime), "%Y-%m-%d %H:%M:%S", &timeinfo);
  #endif
  return sttime;
}

int16_t AT_Database::getFreeSlot(uint8_t size) {
  uint8_t page = 0;
  uint8_t slot = 0;
  do {
    slot = getFreeSlot(page, ATWIDGET_SMALL);
    if (slot<0) {
      page++;
    }
  } while ((slot < 0) && (page < ATMAXPAGES));
  if (slot < 0) return -1;
  return page*ATWIDGETSPERPAGE+slot;
}

int16_t AT_Database::getFreeSlot(uint8_t page, uint8_t size) {
  uint8_t i, result;
  boolean found = false;
  ATDISPLAYWIDGET w;
  uint8_t matrix[9]; //9 elements to avoid out of range error
  if (page > ATMAXPAGES) return -1;
  for (i = 0;i<8; i++) matrix[i] = 0;
  for (i = 0; i<ATWIDGETSPERPAGE; i++) {
    w = _pages[page].widgets[i];
    if (w.status != ATWIDGET_UNUSED) {
      switch (w.size) {
        case ATWIDGET_SMALL: matrix[i]+=3; break;
        case ATWIDGET_LEFT: matrix[i]+=1; matrix[i+1]+=1; break;
        case ATWIDGET_RIGHT: matrix[i+1]+=2; matrix[i]+=2; break;
        case ATWIDGET_BIG: matrix[i]+=3; matrix[i+1]+=3; break;
      }
    }
  }
  i=0; found = false; result = 0;
  while (!found && (i<ATWIDGETSPERPAGE)) {
    if ((size == ATWIDGET_SMALL) && (matrix[i] == 0)) {
      found = true; result=i;
    }
    if ((size == ATWIDGET_BIG) && (i<7) && (matrix[i] == 0) && (matrix[i+1]==0)) {
      found = true; result=i;
    }
    if ((size == ATWIDGET_LEFT) || (size == ATWIDGET_RIGHT)){
      if ((i<7) && (matrix[i] == 2) && (matrix[i+1]==2)) {
        found = true; result=i;
      }
      if ((i<7) && (matrix[i] == 1) && (matrix[i+1]==1)) {
        found = true; result=i+1;
      }
    }
    i++;
  }
  if (!found) result = -1;
  return result;
}
String AT_Database::getValueString(uint16_t index, uint8_t precision, boolean useunit ) {
  ATCURVALUES data;
  int32_t ival;
  char fmt[] = "%10.0f";
  char buf[30];
  float fval;
  String result;
  if (index > ATMAXCHANNELS) index = ATMAXCHANNELS-1;
  data = _results[index];
  if ((data.type = ATTYPE_ANALOGOUT) || (data.type = ATTYPE_ANALOGOUT)) {
    //float values
    if (precision > 9) precision = 9;
    fval = AT_GetFloat(&data.value[0]);
    fmt[4]=char(precision+48);
    sprintf(buf,fmt,fval);
    result = buf;
  } else {
    //integer values
    ival = AT_GetLong(&data.value[0]);
    result = String(ival);
  }
  if (useunit) result += " "+AT_getUnitString(data.unit);
  return result;
}

uint8_t AT_Database::getBooleanValue(uint16_t index){
  if (index > ATMAXCHANNELS) index = ATMAXCHANNELS-1;
  return _results[index].value[0];
}


boolean AT_Database::isValueOutput(uint16_t index){
  if (index > ATMAXCHANNELS) index = ATMAXCHANNELS-1;
  return ((_results[index].type == ATTYPE_ANALOGOUT) ||
          (_results[index].type == ATTYPE_DIGITALOUT) ||
          (_results[index].type == ATTYPE_SWITCHOUT));
}

boolean AT_Database::isSwitchOut(uint16_t index) {
  if (index > ATMAXCHANNELS) index = ATMAXCHANNELS-1;
  return ((_results[index].type == ATTYPE_SWITCHOUT));
}

boolean AT_Database::isValueZero(uint16_t index){
  ATCURVALUES data;
  int32_t ival=0;
  float fval=0;
  if (index > ATMAXCHANNELS) index = ATMAXCHANNELS-1;
  data = _results[index];
  if ((data.type = ATTYPE_ANALOGOUT) || (data.type = ATTYPE_ANALOGOUT)) {
    //float values
    fval = AT_GetFloat(&data.value[0]);
  } else {
    //integer values
    ival = AT_GetLong(&data.value[0]);
  }
  return (ival==0) && (fval==0);
}

//check if a result is valid
boolean AT_Database::isValid(uint16_t index) {
  if (index > ATMAXCHANNELS) return false;
  return (_results[index].valid == 1);
}

//getPage returns the specified display page
ATDISPLAYPAGE AT_Database::getPage(uint8_t page) {
  if (page >= ATMAXPAGES) page = ATMAXPAGES - 1;
  return _pages[page];
}

//getWidget returns the widget in slot on page
ATDISPLAYWIDGET AT_Database::getWidget(uint8_t page, uint8_t slot){
  if (page >= ATMAXPAGES) page = ATMAXPAGES - 1;
  if (slot >= ATWIDGETSPERPAGE) slot = ATWIDGETSPERPAGE - 1;
  return _pages[page].widgets[slot];
}

ATDISPLAYWIDGET *  AT_Database::getWidgetAdr(uint8_t page, uint8_t slot){
  return &_pages[page].widgets[slot];
}



boolean AT_Database::clearDevices() {
  for (uint8_t i = 0; i<ATMAXDEVICE; i++) deleteDevice(i);
  if (writeConfig()) {
    return writeDevices();
  } else {
    return false;
  }
}

//delete one device
boolean AT_Database::deleteDevice(uint8_t device){
  _devices[device].activ = 0;
  deleteWidgetsForDevice(device);
}


void AT_Database::toggleResult(uint16_t index){
  if (index > ATMAXCHANNELS) index = ATMAXCHANNELS-1;
   _results[index].value[0] = (_results[index].value[0] == 0)?1:0;
}

String AT_Database::getDeviceId(uint8_t device){
  if (device >= ATMAXDEVICE) return "";
  String stid;
  char tmp[4];
  sprintf(tmp,"%02x",_devices[device].id[0]);
  stid=tmp;
  for (uint8_t j = 1; j<6; j++) {
    sprintf(tmp,":%02x",_devices[device].id[j]);
    stid = stid += tmp ;
  }
  return stid;
}

//return the device name or id or - if inaktiv
String AT_Database::getDeviceName(uint8_t device){
  if (device >= ATMAXDEVICE) return "";
  Serial.printf("Device = %i von %i\n",device,ATMAXDEVICE );
  if (_devices[device].activ == 0) return "-";
  if ((_devices[device].name == "")||(_devices[device].name == "-")) return getDeviceId(device);
  return _devices[device].name;
}

//return pointer to a device
ATDEVICE * AT_Database::getDevice(uint8_t device){
  if (device >= ATMAXDEVICE) return 0;
  return &_devices[device];
}

//return true if new device waits for registration
boolean AT_Database::hasNewDevice(){
  return _newDevice != "";
}

//read message data into _newMsg
void AT_Database::readNewMessage(String newDevice, const uint8_t * buffer) {
  _newDevice = newDevice;
  _newMsg.readBuffer(buffer);
}

//return the index = pag* ATWIDGETSPERPAGE + slot for a widget of given source
//return -1 if no widget exists
int16_t AT_Database::findWidgetBySource(uint16_t source){
  boolean found = false;
  uint8_t pg = 0;
  uint8_t slt = 0;
  while ((!found) && (pg < ATMAXPAGES)){
    slt = 0;
    while ((!found) && (slt < ATWIDGETSPERPAGE)) {
      found = (_pages[pg].widgets[slt].source == source) && (_pages[pg].widgets[slt].status != ATWIDGET_UNUSED);
      if (!found) slt++;
    }
    if (!found) pg++;
  }
  if (found){
    return pg*ATWIDGETSPERPAGE+slt;
  } else {
    return -1;
  }
}
//delete all widgets for a device
void AT_Database::deleteWidgetsForDevice(uint8_t device){
  uint16_t minSrc = device * ATMAXDEVCHANNELS;
  uint16_t maxSrc = minSrc + ATMAXDEVCHANNELS;
  uint16_t src;
  for (uint8_t pg = 0; pg<ATMAXPAGES; pg++) {
    for(uint8_t slt = 0; slt < ATWIDGETSPERPAGE; slt++){
      src = _pages[pg].widgets[slt].source;
      if ((src >= minSrc) && (src < maxSrc)) _pages[pg].widgets[slt].status = ATWIDGET_UNUSED;
    }
  }
}

//set the source for a widget
void AT_Database::setWidgetSource(uint16_t widget, uint16_t source){
  uint8_t page = widget/ATWIDGETSPERPAGE;
  uint8_t slot = widget % ATWIDGETSPERPAGE;
  _pages[page].widgets[slot].source = source;
}
