/*
 WLAN temperature sensor
 ESP Now to ArduiTouch SmartHome
 If the device does not have a valid server MAC address
 a search will bve made to find a WLAN with SSID ATSmartHome
 The server MAC address will be saved as long as thge power supply 
 will not be interrupted.
 The ESP Now Protokoll is very fast so the higher current for network wiil be
 consumed for a short time (us) only. After sending the values, the device
 switches for five minutes into a deep sleep mode with very low power consumption.
*/

//library for WiFi
#include <ESP8266WiFi.h>
//libraries to use DS18B20
#include <OneWire.h>
#include <DallasTemperature.h>
//library for the used message protocol
#include "AT_MessageBuffer.h"


//library for ESP Now
extern "C" {
  #include <espnow.h>
}

//SSID to search for
#define GW_SSID "ATSmartHome"

//flag to switch debug messages on
#define DEBUG true

#define SEND_TIMEOUT 2000  // 2 Sekunden timeout 

//define channels for the two sensors
#define CHANNEL_TEMP1 0
#define CHANNEL_TEMP2 1

//Pins to connect the sensors
const byte bus1 = 2; //GPIO2
const byte bus2 = 4; //GPIO4

//Data structure to save server MAC address
//and a checksum inRTC memory
struct MEMORYDATA {
  uint32_t crc32; //checksum for validation
  uint8_t mac[6];
};


//Global variables
volatile bool callbackCalled;


boolean has1 = 0;
boolean has2 = 0;

//MAC address and WLAN channel
MEMORYDATA statinfo;

//Bus to the sensors
OneWire oneWire1(bus1);
OneWire oneWire2(bus2);

DallasTemperature sensor1(&oneWire1);
DallasTemperature sensor2(&oneWire2);

//Array to save sensor addresses
DeviceAddress address1;
DeviceAddress address2;

AT_MessageBuffer msg;

//function to calculate the checksum
uint32_t calculateCRC32(const uint8_t *data, size_t length)
{
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

//write server MAC and checksum to RTC memory
void UpdateRtcMemory() {
    uint32_t crcOfData = calculateCRC32(((uint8_t*) &statinfo) + 4, sizeof(statinfo) - 4);
    statinfo.crc32 = crcOfData;
    ESP.rtcUserMemoryWrite(0,(uint32_t*) &statinfo, sizeof(statinfo));
}

//search for the access point
void ScanForSlave() {
  bool slaveFound = 0;
  
  int8_t scanResults = WiFi.scanNetworks();
  // reset on each scan

  if (DEBUG) Serial.println("Scan done");
  if (scanResults == 0) {
    if (DEBUG) Serial.println("No WiFi devices in AP Mode found");
  } else {
    if (DEBUG) Serial.print("Found "); 
    if (DEBUG) Serial.print(scanResults); 
    if (DEBUG) Serial.println(" devices ");
    for (int i = 0; i < scanResults; ++i) {
      // Print SSID and RSSI for each device found
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      int32_t chl = WiFi.channel(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      if (DEBUG) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(SSID);
        Serial.print(" /");
        Serial.print(chl);
        Serial.print(" (");
        Serial.print(RSSI);
        Serial.print(")");
        Serial.println("");
      }
      delay(10);
      // Check if the current device starts with ATSmartHome`
      if (SSID == GW_SSID) {
        // SSID of interest
        if (DEBUG) {
          Serial.println("Found a Slave.");
          Serial.print(i + 1); Serial.print(": "); Serial.print(SSID); Serial.print(" ["); Serial.print(BSSIDstr); Serial.print("]"); Serial.print(" ("); Serial.print(RSSI); Serial.print(")"); Serial.println("");
        }
        int mac[6];
        // get the server MAC and save to RTC memory
        if ( 6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c",  &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5] ) ) {
          for (int ii = 0; ii < 6; ++ii ) {
            statinfo.mac[ii] = (uint8_t) mac[ii];
          }
          UpdateRtcMemory();
        }

        slaveFound = 1;
        //no more search after AP was found
        break;
      }
    }
  }
  
  
  if (DEBUG) {
    if (slaveFound) {
      Serial.println("Slave Found, processing..");
    } else {
      Serial.println("Slave Not Found, trying again.");
    }
  }

  // release RAM
  WiFi.scanDelete();
}
// function to print sensor address
void printAddress(DeviceAddress address)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (address[i] < 16) Serial.print("0");
    Serial.print(address[i], HEX);
  }
}

