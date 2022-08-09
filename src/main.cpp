#include <Arduino.h>
#include <Tasks.h>

void setup()
{
    Serial.begin(115200);

    BLE_app_main();
    DHT11_app_main();
    light_sensor_app_main();
    WiFi_info_report_app_main();
    Mqtt_app_main();

    vTaskDelete(NULL);
}

void loop()
{
    delay(1000);
}