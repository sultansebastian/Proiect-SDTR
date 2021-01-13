#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Arduino_FreeRTOS.h>
#include "semphr.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>


#define NUM_LEDS 24
#define LED_PIN 5

CRGB leds[NUM_LEDS];

RF24 radio(7, 8); // CE, CSN
LiquidCrystal_I2C lcd(0x27,20,4);
const byte address[6] = "00002";

SemaphoreHandle_t xBinarySemaphore;

void Task1_Reception(void *param);
void Task2_PrintingValues(void *param);

TaskHandle_t Task_Handle1;
TaskHandle_t Task_Handle2;

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
  lcd.init();
  lcd.backlight();

  FastLED.addLeds<WS2812, LED_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);
  FastLED.clear();
  FastLED.show();

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);

  xBinarySemaphore = xSemaphoreCreateBinary();
  xTaskCreate(Task1_Reception , "Task1" , 100 , NULL , 1 , &Task_Handle1);
  xTaskCreate(Task2_PrintingValues , "Task2" , 256 , NULL , 1 , &Task_Handle2);
  xSemaphoreGive(xBinarySemaphore);
}

void loop() {}
void Task1_Reception(void *param) {

  (void)param;

  while(1) {
  xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
  radio.startListening();
  while (!radio.available());
  radio.read(&sensorValues, sizeof(sensorValues));
  xSemaphoreGive(xBinarySemaphore);
  vTaskDelay(100/portTICK_PERIOD_MS);
  }
}

void Task2_PrintingValues(void *param) {

  (void)param;

  while(1) {
  xSemaphoreTake(xBinarySemaphore,portMAX_DELAY);
  
  //TEMPERATURA
  Serial.print("Temperature = ");
  Serial.print(sensorValues.TEMPERATURE);
  Serial.print(" °C ");
  lcd.setCursor(0,0);
  lcd.print("T=");
  lcd.setCursor(2,0);
  lcd.print(sensorValues.TEMPERATURE);
  lcd.setCursor(7,0);
  lcd.print((char)223);
  lcd.setCursor(8,0);
  lcd.print("C");

  //UMIDITATE
  Serial.print("  Humidity = ");
  Serial.print( sensorValues.HUMIDITY);
  Serial.print(" % ");
  lcd.setCursor(0,1);
  lcd.print("H= ");
  lcd.setCursor(2,1);
  lcd.print(sensorValues.HUMIDITY);
  lcd.setCursor(4,1);
  lcd.print("%");

  //PLOAIE
  Serial.print("  Rain value = ");
  Serial.print(sensorValues.RAIN_SENSOR);
  lcd.setCursor(6,1);
  lcd.print("RAIN=");
  if (sensorValues.RAIN_SENSOR < 900) {
    lcd.setCursor(12,1);
    lcd.print("TR");
  }
  else {
    lcd.setCursor(12,1);
    lcd.print("FA");
  }
  

  //LUMINA
  Serial.print("  Light value = ");
  Serial.print(sensorValues.LIGHT_SENSOR);
  Serial.println(" ");
  lcd.setCursor(10,0);
  lcd.print("L= ");
    lcd.setCursor(13,0);
  if (sensorValues.LIGHT_SENSOR > 50) {
    for (int i=0; i<NUM_LEDS; i++){
      leds[i] = CRGB(255, 255, 255);
    FastLED.show();
    }
    lcd.print("ON");
    lcd.setCursor(15,0);
    lcd.print("");
  }
  else {
    for (int i=0; i<NUM_LEDS; i++){
      leds[i] = CRGB(0, 0, 0);
    FastLED.show();
    }
    lcd.print("OF");
  }
  xSemaphoreGive(xBinarySemaphore);
  vTaskDelay(100/portTICK_PERIOD_MS);
  }
  
}
