#ifndef TASKS_H
#define TASKS_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <BLE_Tool.h>
#include <DHT11_Sensor.h>
#include <DataUploader.h>
#include <Light_Sensor.h>
#include <MqttCallBacks.h>
#include <WiFi_Tool.h>
#include <oled.h>

// ------------------------ DHT11 Sensor Task ------------------------//

TaskHandle_t dht_task_handle;

esp_timer_handle_t DHT11_temp_humi_timer = 0;

void DHT11_timer_periodic(void *arg);

void DHT11_task(void *DHT11_task)
{
    gpio_pad_select_gpio(DHT11_PIN);

    while (1)
    {
        DHT11_Start();
        if (DHT11_ReadTemHum(DHT11Data))
        {
            Temp = DHT11Data[2];
            Humi = DHT11Data[0];

            printf("Temp= %d, Humi= %d\r\n", Temp, Humi);

            std::string msg = "T/H: " + std::to_string(Temp)+ "C/" + std::to_string(Humi) + "%";
            oled_draw_in_two(msg.c_str());

            if (isWiFiConnected())
            {
                UPLOAD_DATA datas[2];
                datas[0].key = "temperature";
                datas[0].value = std::to_string(Temp);

                datas[1].key = "humidity";
                datas[1].value = std::to_string(Humi);

                uploadData(datas, 2);
            }
        }
        else
        {
            printf("DHT11 Error!\r\n");
        }

        vTaskSuspend(NULL);
    }
}

void DHT11_app_main(void)
{
    esp_timer_create_args_t start_dht = {.callback = &DHT11_timer_periodic, .arg = NULL, .name = "DHT11PeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_dht, &DHT11_temp_humi_timer);

    esp_timer_start_periodic(DHT11_temp_humi_timer, 60 * 1000 * 1000);

    xTaskCreatePinnedToCore(DHT11_task, "DHT11_task", 4000, NULL, 2, &dht_task_handle, 1);
}

void DHT11_timer_periodic(void *arg)
{
    vTaskResume(dht_task_handle);
}

// ------------------------ Light Sensor Task ------------------------//
TaskHandle_t light_sensor_task_handle;

esp_timer_handle_t light_sensor_timer = 0;

void light_sensor_timer_periodic(void *arg);

void light_sensor_task(void *light_sensor_task)
{
    light_sensor_start();

    long timer = 0;

    while (1)
    {
        int light_sensor_data = readLightSensorData();

        Serial.printf("Light: %d\r\n", light_sensor_data);

        std::string detail = (light_sensor_data ? "dark" : "bright");
        std::string msg = "Light: " + detail;
        oled_draw_in_two(msg.c_str());

        if ((timer >= 10) && (timer % 10 == 0) && isWiFiConnected())
        {
            UPLOAD_DATA datas[1];
            datas[0].key = "light";
            datas[0].value = std::to_string(light_sensor_data);
            uploadData(datas, 1);
        }

        timer++;
        vTaskSuspend(NULL);
    }
}

void light_sensor_app_main(void)
{
    esp_timer_create_args_t start_light_sensor = {
        .callback = &light_sensor_timer_periodic, .arg = NULL, .name = "LightSensorPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_light_sensor, &light_sensor_timer);

    esp_timer_start_periodic(light_sensor_timer, 3000 * 1000);

    xTaskCreatePinnedToCore(light_sensor_task, "light_sensor_task", 4000, NULL, 2, &light_sensor_task_handle, 1);
}

void light_sensor_timer_periodic(void *arg)
{
    vTaskResume(light_sensor_task_handle);
}

// ------------------------ Network info report Task ------------------------//
TaskHandle_t network_info_report_task_handle;

esp_timer_handle_t network_info_report_timer = 0;

void network_info_report_timer_periodic(void *arg);

void network_info_report_task(void *network_info_report_task)
{

    while (1)
    {
        if (!isWiFiConnected() || !mqtt_client.connected())
        {
            Serial.println("No WiFi or Mqtt connection, reporting network info failed!");
            Serial.println("Please check WiFi or Mqtt connection!");
        }
        else
        {
            network_info_info_report();
        }

        vTaskSuspend(NULL);
    }
}

void WiFi_info_report_app_main(void)
{

    esp_timer_create_args_t start_network_info_report = {
        .callback = &network_info_report_timer_periodic, .arg = NULL, .name = "WiFiInfoReportPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_network_info_report, &network_info_report_timer);

    esp_timer_start_periodic(network_info_report_timer, 60 * 1000 * 1000);

    xTaskCreatePinnedToCore(network_info_report_task, "network_info_report_task", 6000, NULL, 1,
                            &network_info_report_task_handle, 0);
}

void network_info_report_timer_periodic(void *arg)
{
    vTaskResume(network_info_report_task_handle);
}

// ------------------------ Mqtt Task ------------------------//
TaskHandle_t Mqtt_task_handle;

esp_timer_handle_t Mqtt_timer = 0;

void Mqtt_timer_periodic(void *arg);

void Mqtt_task(void *Mqtt_task)
{
    std::string broker = "124.222.158.8";
    std::string username = "mosquitto";
    std::string password = "314159.com";
    int port = 1883;

    while (1)
    {
        // connect to mqtt server
        if (isWiFiConnected() && !isMqttConnected())
        {
            mqtt_init(broker, port, username, password, customizedMqttCallback);
        }
        // start mqtt receive service
        mqtt_client.loop();

        vTaskSuspend(NULL);
    }
}

void Mqtt_app_main(void)
{
    esp_timer_create_args_t start_Mqtt = {.callback = &Mqtt_timer_periodic, .arg = NULL, .name = "MqttPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_Mqtt, &Mqtt_timer);

    esp_timer_start_periodic(Mqtt_timer, 1000 * 1000);

    xTaskCreatePinnedToCore(Mqtt_task, "Mqtt_task", 6000, NULL, 1, &Mqtt_task_handle, 0);
}

void Mqtt_timer_periodic(void *arg)
{
    vTaskResume(Mqtt_task_handle);
}

// ------------------------ BLE Task ------------------------//
TaskHandle_t BLE_task_handle;

esp_timer_handle_t BLE_timer = 0;

void BLE_timer_periodic(void *arg);

void BLE_task(void *BLE_task)
{
    start_BLE();

    while (1)
    {
        // keep BLE alive
        handle_BLE_service();

        vTaskSuspend(NULL);
    }
}

void BLE_app_main(void)
{
    esp_timer_create_args_t start_BLE = {.callback = &BLE_timer_periodic, .arg = NULL, .name = "BLEPeriodicTimer"};

    esp_timer_init();

    esp_timer_create(&start_BLE, &BLE_timer);

    esp_timer_start_periodic(BLE_timer, 1000 * 1000);

    xTaskCreatePinnedToCore(BLE_task, "BLE_task", 4000, NULL, 1, &BLE_task_handle, 1);
}

void BLE_timer_periodic(void *arg)
{
    vTaskResume(BLE_task_handle);
}

#endif