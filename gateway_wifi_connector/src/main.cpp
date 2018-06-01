#define PJON_PACKET_MAX_LENGTH 250
#include <PJON.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"

#define bus_pin 4 // pin for the bus

#define self_addr 0x31            //own address
uint8_t bus_id[4] = {0, 0, 1, 5}; // pjon bus id

PJON<SoftwareBitBang> bus(bus_id, self_addr); // bus mode

WiFiClient espClient;
PubSubClient client(espClient);

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
{
  StaticJsonBuffer<200> jsonBuffer;                   // create json buffer for 200 chars
  JsonObject &root = jsonBuffer.parseObject(payload); // parse received string

  const char *topic = root["topic"];    // get the message topic
  const char *_payload = root["payload"]; // get the payload

  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("payload: ");
  Serial.println(_payload);

  if (!root.success())
  {
    Serial.println("ArduinoJson parseObject() failed");
  }
  else if (topic)
  {
    Serial.println("published to mqtt server");
    client.publish(topic, _payload);  // publish message to mqtt
  }

  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
}

void setup_wifi()
{
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(esp_ssid);

  WiFi.begin(esp_ssid, esp_password); // start wifi

  while (WiFi.status() != WL_CONNECTED) // try connecting until connected
  {
    delay(500); // wait half a second
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  delay(100);
  Serial.begin(74880); // start serial for debugging
  delay(100);
  Serial.println("init serial done");

  pinMode(2, OUTPUT);   // set led pin to output
  digitalWrite(2, LOW); // drive it low

  bus.strategy.set_pin(bus_pin);       // set pin for bus communication
  bus.begin();                         // start bus communication
  bus.set_receiver(receiver_function); // define receiver function

  setup_wifi();                                     // setup wifi
  client.setServer(esp_mqtt_server, esp_mqtt_port); // set mqtt server
};

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("attempting mqtt connection...");
    String clientId = "espnow-gateway";
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected to mqtt server");
      client.publish("espnow-gateway", "connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // try again in 5 seconds
    }
  }
}

void loop()
{
  bus.receive();           // receive bus messages
  if (!client.connected()) // check if mqtt client is connected
  {
    reconnect(); // if not do so
  }
  client.loop(); // handle mqtt client loop stuff
};