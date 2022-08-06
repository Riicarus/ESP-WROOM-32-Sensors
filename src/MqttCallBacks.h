#ifndef MQTT_CALLBACKS_H
#define MQTT_CALLBACKS_H

#include <WiFi_Tool.h>

void defaltCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.println();
    Serial.print("Mqtt message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Mqtt message received:");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println();
}

void customizedMqttCallback(char *topic, byte *payload, unsigned int length)
{
    defaltCallback(topic, payload, length);
}

#endif