#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DHT.h>
#include <Arduino_FreeRTOS.h>
#include "semphr.h"

#define RAIN_SENSOR A0
#define LIGHT_SENSOR A1
#define DHT_PIN 9
#define DHT_TYPE DHT11

RF24 radio(7, 8); // CE, CSN
DHT dht(DHT_PIN, DHT_TYPE);
const byte address[6] = "00002";

SemaphoreHandle_t xBinarySemaphore;

void Task1_Rain_Sensor_Read(void *param);
void Task2_TempHum_Sensor_Read(void *param);
void Task3_Light_Sensor_Read(void *param);
void Task4_Transmission(void *param);

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;
TaskHandle_t Task_Handle3;
TaskHandle_t Task_Handle4;

struct Values {
  float TEMPERATURE;
  int HUMIDITY;
  int RAIN_SENSOR; 
  int LIGHT_SENSOR;
};

typedef Values Package;
Package sensorValues;

void setup() {

Serial.begin(9600);

dht.begin();

pinMode(RAIN_SENSOR, INPUT);
pinMode(LIGHT_SENSOR, INPUT);

radio.begin();
radio.openWritingPipe(address);
radio.setPALevel(RF24_PA_MIN);
radio.stopListening();
  
xBinarySemaphore = xSemaphoreCreateBinary();
xTaskCreate(Task1_Rain_Sensor_Read , "Task1" , 100 , NULL , 1 , &Task_Handle1);
xTaskCreate(Task2_TempHum_Sensor_Read , "Task2" , 512 , NULL , 1 , &Task_Handle2);
xTaskCreate(Task3_Light_Sensor_Read , "Task3" , 100 , NULL , 1 , &Task_Handle3);
xTaskCreate(Task4_Transmission , "Task3" , 100 , NULL , 1 , &Task_Handle3);
xSemaphoreGive(xBinarySemaphore);

}

void loop() {}

void Task1_Rain_Sensor_Read(void *param) {
  
  (void)param;

  while(1) {
  xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
  sensorValues.RAIN_SENSOR = analogRead(A0);
  Serial.print(sensorValues.RAIN_SENSOR);
  Serial.println();
  xSemaphoreGive(xBinarySemaphore);
  vTaskDelay(100/portTICK_PERIOD_MS);
  }
  
}

void Task2_TempHum_Sensor_Read(void *param) {
  
  (void)param;

  while(1) {
  xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
  sensorValues.TEMPERATURE = dht.readTemperature();
  sensorValues.HUMIDITY = dht.readHumidity();
  Serial.print(sensorValues.TEMPERATURE);
  Serial.println();
  Serial.print(sensorValues.HUMIDITY);
  Serial.println();
  xSemaphoreGive(xBinarySemaphore);
  vTaskDelay(100/portTICK_PERIOD_MS);
  }
  
}

void Task3_Light_Sensor_Read(void *param) {
  
  (void)param;

  while(1) {
  xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
  sensorValues.LIGHT_SENSOR = analogRead(A1);
  Serial.print(sensorValues.LIGHT_SENSOR);
  Serial.println();
  xSemaphoreGive(xBinarySemaphore);
  vTaskDelay(100/portTICK_PERIOD_MS);
  }
  
}

void Task4_Transmission(void *param) {
  
  (void)param;

  while(1) {
  xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
  radio.write(&sensorValues, sizeof(sensorValues));
  xSemaphoreGive(xBinarySemaphore);
  vTaskDelay(100/portTICK_PERIOD_MS);
  }
  
}
