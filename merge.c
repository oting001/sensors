#include "DHT.h"
#include <Wire.h>
#include <BH1750.h>

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

float calculating_rain(){
  return RAIN_FACTOR * rainCounter;
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
int sampleTime = 5;

float calculating_wind_speed(){
  
  windCounter = 0;
  delay(sampleTime*1000);
  windSpeed = ((float)windCounter/sampleTime) * WIND_FACTOR;

  return windSpeed;
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
#define ADJUST3OR5 1.0

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

    testPrintWindVane();
    testPrintRain();
    testPrintTemperature();
    testPrintHumidity();
    testPrintWindSpeed();
    testPrintLux();
    Serial.println();
    calculating_wind_speed();
  }
}

void testPrintWindVane(){
  Serial.print(" adc = ");
  Serial.print(adcWindVane);
  Serial.print(" Volt = ");
  Serial.print(voltageWind);
  Serial.print(" Degree = ");
  Serial.println(directionWind);
}
void testPrintRain(){
  Serial.print(" rainCounter = ");
  Serial.print(rainCounter);
  Serial.print(" totalRain = ");
  Serial.println(calculating_rain());
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
  Serial.println(lux);
}