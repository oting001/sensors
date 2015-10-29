#include "DHT.h"
#include <Wire.h>
#include <BH1750.h>
#include <ArduinoJson.h>

/*
BH1750

Connection:
VCC-5v
GND-GND
SCL-SCL(analog pin 5)
SDA-SDA(analog pin 4)
ADD-NC or GND
*/
//read light
BH1750 lightMeter;
uint16_t lux = 0;

#define VCC 5

//read Temperature
int inputDHT22 = 4;
DHT dht(inputDHT22, DHT22); //dht type = DHT22 = AM2302
float humidity = 0.0;
float celsius = 0.0;

// read wind vane
int inputWindVane = A0;
float directionWind = 0.0;
float voltageWind = 0.0;
int adcWindVane = 0;

// read RPM
int inputAnemometer = 2;
int intAnem = digitalPinToInterrupt(inputAnemometer); // interrupt of Anemometer

// read rain
int inputRain = 3;
int intRain = digitalPinToInterrupt(inputRain); // interrupt of rain

// read current
int inputCurrent1 = A1;
float current1 = 0;

int inputCurrent2 = A2;
float current2 = 0;

int inputCurrent3 = A3;
float current3 = 0;

void setup() {
  
  Serial.begin(9600);

  pinMode(inputAnemometer, INPUT_PULLUP);
  attachInterrupt(intAnem, wind_count, FALLING);

  pinMode(inputRain, INPUT_PULLUP);
  attachInterrupt(intRain, rain_count, FALLING);

  lightMeter.begin();
}

// read rain
unsigned long rainCounter = 0;
unsigned long lastRainTime = 0;

void rain_count(){
  unsigned long currentTime=(unsigned long) (micros()-lastRainTime);
  lastRainTime=micros();
  if(currentTime>500)   // debounce
  {
     rainCounter++;
  }
}

void reset_rain_counter(){
  rainCounter = 0;
}

#define RAIN_FACTOR 0.2794
float rainfall = 0.0;

void calculating_rain(int sTime){
  rainfall = ((float)rainCounter/sTime) * RAIN_FACTOR;
  rainCounter = 0;
}

unsigned long windCounter = 0;
unsigned long lastWindTime = 0;

void wind_count(){
  unsigned long currentTime=(unsigned long) (micros()-lastWindTime);
  lastWindTime=micros();
  if(currentTime>800)   // debounce
  {
     windCounter++;
  }
}

//wind speed
#define WIND_FACTOR 2.4
float windSpeed = 0.0;

void calculating_wind_speed(int sTime){
  windSpeed = ((float)windCounter/sTime) * WIND_FACTOR;
  windCounter = 0;
}

unsigned long lastmillis = 0;
int sampleTime = 3;

void calculating_TSampling(){
   if ((millis() - lastmillis) > (1000*sampleTime)) {
    calculating_wind_speed(sampleTime);
    calculating_rain(sampleTime);
    lastmillis = millis();
  }
}

boolean fuzzyCompare(float compareValue, float value) {
#define VARYVALUE 0.03
   if ( (value > (compareValue * (1.0-VARYVALUE)))  && (value < (compareValue *(1.0+VARYVALUE))) )
   {
     return true;
   }
   return false;
}

float voltageToDegrees(float value) { // convert volt to degree (change directionWind) 
  
  // Note:  The original documentation for the wind vane says 16 positions.  Typically only recieve 8 positions.  And 315 degrees was wrong.
  
  // For 5V, use 1.0.  For 3.3V use 0.66
#define ADJUST3OR5 1.02

  if (fuzzyCompare(3.84 * ADJUST3OR5 , value))
      directionWind = 0.0;

  if (fuzzyCompare(1.98 * ADJUST3OR5, value))
      directionWind = 22.5;

  if (fuzzyCompare(2.25 * ADJUST3OR5, value))
      directionWind = 45;

  if (fuzzyCompare(0.41 * ADJUST3OR5, value))
      directionWind = 67.5;

  if (fuzzyCompare(0.45 * ADJUST3OR5, value))
      directionWind = 90.0;

  if (fuzzyCompare(0.32 * ADJUST3OR5, value))
      directionWind = 112.5;

  if (fuzzyCompare(0.90 * ADJUST3OR5, value))
      directionWind = 135.0;

  if (fuzzyCompare(0.62 * ADJUST3OR5, value))
      directionWind = 157.5;

  if (fuzzyCompare(1.40 * ADJUST3OR5, value))
      directionWind = 180;

  if (fuzzyCompare(1.19 * ADJUST3OR5, value))
      directionWind = 202.5;

  if (fuzzyCompare(3.08 * ADJUST3OR5, value))
      directionWind = 225;

  if (fuzzyCompare(2.93 * ADJUST3OR5, value))
      directionWind = 247.5;

  if (fuzzyCompare(4.62 * ADJUST3OR5, value))
      directionWind = 270.0;

  if (fuzzyCompare(4.04 * ADJUST3OR5, value))
      directionWind = 292.5;

  if (fuzzyCompare(4.34 * ADJUST3OR5, value))  // chart in documentation wrong
      directionWind = 315.0;

  if (fuzzyCompare(3.43 * ADJUST3OR5, value))
      directionWind = 337.5;
      
}

