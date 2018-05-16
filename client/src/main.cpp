#include <ESP8266WiFi.h>

extern "C"
{
#include <espnow.h>
}

uint8_t remoteMac[] = {0x36, 0x11, 0x22, 0x33, 0x44, 0x55};

#define WIFI_CHANNEL 4

void setup()
{
    Serial.begin(115200);

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
    });

    String msg = "test";
    byte plain[msg.length()];
    msg.getBytes(plain, msg.length());

    esp_now_send(NULL, plain, msg.length());
}

void loop()
{
}