//function to initilize DS18B20 sensor
boolean initDS18B20(uint8_t pin, DallasTemperature sensor, DeviceAddress address ) {
  boolean found = 0;
  pinMode(pin,INPUT_PULLUP);
  sensor.begin();
  if (DEBUG) {
    Serial.print(sensor.getDeviceCount(), DEC);
    Serial.println(" Sensoren gefunden.");
  }
  //check if temperature sensor
  if (!sensor.getAddress(address,0)) {
    if (DEBUG) Serial.println("No temperature sensor exists!");
  } else { 
    //show address
    if (DEBUG) {
      Serial.print("Adresse: ");
      printAddress(address);
      Serial.println();
    }
    //set resolution to (9, 10, 11 or 12 bit)
    sensor.setResolution(address,10);
    //start measurement
    sensor.requestTemperatures();
    found = 1;
  }
  return found;
}

void setup() {

  if (DEBUG) {
    Serial.begin(115200); 
    Serial.println("Start");
  }
  //get local MAC address to use it as device id
  String strmac = WiFi.macAddress();
  if (DEBUG) {
    Serial.print("My MAC address = ");
    Serial.println(strmac);
  }
  msg.setId(strmac);
  msg.clear();
  if (DEBUG) {
    Serial.println("Sensor 1");
  }
  has1 = initDS18B20(bus1,sensor1,address1);
  if (DEBUG) {
    Serial.println("Sensor 2");
  }
  has2 = initDS18B20(bus2,sensor2,address2);
  //read server MAC from RTC memory
  ESP.rtcUserMemoryRead(0, (uint32_t*) &statinfo, sizeof(statinfo));
  if (DEBUG) Serial.println("RTC Done");
  uint32_t crcOfData = calculateCRC32(((uint8_t*) &statinfo) + 4, sizeof(statinfo) - 4);
  WiFi.mode(WIFI_STA); // Station mode for esp-now sensor node
  if (DEBUG) Serial.println("WifiMode");

  if (statinfo.crc32 != crcOfData) { //if checksum different we do not have a valid server MAC
    if (DEBUG) Serial.println("Scan for slave");
    ScanForSlave();
    //for (uint8_t i = 0; i<6;i++) statinfo.mac[i] = gwmac[i];
    if (DEBUG) {
      Serial.printf("This mac: %s, ", WiFi.macAddress().c_str()); 
      Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", statinfo.mac[0], statinfo.mac[1], statinfo.mac[2], statinfo.mac[3], statinfo.mac[4], statinfo.mac[5]); 
    }
  }
  if (esp_now_init() != 0) {
    if (DEBUG) Serial.println("*** ESP_Now init failed");
    ESP.restart();
  }
  //ESP Now Controller
  WiFi.setAutoConnect(false);
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  uint8_t ch = esp_now_get_peer_channel(statinfo.mac);
  if (DEBUG) Serial.printf("Channel = %i\r\n",ch);
  //initialize Peer data
  int res = esp_now_add_peer(statinfo.mac, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);
  if (res==0) Serial.println("Erfolgreich gepaart");
  //register callback
  esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    if (DEBUG) {
      Serial.print("send_cb, status = "); Serial.print(sendStatus); 
      Serial.print(", to mac: "); 
      char macString[50] = {0};
      sprintf(macString,"%02X:%02X:%02X:%02X:%02X:%02X", statinfo.mac[0], statinfo.mac[1], statinfo.mac[2], statinfo.mac[3], statinfo.mac[4], statinfo.mac[5]);
      Serial.println(macString);
    }
    callbackCalled = true;
  });
  
  //set flag to false
  callbackCalled = false;
  
  //start measurement
  if (has1) sensor1.requestTemperatures();
  if (has2) sensor2.requestTemperatures();
  delay(750); //750 ms warten bis die Messung fertig ist
  //read values and save for sending
  if (has1) msg.addCelsius(sensor1.getTempC(address1), CHANNEL_TEMP1);
  if (has2) msg.addCelsius(sensor2.getTempC(address2), CHANNEL_TEMP2);
  //copy datastructure into sendbuffer
  uint8_t buf[255];
  uint8_t sz;
  sz = 255;
  if (msg.fillBuffer(&buf[0], &sz)) esp_now_send(NULL, buf, sz); // NULL means send to all peers
}

void loop() {
  //wait for data to be sent
  if (callbackCalled || (millis() > SEND_TIMEOUT)) {
    if (DEBUG) Serial.println("Sleep");
    delay(100);
    //go for 300 seconds into deep sleep mode
    //wakeup by reset
    //reset does not delete data in  RTCmemory
    ESP.deepSleep(10E6);
  }
}
