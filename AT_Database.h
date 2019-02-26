/*
||
|| @file AT_Database.h
|| @version 0.1
|| @author Gerald Lechner
|| @contact lechge@gmail.com
||
|| @description
|| |This library defines structures an functions to manage devices, results
|| |forms and configurations in a smart home control center
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
*/

#include <Arduino.h>
#include <AT_MessageBuffer.h>


#ifndef AT_Database_h
#define AT_Database_h

#define ATMAXCHANNELS 256 //max number of channels
#define ATMAXDEVICE 32 //max number of devices
#define ATMAXDEVCHAN 8 //max number channels per device
#define ATMAXPAGES 32 //max number of result pages
#define ATWIDGETSPERPAGE 8 //number of widgets to be placed on a result page

typedef  //structure to hold current values
struct ATCURVALUES {
  uint8_t valid;  //the value is valid
  uint8_t step;   //processing step 1 means updated
  uint8_t type;   //type of the data see AT_MessageBuffer.h
  uint8_t unit;   //unit of the data see AT_MessageBuffer.h
  uint8_t value[4]; //four data bytes to hold a float or a long integer
};

typedef //structure to  hold a device definition
struct ATDEVICE {
  uint8_t activ = 0;        //the device is in use
  uint8_t service = 0;      //0=ESP-Now
  uint8_t id[6] = {0,0,0,0};//id of the device
  String name = "";         //name of the device
  String last = "";         //timestamp of last receiving data
};

typedef //structure to hold a widget setup
struct ATDISPLAYWIDGET {
  uint8_t source;      //link to the associated result
  uint8_t size;        //size 0=240x30,1=120x60 left 2=120x60 right 3=240x60
  uint16_t bgcolor;    //fill color
  uint16_t fontcolor;  //font color
  uint8_t visible;     //flag to switch visibility
  String label = "";   //label
};

typedef //struct to hold widgets per page
struct ATDISPLAYPAGE {
  ATDISPLAYWIDGET widgets[ATWIDGETSPERPAGE];
};

//convers id to a string with format xx:xx:xx:xx:xx:xx
String AT_GetId(uint8_t id[6]);
//returns the current date and time as a string (not working yet)
String AT_GetLocalTime();

class AT_Database {
public:
  AT_Database();
  //read the widget configuration from SPIFFS
  boolean readConfig(String fileName);
  //write the widget configuration into a SPIFFS file
  boolean writeConfig(String fileName);
  //read the device list from SPIFFS file
  boolean readDevices(String fileName);
  //write the device list into a SPIFFS file
  boolean writeDevices(String fileName);
  //return the index for a device with a certain id or -1 if not Foundation
  int16_t findDevice(uint8_t id[6]);
  //set result from a message buffer data packets
  //index = device index x channels per device + channel number
  void setResult(uint8_t index, ATDATAPACKET data);
  //register a device with the device id as string with format xx:xx:xx:xx:xx:xx
  boolean registerDev(String deviceId);
  //get the pointer on a result data structure
  //index = device index x channels per device + channel number
  ATCURVALUES getResult(uint16_t index);
  //set the step value for a result
  //index = device index x channels per device + channel number
  void setStep(uint8_t step, uint16_t index);

private:
  ATCURVALUES _results[ATMAXCHANNELS]; //array to hold results
  ATDEVICE _devices[ATMAXDEVICE];      //array to hold devices
  ATDISPLAYPAGE _pages[ATMAXPAGES];    //array to hold widget configurations
};

#endif
