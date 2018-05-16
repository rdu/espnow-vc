#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <Wire.h>

#include <espnow.h>
#include "user_interface.h"

#define i2c_slave_address 88

/*
 *  http://serverfault.com/questions/40712/what-range-of-mac-addresses-can-i-safely-use-for-my-virtual-machines
 */
uint8_t mac[] = {0x36, 0x11, 0x22, 0x33, 0x44, 0x55};


void initVariant()
{
    WiFi.mode(WIFI_AP);
    wifi_set_macaddr(SOFTAP_IF, &mac[0]);
}

void initEspNow()
{
    if (esp_now_init() != 0)
    {
        Serial.println("*** ESP_Now init failed");
        ESP.restart();
    }

    esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
    esp_now_register_recv_cb([](uint8_t *mac, uint8_t *data, uint8_t len) {
        Serial.println("received data from esp-now");
        Wire.beginTransmission(i2c_slave_address);
        Wire.write(mac, 6);
        Wire.write(len);
        Wire.write(data, len);
    });
}

void setup()
{
    Serial.begin(74880);
    Serial.println("starting gateway host...");
    Wire.begin();

    Serial.print("This node AP mac: ");
    Serial.println(WiFi.softAPmacAddress());
    Serial.print("This node STA mac: ");
    Serial.println(WiFi.macAddress());

    initEspNow();
}

int heartBeat;

void loop()
{
    if (millis() - heartBeat > 30000)
    {
        String msg = "heartbeat";
        Serial.println("Waiting for ESP-NOW messages...");
        heartBeat = millis();

        Wire.beginTransmission(i2c_slave_address);
        byte tmp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        Wire.write(tmp, 6);
        byte plain[msg.length()];
        Wire.write(msg.length());
        msg.getBytes(plain, msg.length());
        Wire.write(plain, msg.length());
        Serial.println("send heartbeat to slave");
    }
}