//Version 0.1
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include <Arduino.h>
#include <AT_MessageBuffer.h>
#include "AT_Database.h"
#include "FS.h"
#include "SPIFFS.h"



AT_Database::AT_Database() {
  //init the database
  uint16_t i;
  for (i = 0; i<ATMAXCHANNELS; i++) _results[i].valid = 0;
  for (i = 0; i<ATMAXDEVICE; i++) _devices[i].activ = 0;
  for (i = 0; i<ATMAXPAGES; i++) {
    for (uint8_t j = 0; j<ATWIDGETSPERPAGE; j++) _pages[i].widgets[j].visible = 0;
  }
}

boolean AT_Database::readConfig(String fileName) {
  uint16_t i = 0;
  uint8_t j = 0;
  String tmp;
  char hex[3];
  if (!SPIFFS.exists(fileName)) {
    //does not exist create
    return writeConfig(fileName);
  }
  File f = SPIFFS.open(fileName, "r");
  if (!f) {
    return false;
  }
  while (f.available() && (i<ATMAXPAGES)) {
    while (f.available() && (j<ATWIDGETSPERPAGE))
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].visible = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].source = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].size = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].bgcolor = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].fontcolor = tmp.toInt();
      tmp = f.readStringUntil('\n');
      _pages[i].widgets[j].label = tmp;
      i++;
  }
  return true;

}

boolean AT_Database::writeConfig(String fileName) {
  File f = SPIFFS.open(fileName, FILE_WRITE);
  if (!f) return false;
  for (uint16_t i = 0; i<ATMAXPAGES; i++) {
    for (uint8_t j = 0; j<ATWIDGETSPERPAGE; j++) {
      f.print(_pages[i].widgets[j].visible);f.print('\n');
      if (_pages[i].widgets[j].visible) {
        f.print(_pages[i].widgets[j].source);f.print('\n');
        f.print(_pages[i].widgets[j].size);f.print('\n');
        f.print(_pages[i].widgets[j].bgcolor);f.print('\n');
        f.print(_pages[i].widgets[j].fontcolor);f.print('\n');
        f.print(_pages[i].widgets[j].label);f.print('\n');
      } else {
        f.printf("0\n0\n-65535\n0\n-\n");
      }
    }
  }
  return true;

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

boolean AT_Database::registerDev(String deviceId, uint16_t devicebits) {
  uint8_t id[6];
  sscanf(deviceId.c_str(), "%x:%x:%x:%x:%x:%x%c",  &id[0], &id[1], &id[2], &id[3], &id[4], &id[5] );
  uint16_t i = 0;
  while ((i<ATMAXDEVICE) && (_devices[i].activ == 1)) i++;
  if (i >= ATMAXDEVICE) return false;
  for (uint8_t j = 0; j<6; j++) _devices[i].id[j] = id[j];
  _devices[i].activ = 1;
  _devices[i].service = 0;
  _devices[i].devicebits = devicebits;
  return true;
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
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return sttime;
  }
  strftime(sttime, sizeof(sttime), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return sttime;
}
