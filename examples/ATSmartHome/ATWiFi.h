/* this subfile to ATSmartHome.ino contains allfunctions
 *  and global variables associated to WiFi usage
 */
#include <WiFi.h>
#include <esp_now.h>

//Access point password and channel
#define APPWD "123456789"
#define APCHANNEL 0


//NTP server and offsets for MET
#define NTP_SERVER "de.pool.ntp.org"
#define GMT_OFFSET_SEC 3600
#define DAYLIGHT_OFFSET_SEC 0

//Global variables
String newdevice; //new detected device
uint16_t newdevicebits; //capabilities for new device

//init WiFi and start the access point
void initWiFi() {
  //ESP32 as access point and station to use Internet connection in parallel
  //Attention !! Internet connection works only if your WLAN router uses channel 0
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ATSmartHome",APPWD,APCHANNEL,0);
  return;
}

//establish a Wlan connection 
boolean connectWlan(String ssid, String password) {
  //pointer to the characters inside the string
  char* txtSSID = const_cast<char*>(ssid.c_str());
  char* txtPassword = const_cast<char*>(password.c_str());  
  //connect to wlan
  WiFi.begin(txtSSID, txtPassword);
  uint8_t timeout = 0;
  while ((WiFi.status() != WL_CONNECTED) && (timeout<10)) {
    timeout++;
    delay(1000);
  }
  //we wait max 10 s until connected
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    //Aktuelle Uhrzeit ausgeben
    Serial.println(AT_GetLocalTime());
 }
 return (WiFi.status() == WL_CONNECTED);
}

// callback for ESP Now
void readESPNow(const uint8_t *mac_addr, const uint8_t *r_data, int data_len) {
  int16_t devnr;
  uint8_t pkts;
  String strid;
  uint8_t sz;
  uint8_t buf[256];
  int8_t packets;
  ATMESSAGEHEADER msghdr;
  ATDATAPACKET dp;
  //r_data points on to the received data 
  msg.readBuffer(r_data);
  msghdr = msg.getHeader();
  strid = AT_GetId(msghdr.id);
  //look into database if device is known
  devnr = database.findDevice(msghdr.id);
  Serial.print("Got message from ");
  Serial.print(strid);
  Serial.print(" = device ");
  Serial.println(devnr);
  if (devnr < 0) {
    //unknown device we remember the id
    newdevice = strid;
    newdevicebits = msghdr.devicebits;
  } else {
    //know device we update the results
    pkts = msg.getPackets();
    for (uint8_t i = 0; i< pkts; i++) {
      dp = msg.getData(i);
      database.setResult(devnr * ATMAXDEVCHANNELS + dp.channel, dp);
    }
    //we look if there is something to answer
    sz = 250; //maximum payload for esp now 
    packets = database.getResponse(devnr,&buf[0],&sz);
    if (packets>0) esp_now_send(buf, buf, sz); //the first buf pointer is used to reference the MAC address
  }
  msg.clear();
}

//start ESP NOW
boolean initESPNow() {
  boolean result = (esp_now_init() == ESP_OK);
  if (result) esp_now_register_recv_cb(readESPNow);
  return result;
}
