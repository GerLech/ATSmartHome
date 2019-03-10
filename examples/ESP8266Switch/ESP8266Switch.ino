//library for WiFi
#include <ESP8266WiFi.h>

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

#define INTERVALL 10000 //intervall = 10 seconds

//define channels for the relais
#define CHANNEL_RELAIS 0

//Pins to connect the sensors
#define RELAIS_PIN 2 //GPIO2

//Data structure to save server MAC address
//and a checksum inRTC memory
struct MEMORYDATA {
  uint32_t crc32; //checksum for validation
  uint8_t mac[6];
};

//MAC address and WLAN channel
MEMORYDATA statinfo;
//time when last sent
uint32_t last_sent = 0; //Timestamp for last sent
uint8_t relais_status = 0;
String mymac; //own mac address

//we have two message buffers for send and receive
AT_MessageBuffer sendmsg;
AT_MessageBuffer rcvmsg;

//write server MAC and checksum to RTC memory
void UpdateRtcMemory() {
    uint32_t crcOfData =AT_CalculateCRC32(((uint8_t*) &statinfo) + 4, sizeof(statinfo) - 4);
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

void sendData() {
  uint8_t buf[100]; //buffer for esp
  uint8_t sz = 100;
  sendmsg.clear();
  if (DEBUG) {
    Serial.println("Sende "+mymac);
  }
  sendmsg.setId(mymac);
  sendmsg.addSwitchOut(relais_status != 0,CHANNEL_RELAIS);
  if (sendmsg.fillBuffer(buf,&sz)) esp_now_send(NULL, buf, sz);
  last_sent=millis();
}

void readESPNow(uint8_t *mac_addr, uint8_t *r_data, uint8_t data_len) {
  ATDATAPACKET data;
  uint32_t newstatus;
  rcvmsg.readBuffer(r_data);
  if (mymac.equalsIgnoreCase(rcvmsg.getId())) {
    if (rcvmsg.getPackets() > 0) {
      data = rcvmsg.getData(0);
      if (DEBUG) {
        Serial.printf("Got data for channel %i  value %i \n",data.channel, data.value[0]);
      }  
      if (data.channel == CHANNEL_RELAIS) {
        newstatus = data.value[0];
        if (newstatus != relais_status) {
          relais_status = newstatus;
          digitalWrite(RELAIS_PIN,relais_status);
          if (DEBUG){
            Serial.printf("Relais status = %i\n",relais_status);
          }
        }
      }
    }
  }
}

void setup() {

  if (DEBUG) {
    Serial.begin(115200); 
    Serial.println("Start");
  }
  pinMode(RELAIS_PIN,OUTPUT);
  digitalWrite(RELAIS_PIN,relais_status);
  //get local MAC address to use it as device id
  mymac = WiFi.macAddress();
  if (DEBUG) {
    Serial.print("My MAC address = ");
    Serial.println(mymac);
  }
  ESP.rtcUserMemoryRead(0, (uint32_t*) &statinfo, sizeof(statinfo));
  if (DEBUG) Serial.println("RTC Done");
  uint32_t crcOfData = AT_CalculateCRC32(((uint8_t*) &statinfo) + 4, sizeof(statinfo) - 4);
  WiFi.mode(WIFI_STA); // Station mode for esp-now aktor node
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
  esp_now_set_self_role(3); //(ESP_NOW_ROLE_CONTROLLER);
  uint8_t ch = esp_now_get_peer_channel(statinfo.mac);
  if (DEBUG) Serial.printf("Channel = %i\r\n",ch);
  //initialize Peer data
  int res = esp_now_add_peer(statinfo.mac, ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);
  if (res==0) Serial.println("Successful paired");
  //register callback
  esp_now_register_recv_cb(readESPNow); 
}

void loop() {
  if ((millis() - last_sent) > INTERVALL) {  //intervall to send data change this if required
    sendData();
  }
}