unsigned adcCurrent1 = 0;
unsigned adcCurrent2 = 0;
unsigned adcCurrent3 = 0;

#define MAXCURRENT 80
short maxADCCurrent1;
short minADCCurrent1;
short maxADCCurrent2;
short minADCCurrent2;
short maxADCCurrent3;
short minADCCurrent3;

byte sampleCurrent = 20;
byte currentCounter = 0;

void calculation_current(){
  maxADCCurrent1 = 0;
  minADCCurrent1 = 1023;
  maxADCCurrent2 = 0;
  minADCCurrent2 = 1023;
  maxADCCurrent3 = 0;
  minADCCurrent3 = 1023;
  for(currentCounter = 0; currentCounter < sampleCurrent;currentCounter++){
    adcCurrent1 = analogRead(inputCurrent1);
    adcCurrent2 = analogRead(inputCurrent2);
    adcCurrent3 = analogRead(inputCurrent3);
    if(maxADCCurrent1 < adcCurrent1){
      maxADCCurrent1 = adcCurrent1;
    }
    if(minADCCurrent1 > adcCurrent1){
      minADCCurrent1 = adcCurrent1;
    }
    if(maxADCCurrent2 < adcCurrent2){
      maxADCCurrent2 = adcCurrent2;
    }
    if(minADCCurrent2 > adcCurrent2){
      minADCCurrent2 = adcCurrent2;
    }
    if(maxADCCurrent3 < adcCurrent3){
      maxADCCurrent3 = adcCurrent3;
    }
    if(minADCCurrent3 > adcCurrent3){
      minADCCurrent3 = adcCurrent3;
    }
  }
        
  current1 = ((abs((523-minADCCurrent1)+(maxADCCurrent1-526))/2.0)/512)*MAXCURRENT;
  current2 = ((abs((523-minADCCurrent2)+(maxADCCurrent2-526))/2.0)/512)*MAXCURRENT;
  current3 = ((abs((523-minADCCurrent3)+(maxADCCurrent3-526))/2.0)/512)*MAXCURRENT;

}

StaticJsonBuffer<200> jsonBuffer;

JsonObject& jsonObject = jsonBuffer.createObject();
JsonObject& voltJson = jsonObject.createNestedObject("voltage");
JsonObject& currentJson = jsonObject.createNestedObject("current");
JsonObject& tempJson = jsonObject.createNestedObject("temperature");

void loop() {
              
  delay(100); { //Update every one second, this will be equal to reading frequency (Hz).
  
    //Wind Vane
    adcWindVane = analogRead(inputWindVane);
    voltageWind = (adcWindVane / 1023.0) * VCC; // adc to volt
    voltageToDegrees(voltageWind);
    
    //Temperature
    celsius = dht.readTemperature();

    //Humidity
    humidity = dht.readHumidity();

    //light
    lux = lightMeter.readLightLevel();

//    testPrintWindVane();
//    testPrintRain();
//    testPrintTemperature();
//    testPrintHumidity();
//    testPrintWindSpeed();
//    testPrintLux();
//    testPrintCurrent();xc
    Serial.println();
    calculating_TSampling();
    setJson();
    jsonObject.printTo(Serial);
  }
}

void testPrintWindVane(){
  //Serial.print(" adc = ");
  //Serial.print(adcWindVane);
  //Serial.print(" Volt = ");
  //Serial.print(voltageWind);
  Serial.print(" Degree = ");
  Serial.println(directionWind);
}
void testPrintRain(){
  Serial.print(" rainCounter = ");
  Serial.print(rainCounter);
  Serial.print(" totalRain = ");
  Serial.println(rainfall);
}

void testPrintTemperature(){
  Serial.print(" Temperature = ");
  Serial.println(celsius);
}

void testPrintHumidity(){
  Serial.print(" Humidity = ");
  Serial.println(humidity);
}

void testPrintWindSpeed(){
  Serial.print(" Speed = ");
  Serial.println(windSpeed);
}

void testPrintLux(){
  Serial.print(" Light = ");
  Serial.println(lux/1.5);
}

void testPrintCurrent(){
  Serial.print(" Current = ");
  Serial.println(current1);
}

void setJson(){
   jsonObject["light"] = lux/1.5;
   jsonObject["humidity"] = humidity;
   tempJson["inn"]= celsius;
   tempJson["out"]= celsius;   
   jsonObject["rainfall"] = rainfall;
   jsonObject["windspeed"] = windSpeed; 
   currentJson["c1"] = current1;
   currentJson["c2"] = current1;
   currentJson["c3"] = current1;
   jsonObject["degree"] = directionWind;
   voltJson["v1"] = 0;
   voltJson["v2"] = 1;
   voltJson["v3"] = 2;
}