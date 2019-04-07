/*
||
|| @file AT_Database.h
|| @version 0.4
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
#if defined(ESP32)
  #define ATMAXCHANNELS 256 //max number of channels
  #define ATMAXDEVICE 32 //max number of devices
  #define ATMAXDEVCHAN 8 //max number channels per device
  #define ATMAXPAGES 32 //max number of result pages
#else
  #define ATMAXCHANNELS 64 //max number of channels
  #define ATMAXDEVICE 16 //max number of devices
  #define ATMAXDEVCHAN 4 //max number channels per device
  #define ATMAXPAGES 8 //max number of result pages
#endif

#define ATWIDGETSPERPAGE 8 //number of widgets to be placed on a result page

#define ATWIDGET_UNUSED 0
#define ATWIDGET_USED 1
#define ATWIDGET_PLACE 2
#define ATWIDGET_HIDDEN 3

#define ATWIDGET_SMALL 0
#define ATWIDGET_LEFT 1
#define ATWIDGET_RIGHT 2
#define ATWIDGET_BIG 3
#define ATWIDGET_BIG1 4

#define ATWIDGET_SIMPLE 0

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
  uint16_t devicebits = 0;  //capabilities for the device
  String name = "";         //name of the device
  String last = "";         //timestamp of last receiving data
};

typedef //structure to hold a widget setup
struct ATDISPLAYWIDGET {
  uint16_t source;      //index into result list
  uint8_t status;     //status 0=not used, 1=used, 2=placeholder, 3=hidden
  uint8_t size;        //size 0=240x30,1=240x60 left 2=120x60 right 3=120x60
  uint8_t type;        //type 0=simple
  uint16_t bgcolor;    //fill color normal
  uint16_t bgcolorOn;  //fillcolor for buttons on
  uint16_t fontcolor;  //font color
  uint8_t image;       //index to an image
  uint8_t precision;   //precision to show values
  String label = "";   //label
  float val1; //threshold value1
  uint16_t color1; //color associated with value 1
  float val2; //threshold value2
  uint16_t color2; //color associated with value 2
  float val3; //threshold value3
  uint16_t color3; //color associated with value 3
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
  AT_Database(String deviceFile = "/devices.txt", String confFile = "/config.txt");
  //read the widget configuration from SPIFFS
  boolean readConfig(String fileName);
  boolean readConfig();
  //write the widget configuration into a SPIFFS file
  boolean writeConfig(String fileName);
  boolean writeConfig();
  //read the device list from SPIFFS file
  boolean readDevices(String fileName);
  boolean readDevices();
  //write the device list into a SPIFFS file
  boolean writeDevices(String fileName);
  boolean writeDevices();
  //return the index for a device with a certain id or -1 if not Foundation
  int16_t findDevice(uint8_t id[6]);
  //set result from a message buffer data packets
  //index = device index x channels per device + channel number
  void setResult(uint8_t index, ATDATAPACKET data);
  //register a device with the device id as string with format xx:xx:xx:xx:xx:xx
  // devicebits are the device capabilities channels is the number of channels
  //return the new device number or -1 if not possible
  int8_t registerDev(String deviceId, AT_MessageBuffer msg);
  //use information saved with readNewMessage
  int8_t registerDev();
  //get the pointer on a result data structure
  //index = device index x channels per device + channel number
  ATCURVALUES getResult(uint16_t index);
  //set the step value for a result
  //index = device index x channels per device + channel number
  void setStep(uint8_t step, uint16_t index);
  //check if a device has data to be returned parameters are
  //the number of the device, a pointer to the buffer and a pointer
  //to the maximum size it returns the number of packets found or -1 if buffer overflow
  //after return the size parameter contains the used buffer size
  int8_t getResponse(int16_t device, uint8_t * buffer, uint8_t * size);
  //get a free widget slot for a defined size return -1 if no free slot
  int16_t getFreeSlot(uint8_t page, uint8_t size);
  //fin a free widget slot on all pages for a defined size return -1 if no free slot
  int16_t getFreeSlot(uint8_t size);
  //getPage returns the specified display page
  ATDISPLAYPAGE getPage(uint8_t page);
  //getWidget returns the widget in slot on page
  ATDISPLAYWIDGET getWidget(uint8_t page, uint8_t slot);
  //getWidget returns the address of a widget in slot on page to allow editing
  ATDISPLAYWIDGET * getWidgetAdr(uint8_t page, uint8_t slot);
  //return a value as formatted string with or without units
  String getValueString(uint16_t index, uint8_t precision, boolean useunit );
  //return value for switches
  uint8_t getBooleanValue(uint16_t index);
  //check if a result is a output to device
  boolean isValueOutput(uint16_t index);
  //check if a result is a output switch
  boolean isSwitchOut(uint16_t index);
  //check if a value is 0
  boolean isValueZero(uint16_t index);
  //check if a result is valid
  boolean isValid(uint16_t index);
  //delete all device
  boolean clearDevices();
  //delete one device
  boolean deleteDevice(uint8_t device);
  //toggle switch result
  void toggleResult(uint16_t index);
  //get the device id
  String getDeviceId(uint8_t device);
  //return the device name or id or - if inaktiv
  String getDeviceName(uint8_t device);
  //return pointer to a device
  ATDEVICE * getDevice(uint8_t device);
  //return true if new device waits for registration
  boolean hasNewDevice();
  //read message data into _newMsg
  void readNewMessage(String newDevice, const uint8_t * buffer);
  //return the index = pag* ATWIDGETSPERPAGE + slot for a widget of given source
  //return -1 if no widget exists
  int16_t findWidgetBySource(uint16_t source);
  //delete all widgets for a device
  void deleteWidgetsForDevice(uint8_t device);
  //set the source for a widget
  void setWidgetSource(uint16_t widget, uint16_t source);



private:
  ATCURVALUES _results[ATMAXCHANNELS]; //array to hold results
  ATDEVICE _devices[ATMAXDEVICE];      //array to hold devices
  ATDISPLAYPAGE _pages[ATMAXPAGES];    //array to hold widget configurations
  String _deviceFile;                   //file to save devices
  String _confFile;                   //file to save devices
  String _newDevice = "";                  //id of a newly detected device
  AT_MessageBuffer _newMsg;            //messageblock received from this device
};

#endif
