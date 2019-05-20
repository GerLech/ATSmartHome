/* this subfile to ATSmartHome.ino contains allfunctions
 *  and global variables associated to WiFi usage
 */
#include <WiFi.h>
#include <time.h>
#include <esp_now.h>
#include <ESPmDNS.h>

//Access point password and channel
#define APPWD "123456789"
#define APCHANNEL 0


//NTP server and offsets for MET

#define GMT_OFFSET_SEC 3600
#define DAYLIGHT_OFFSET_SEC 3600

#define HOSTNAME "smart.home"

//Global variables
esp_now_peer_info_t info;

//init WiFi and start the access point
void initWiFi() {
  //ESP32 as access point and station to use Internet connection in parallel
  //Attention !! Internet connection works only if your WLAN router uses channel 0
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("ATSmartHome",APPWD,APCHANNEL,0);
  return;
}

void initMDNS() {
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("MDNS init failed");
  } else {
    Serial.println("MDNS successful started");
    MDNS.addService("http", "tcp", 80);
  }
  
}


void disconnectWlan() {
  if (WiFi.status() == WL_CONNECTED) WiFi.disconnect();
}

//establish a Wlan connection 
boolean connectWlan(String ssid, String password, String NTPserver = "") {
  disconnectWlan();
  //pointer to the characters inside the string
  char* txtSSID = const_cast<char*>(ssid.c_str());
  char* txtPassword = const_cast<char*>(password.c_str());  
  char* txtNTP = const_cast<char*>(NTPserver.c_str());
  //connect to wlan
  WiFi.begin(txtSSID, txtPassword);
  uint8_t timeout = 0;
  while ((WiFi.status() != WL_CONNECTED) && (timeout<10)) {
    timeout++;
    delay(1000);
  }
  //we wait max 10 s until connected
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.setHostname(HOSTNAME);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println(txtNTP);
    if (NTPserver != "") configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, txtNTP);
    //Aktuelle Uhrzeit ausgeben
    Serial.println(AT_GetLocalTime());
    initMDNS();
 }
 return (WiFi.status() == WL_CONNECTED);
}


void printESP_Now_Err (String prefix, esp_err_t err) {
  switch (err) {
    case ESP_ERR_ESPNOW_BASE: Serial.println(prefix+" ESPNOW error number base"); break;
    case ESP_ERR_ESPNOW_NOT_INIT: Serial.println(prefix+"ESPNOW is not initialized"); break;
    case ESP_ERR_ESPNOW_ARG: Serial.println(prefix+"Invalid argument"); break;
    case ESP_ERR_ESPNOW_NO_MEM: Serial.println(prefix+"Out of memory"); break;
    case ESP_ERR_ESPNOW_FULL: Serial.println(prefix+" ESPNOW peer list is full"); break;
    case ESP_ERR_ESPNOW_NOT_FOUND: Serial.println(prefix+"ESPNOW peer is not found"); break;
    case ESP_ERR_ESPNOW_INTERNAL: Serial.println(prefix+"Internal error"); break;
    case ESP_ERR_ESPNOW_EXIST: Serial.println(prefix+"ESPNOW peer has existed"); break;
    case ESP_ERR_ESPNOW_IF: Serial.println(prefix+"Interface error"); break;
    default : Serial.println(prefix+"Unknown error"); break;
  }
}


//add a new peer to the peer list
esp_err_t addPeer(const uint8_t mac[6]) {
  esp_err_t res = ESP_OK;
  if (!esp_now_is_peer_exist(mac)) {
    for (int ii=0;ii<6;ii++) info.peer_addr[ii] = mac[ii];
    info.channel = 0;
    info.encrypt = 0;
    const esp_now_peer_info_t *peer = &info;
    res = esp_now_add_peer(peer);
    if (res != ESP_OK) printESP_Now_Err("Add Peer ",res);
  }
  return res;
} 


// callback for ESP Now
void readESPNow(const uint8_t *mac_addr, const uint8_t *r_data, int data_len) {
  int16_t devnr;
  uint8_t pkts;
  String strid;
  uint8_t sz;
  uint8_t buf[256];
  int8_t packets;
  esp_err_t err;
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
    //unknown device we save it as new device
    database.readNewMessage(strid,r_data);
    dsp.setStatus(strid);
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
    if (packets>0) {
      err = addPeer(mac_addr);
      if (err == ESP_OK) {
        err = esp_now_send(mac_addr, buf, sz);
        if (err != ESP_OK) printESP_Now_Err("Send Data ",err);
      }
    }
  }
  msg.clear();
}

//send data to an esp-device
//todo add usage for other outputs too currentrly only switch
void sendData(uint16_t index) {
  uint8_t dev = index/ATMAXDEVCHAN;
  String id;
  esp_err_t err;
  id = database.getDeviceId(dev);
  if (id != "") {
    Serial.println("Send Data");
    AT_MessageBuffer sendbuf;
    ATCURVALUES val;
    uint8_t buf[100];
    uint8_t sz = 100;
    uint8_t ch = index - (dev * ATMAXDEVCHAN);
    sendbuf.clear();
    sendbuf.setId(id);
    val = database.getResult(index);
    sendbuf.addSwitchOut(val.value[0]!=0, ch);
    sendbuf.fillBuffer(buf,&sz);
    err = addPeer(buf);
    if (err == ESP_OK) {
      err = esp_now_send(buf, buf, sz);
      if (err != ESP_OK) printESP_Now_Err("Send Data ",err);
    }
  }
}

//start ESP NOW
boolean initESPNow() {
  esp_now_deinit();
  boolean result = (esp_now_init() == ESP_OK);
  info.channel = APCHANNEL;
  info.encrypt = 0;
  if (result) esp_now_register_recv_cb(readESPNow);
  return result;
}
