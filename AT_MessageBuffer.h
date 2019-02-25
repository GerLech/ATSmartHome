/*
||
|| @file AT_Messagebuffer.h
|| @version 0.1
|| @author Gerald Lechner
|| @contact lechge@gmail.com
||
|| @description
|| |This library defines structures an functions to send data from and to
|| |remote sensors. It defines a buffer to collect packages and send it as a block
|| #
||
|| @license
|| | This library is free software; you can redistribute it and/or
|| | modify it under the terms of the GNU Lesser General Public
|| | License as published by the Free Software Foundation; version
|| | 2.1 of the License.
|| |
|| | This library is distributed in the hope that it will be useful,
|| | but WITHOUT ANY WARRANTY; without even the implied warranty of
|| | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
|| | Lesser General Public License for more details.
|| |
|| | You should have received a copy of the GNU Lesser General Public
|| | License along with this library; if not, write to the Free Software
|| | Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
|| #
||
*/#include <Arduino.h>


#ifndef AT_MessageBuffer_h
#define AT_MessageBuffer_h

#define ATMAXDEVCHANNELS 8 //max number channels per device

//types of data packages
#define ATTYPE_DIGITALIN 0  //integer value from sensor
#define ATTYPE_ANALOGIN 1  //float value from sensor
#define ATTYPE_DIGITALOUT 2  //integer value to sensor
#define ATTYPE_ANALOGOUT 3 //float value to sensor

//units to classify values
#define ATUNIT_NONE 0        //no unit
#define ATUNIT_CELSIUS 1     //temperature in °C
#define ATUNIT_FAHRENHEIT 2  //temperature in °F
#define ATUNIT_PERCENT 3     //percent
#define ATUNIT_PASCAL 4      //pressure in Pa
#define ATUNIT_LUX 5         //illuminance in lx
#define ATUNIT_METER 6       //length in m


typedef //structure of a data packet
struct ATDATAPACKET {
  uint8_t channel; //channel as an identification
  uint8_t type;    //type of the packet
  uint8_t unit;    //unit of the packet
  uint8_t value[4];//four data bytes to hold a long integer or a float
};

typedef //structure of a message packet
struct ATMSGPACKET {
  uint8_t id[6];    //id of the remote device typical its MAC address
  uint8_t packets;  //number of packets in the message
};

typedef
struct ATMESSAGEHEADER {
  uint8_t id[6];
};

//returns the value from data packet as a float
float AT_GetFloat(uint8_t * data);
//returns the value from a data packet as a long integer
long AT_GetLong(uint8_t * data);

class AT_MessageBuffer {
public:
  AT_MessageBuffer();
  //Set the Id from array
  void setId(int id[6]);
  //Set the Id from a string with format xx:xx:xx:xx:xx:xx
  void setId(String id);
  //Return the message header
  ATMESSAGEHEADER getHeader();
  //Clear messagebuffer
  void clear();
  //add a float to the messagebuffer
  void addFloat(float value, uint8_t channel, uint8_t unit);
  //add a long integer to the messagebuffer
  void addLong(long value, uint8_t channel, uint8_t unit);
  //add temperature as °C to the message buffer
  void addCelsius(float value, uint8_t channel);
  //fill a memory block with data from messagebuffer
  //the parameter buffer points to the memory block
  //the parameter size points to an integer with the size of the memory block
  //after call contains size the real used part of the memory block
  //it returns false if messagebuffer does not fit in memory block
  boolean fillBuffer(uint8_t * buffer, uint8_t * size);
  //read message data from a memory block
  //the parameter buffer points to the memory block
  //the function assumes that the memory block contains the correct structure
  //ATMESSAGEPACKET followed by ATDATAPACKETs
  void readBuffer(const uint8_t * buffer);
  //return the number of packets in the messagebuffer
  uint8_t getPackets();
  //return a pointer on a data packet in the messagebuffer
  ATDATAPACKET getData(uint8_t index);


private:
  ATMSGPACKET _messageBuffer; //header of the messagebuffer
  ATDATAPACKET _dataPackets [8]; //array of data packets
};

#endif
