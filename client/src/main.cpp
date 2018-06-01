#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <espnow.h>
#include "user_interface.h"

#define topic "esp_test_new/state"

uint8_t local_mac[] = {0x36, 0x11, 0x22, 0x33, 0x44, 0x56};
uint8_t remoteMac[] = {0x36, 0x11, 0x22, 0x33, 0x44, 0x55};

#define WIFI_CHANNEL 1

int interval;
int startup;

void initVariant()
{
    WiFi.mode(WIFI_STA);                        // set wifi mode to AP
    wifi_set_macaddr(SOFTAP_IF, &local_mac[0]); // set custom mac
}

void setup()
{
    Serial.begin(74880);

    pinMode(2, OUTPUT);   // set led pin to output
    digitalWrite(2, LOW); // drive it low

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    Serial.printf("This mac: %s, ", WiFi.macAddress().c_str());
    Serial.printf("target mac: %02x%02x%02x%02x%02x%02x", remoteMac[0], remoteMac[1], remoteMac[2], remoteMac[3], remoteMac[4], remoteMac[5]);
    Serial.printf(", channel: %i\n", WIFI_CHANNEL);

    if (esp_now_init() != 0)
    {
        Serial.println("*** ESP_Now init failed");
    }

    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_add_peer(remoteMac, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0);

    esp_now_register_send_cb([](uint8_t *mac, uint8_t sendStatus) {
        Serial.printf("send_cb, send done, status = %i\n", sendStatus);
        digitalWrite(2, LOW);
    });

    interval = millis();
    startup = millis();
}

void send_message(char *state)
{
    String json;
    String payload;
    char buff[200];

    StaticJsonBuffer<200> jsonBuffer;    // max message length 200
    StaticJsonBuffer<200> payloadBuffer; //...

    JsonObject &payload_root = payloadBuffer.createObject(); // create json object for payload
    JsonObject &root = jsonBuffer.createObject();            // create json object for root
    payload_root["uptime"] = String(millis() - startup);     // stamp uptime
    payload_root["state"] = state;
    payload_root.printTo(payload);  // create payload string
    root["topic"] = topic;          // set topic heartbeat
    root["payload"] = payload;      // set payload
    root.printTo(json);             // create result string
    int length = json.length() + 1; // ??? why is the lenght to small?
    json.toCharArray(buff, length); // create char array

    Serial.print("len: #");
    Serial.println(String(json.length()));
    Serial.print("message #1: ");
    Serial.println(buff);
    Serial.print("message #2: ");
    Serial.println(json);
    byte plain[length];
    json.getBytes(plain, length);
    esp_now_send(NULL, plain, length);
}

void loop()
{
    if (millis() - interval > 5000)
    {
        digitalWrite(2, HIGH);
        send_message("TEST");
        interval = millis();
    }
}