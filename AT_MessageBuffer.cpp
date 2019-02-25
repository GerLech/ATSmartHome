//Version 0.1
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

#include <Arduino.h>

#include "AT_MessageBuffer.h"

AT_MessageBuffer::AT_MessageBuffer() {
  _messageBuffer.packets = 0; //clear packetcount
}

void AT_MessageBuffer::setId(int id[6]) {
  for (uint8_t i = 0; i<6; i++) _messageBuffer.id[i] = id[i];
}

void AT_MessageBuffer::setId(String id){
  int imac[6];
  sscanf(id.c_str(), "%x:%x:%x:%x:%x:%x%c",  &imac[0], &imac[1], &imac[2], &imac[3], &imac[4], &imac[5] );
  for (uint8_t i = 0; i<6; i++) _messageBuffer.id[i]=imac[i];
}

ATMESSAGEHEADER AT_MessageBuffer::getHeader() {
  ATMESSAGEHEADER result;
  for (uint8_t i = 0; i<6; i++) result.id[i] = _messageBuffer.id[i];
  return result;
}
void AT_MessageBuffer::clear(){
  _messageBuffer.packets = 0;
}

void AT_MessageBuffer::addFloat(float value, uint8_t channel, uint8_t unit = 0) {
  if (_messageBuffer.packets < ATMAXDEVCHANNELS){
    uint8_t index = _messageBuffer.packets;
    _dataPackets[index].channel = channel;
    _dataPackets[index].type = ATTYPE_ANALOGOUT;
    _dataPackets[index].unit = unit;
    Serial.println(value);
    memcpy(&_dataPackets[index].value[0],&value,4);
    _messageBuffer.packets++;
  }
}

void AT_MessageBuffer::addLong(long value, uint8_t channel, uint8_t unit = 0) {
  if (_messageBuffer.packets < ATMAXDEVCHANNELS){
    uint8_t index = _messageBuffer.packets;
    _dataPackets[index].channel = channel;
    _dataPackets[index].type = ATTYPE_ANALOGOUT;
    _dataPackets[index].unit = unit;
    Serial.println(value);
    memcpy(&_dataPackets[index].value[0],&value,4);
    _messageBuffer.packets++;
  }
}

void AT_MessageBuffer::addCelsius(float value, uint8_t channel) {
  addFloat(value, channel, ATUNIT_CELSIUS);
}

boolean AT_MessageBuffer::fillBuffer(uint8_t * buffer, uint8_t * size){
  uint8_t max_size = * size;
  uint8_t sz = sizeof(_messageBuffer.id) + sizeof(_messageBuffer.packets);
  *size = sz;
  if (sz <= max_size) memcpy(buffer, & _messageBuffer, sz); else return false;
  uint8_t szp = sizeof(ATDATAPACKET);
  for (uint8_t i = 0; i<_messageBuffer.packets; i++) {
    if ((sz + szp) <= max_size) memcpy(buffer + sz, &_dataPackets[i], szp);
    sz += szp;
  }
  * size = sz;
  return (sz <= max_size);
}

void AT_MessageBuffer::readBuffer(const uint8_t * buffer){
  uint8_t sz = sizeof(_messageBuffer.id) + sizeof(_messageBuffer.packets);
  memcpy(& _messageBuffer, buffer, sz);
  if (_messageBuffer.packets > ATMAXDEVCHANNELS) _messageBuffer.packets = ATMAXDEVCHANNELS;
  for (uint8_t i = 0; i<_messageBuffer.packets; i++) {
    memcpy(& _dataPackets[i], buffer+sz, sizeof(ATDATAPACKET));
    sz += sizeof(ATDATAPACKET);
  }
}

uint8_t AT_MessageBuffer::getPackets() {
  return _messageBuffer.packets;
}

ATDATAPACKET AT_MessageBuffer::getData(uint8_t index) {
  if (index < _messageBuffer.packets) {
    return _dataPackets[index];
  }
}

float AT_GetFloat(uint8_t * data) {
  float value;
  memcpy(&value,data,4);
  return value;
}

long AT_GetLong(uint8_t * data){
  long value;
  memcpy(&value,data,4);
  return value;
}
