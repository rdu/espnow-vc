#define PJON_PACKET_MAX_LENGTH 250
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include "user_interface.h"
#include <ArduinoJson.h>
#include <PJON.h>

#define bus_pin 4                                     // pin for the bus
#define hb_interval 2000                              // heartbeat interval
uint8_t mac[] = {0x36, 0x11, 0x22, 0x33, 0x44, 0x55}; // AP mac address

#define target_addr 0x31          // target address
#define self_addr 0x30            //own address
uint8_t bus_id[4] = {0, 0, 1, 5}; // pjon bus id

PJON<SoftwareBitBang> bus(bus_id, self_addr); // bus mode

void initVariant()
{
  WiFi.mode(WIFI_AP);                   // set wifi mode to AP
  wifi_set_macaddr(SOFTAP_IF, &mac[0]); // set custom mac
}

void initEspNow()
{
  if (esp_now_init() != 0) // init ESP_Now
  {
    Serial.println("*** ESP_Now init failed");
    ESP.restart();
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO); // slave and conroller
  esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {
    Serial.println("received data from esp-now");
    bus.send(target_addr, (char *)data, len); // send data to bus
  });

  Serial.println("Init ESP_Now done");
}

int startup;

void setup()
{
  startup = millis();
  Serial.begin(74880);
  Serial.println("starting gateway host...");
  bus.strategy.set_pin(bus_pin);
  bus.begin(); // start bus
  Serial.print("This node AP mac: ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.print("This node STA mac: ");
  Serial.println(WiFi.macAddress());

  initEspNow();
}

int heartBeat;

void send_heartbeat()
{
  Serial.println("heartbeat...");

  String json;
  String payload;
  char buff[200];

  StaticJsonBuffer<200> jsonBuffer;    // max message length 200
  StaticJsonBuffer<200> payloadBuffer; //...

  JsonObject &payload_root = payloadBuffer.createObject(); // create json object for payload
  JsonObject &root = jsonBuffer.createObject();            // create json object for root
  payload_root["uptime"] = String(millis() - startup);     // stamp uptime
  payload_root.printTo(payload);                           // create payload string
  root["topic"] = "epsnowgateway/heartbeat";               // set topic heartbeat
  root["payload"] = payload;                               // set payload
  root.printTo(json);                                      // create result string
  int length = json.length() + 1;                          // ??? why is the lenght to small?
  json.toCharArray(buff, length);                          // create char array

  Serial.print("len: #");
  Serial.println(String(json.length()));
  Serial.print("message #1: ");
  Serial.println(buff);
  Serial.print("message #2: ");
  Serial.println(json);
  bus.send(target_addr, buff, length); // send message to bus
}

void loop()
{
  if (!bus.update())
  {
    if (millis() - heartBeat > hb_interval) // send a heartbeat message
    {
      heartBeat = millis();
      send_heartbeat();
    }
  }
};