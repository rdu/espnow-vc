#define PJON_PACKET_MAX_LENGTH 250
#include <PJON.h>
#include <ArduinoJson.h>

#define bus_pin 4 // pin for the bus

#define self_addr 0x31            //own address
uint8_t bus_id[4] = {0, 0, 1, 5}; // pjon bus id

PJON<SoftwareBitBang> bus(self_addr); // bus mode

void receiver_function(uint8_t *payload, uint16_t length, const PJON_Packet_Info &packet_info)
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(payload);

  const char *topic = root["topic"];
  const char *_payload = root["payload"];

  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("payload: ");
  Serial.println(_payload);

  if (!root.success())
  {
    Serial.println("ArduinoJson parseObject() failed");
  }
  digitalWrite(2, HIGH);
  delay(100);
  digitalWrite(2, LOW);
}

void setup()
{
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW); // Initialize LED 13 to be off

  bus.strategy.set_pin(bus_pin);
  bus.begin();
  bus.set_receiver(receiver_function);

  Serial.begin(74880);
};

void loop()
{
  bus.receive();
};