/*
 WLAN pressure sensor BMP280 and or BME280
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
//libraries to use BMP280 and BME280
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
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
#define CHANNEL_TEMP_BME 0
#define CHANNEL_PRESS_BME 1
#define CHANNEL_ALT_BME 2
#define CHANNEL_HUM_BME 3
#define CHANNEL_TEMP_BMP 4
#define CHANNEL_PRESS_BMP 5
#define CHANNEL_ALT_BMP 6
 
#define SEALEVELPRESSURE_HPA (1013.25)

//Data structure to save server MAC address
//and a checksum inRTC memory
struct MEMORYDATA {
  uint32_t crc32; //checksum for validation
  uint8_t mac[6];
};


//Global variables
volatile bool callbackCalled;


boolean hasBme = 0;
boolean hasBmp = 0;

//MAC address and WLAN channel
MEMORYDATA statinfo;

AT_MessageBuffer msg;

Adafruit_BME280 bme; // I2C
Adafruit_BMP280 bmp; // I2C

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

//function to initilize BME Sensor
//try both valid addresses 77 or 76
boolean initBme() {
  boolean status = bme.begin(0x77);  
  if (!status) status = bme.begin(0x76);
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      hasBme = false;
  }
  return status;
}

//function to initilize BMP Sensor
//try both valid addresses 77 or 76
boolean initBmp() {
  boolean status = bmp.begin(0x77);  
  if (!status) status = bmp.begin(0x76);
  if (!status) {
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
      hasBmp = false;
  }
  if (status) {
  /* Default settings from datasheet. */
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }
  return status;
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
  hasBme = initBme();
  if (hasBme && DEBUG) {
    Serial.println("Found Sensor BME");
  }
  hasBmp = initBmp();
  if (hasBmp && DEBUG) {
    Serial.println("Found Sensor BMP");
  }
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
  //read values and save for sending
  if (hasBme) {
    if (DEBUG){
      Serial.print("Temperature = ");
      Serial.print(bme.readTemperature());
      Serial.println(" *C");
  
      Serial.print("Pressure = ");
  
      Serial.print(bme.readPressure() / 100.0F);
      Serial.println(" hPa");
  
      Serial.print("Approx. Altitude = ");
      Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
      Serial.println(" m");
  
      Serial.print("Humidity = ");
      Serial.print(bme.readHumidity());
      Serial.println(" %");
     
    }
    msg.addCelsius(bme.readTemperature(), CHANNEL_TEMP_BME);
    msg.addHektoPascal(bme.readPressure()/100.0F, CHANNEL_PRESS_BME);
    msg.addMeter(bme.readAltitude(SEALEVELPRESSURE_HPA), CHANNEL_ALT_BME);
    msg.addPercent(bme.readHumidity(), CHANNEL_HUM_BME);
  }
  if (hasBmp) {
    if (DEBUG) {
      Serial.print(F("Temperature = "));
      Serial.print(bmp.readTemperature());
      Serial.println(" *C");
  
      Serial.print(F("Pressure = "));
      Serial.print(bmp.readPressure());
      Serial.println(" Pa");
  
      Serial.print(F("Approx altitude = "));
      Serial.print(bmp.readAltitude(SEALEVELPRESSURE_HPA)); /* Adjusted to local forecast! */
      Serial.println(" m");
      
    }
    msg.addCelsius(bmp.readTemperature(), CHANNEL_TEMP_BMP);
    msg.addHektoPascal(bmp.readPressure()/100.0F, CHANNEL_PRESS_BMP);
    msg.addMeter(bmp.readAltitude(SEALEVELPRESSURE_HPA), CHANNEL_ALT_BMP);
  }
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
    //go for 10 seconds into deep sleep mode
    //wakeup by reset
    //reset does not delete data in  RTCmemory
    ESP.deepSleep(10E6);
  }
}
