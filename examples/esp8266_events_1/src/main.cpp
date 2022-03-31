#include <Arduino.h>
#include "main.h"

#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>
#include <WiFiUdp.h>

#include <esp8266_events.h>
#include <alwificonnect.h>

WifiConnect * _DeviceWifi;


WiFiUDP ntpUDP;
// By default 'pool.ntp.org' is used with 60 seconds update interval and
// no offset
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
EventManager * _EventManager;

TestEventItem * TestEventItemArray[3];

void setup() {
  Serial.begin(115200);

  for(unsigned long const serialBeginTime = millis(); !Serial && (millis() - serialBeginTime > 5000); ) { }
  delay(300);

  Serial.println(F("\n##################################################\n"));  

  Serial.setDebugOutput(false);

  LittleFS.begin();

  _DeviceWifi   = new WifiConnect();
  _EventManager = new EventManager(&timeClient, 6);

  TestEventItemArray[0] = new TestEventItem(0,1,"evt_1") ;  
  TestEventItemArray[1] = new TestEventItem(2,3,"evt_2") ;  
  TestEventItemArray[2] = new TestEventItem(4,5,"evt_3") ;    
}

void loop() {
    _DeviceWifi->handleConnection();

    if (_DeviceWifi->WIFIsetupIsReady()) ArduinoOTA.handle();

    if (_DeviceWifi->needSTA()) {
      if (_DeviceWifi->STAisReady()) {

        MDNS.update();

        timeClient.update();

        _EventManager->loop();


      }
    } else {
      // if (_DeviceWifi->WIFIsetupIsReady()) _ads_httpserver.loop();
    }


}
uint8_t _sMin = 0;
uint8_t _sHr = 18;
TestEventItem::TestEventItem(uint8_t eP1, uint8_t eP2, const char * name){
  _event = new uint8_t[2];
  _event[0] = eP1;
  _event[1] = eP2;
  _name = name;
  _EventManager->set_triggerFunc(eP1, std::bind(&TestEventItem::event_1, this));
  _EventManager->set_triggerFunc(eP2, std::bind(&TestEventItem::event_2, this));
  _EventManager->set_time(eP1, _sHr, _sMin);
  _EventManager->set_activate(eP1, true);
  _sMin++;
  _EventManager->set_time(eP2, _sHr, _sMin);
  _EventManager->set_activate(eP2, true);
  _sMin++;

  Serial.printf_P(PSTR("[name: %s][eP1: %d][eP2: %d]\n"), name, eP1, eP2);
}
void TestEventItem::event_1(){
  Serial.printf_P(PSTR("event_1 = [name: %s][eP1: %d][eP2: %d]\n"), _name, _event[0], _event[1]);

}
void TestEventItem::event_2(){
  Serial.printf_P(PSTR("event_2 = [name: %s][eP1: %d][eP2: %d]\n"), _name, _event[0], _event[1]);
}