#include <Arduino.h>
#include "Dps3xx.h"
#include <SD.h>
#include "RTClib.h"
#include <Wire.h>

#define LOG_PIN 10


Dps3xx Dps368PressureSensor = Dps3xx();

RTC_PCF8523 rtc;
DateTime curTime;
File logFile;


void setup() {
  pinMode(LOG_PIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial)
    ;


  // Initialize SD card
  if (!SD.begin(LOG_PIN)) {
    Serial.println("SD card initialization failed!");
    while (1);  // Halt if initialization failed
  }


  // Initialize RTC
  rtc.begin();


  //Call begin to initialize Dps368PressureSensor
  //The parameter 0x76 is the bus address. The default address is 0x77 and does not need to be given.
  //Dps368PressureSensor.begin(Wire, 0x76);
  //Use the commented line below instead to use the default I2C address.
  Dps368PressureSensor.begin(Wire);


  //temperature measure rate (value from 0 to 7)
  //2^temp_mr temperature measurement results per second
  int16_t temp_mr = 2;
  //temperature oversampling rate (value from 0 to 7)
  //2^temp_osr internal temperature measurements per result
  //A higher value increases precision
  int16_t temp_osr = 2;
  //pressure measure rate (value from 0 to 7)
  //2^prs_mr pressure measurement results per second
  int16_t prs_mr = 2;
  //pressure oversampling rate (value from 0 to 7)
  //2^prs_osr internal pressure measurements per result
  //A higher value increases precision
  int16_t prs_osr = 2;
  //startMeasureBothCont enables background mode
  //temperature and pressure ar measured automatically
  //High precision and hgh measure rates at the same time are not available.
  //Consult Datasheet (or trial and error) for more information
  int16_t ret = Dps368PressureSensor.startMeasureBothCont(temp_mr, temp_osr, prs_mr, prs_osr);
  //Use one of the commented lines below instead to measure only temperature or pressure
  //int16_t ret = Dps368PressureSensor.startMeasureTempCont(temp_mr, temp_osr);
  //int16_t ret = Dps368PressureSensor.startMeasurePressureCont(prs_mr, prs_osr);




  if (ret != 0) {
    Serial.print("Init FAILED! ret = ");
    Serial.println(ret);
  } else {
    Serial.println("Init complete!");
  }
  pinMode(LOG_PIN, OUTPUT);
  Serial.begin(9600);
  SD.begin(LOG_PIN);
  rtc.begin();
  char fileName[] = "LOG023.CSV";
  for (int i = 0; i < 1000; i++) {
    fileName[3] = i / 100 + '0';
    fileName[4] = (i % 100) / 10 + '0';
    fileName[5] = i % 10 + '0';
    if (!SD.exists(fileName)) {
      // only open a new file if it doesn't exist
      logFile = SD.open(fileName, FILE_WRITE);
      break;  // leave the loop!
    }
  }
}






void loop() {
  uint8_t pressureCount = 20;
  float pressure[pressureCount];
  uint8_t temperatureCount = 20;
  float temperature[temperatureCount];


  //This function writes the results of continuous measurements to the arrays given as parameters
  //The parameters temperatureCount and pressureCount should hold the sizes of the arrays temperature and pressure when the function is called
  //After the end of the function, temperatureCount and pressureCount hold the numbers of values written to the arrays
  //Note: The Dps368 cannot save more than 32 results. When its result buffer is full, it won't save any new measurement results
  int16_t ret = Dps368PressureSensor.getContResults(temperature, temperatureCount, pressure, pressureCount);


  if (ret != 0) {
    Serial.println();
    Serial.println();
    Serial.print("FAIL! ret = ");
    Serial.println(ret);
  } else {
    Serial.println();
    Serial.println();
    Serial.print(temperatureCount);
    Serial.println(" temperature values found: ");
    for (int16_t i = 0; i < temperatureCount; i++) {
      Serial.print(temperature[i]);
      Serial.println(" degrees of Celsius");
    }


    Serial.println();
    Serial.print(pressureCount);
    Serial.println(" pressure values found: ");
    for (int16_t i = 0; i < pressureCount; i++) {
      Serial.print(pressure[i]);
      Serial.println(" Pascal");
    }
  }


  //Wait some time, so that the Dps368 can refill its buffer
  delay(10000);


  curTime = rtc.now();

  // Save data to SD file //
  logFile.print(curTime.year());
  logFile.print(", ");
  logFile.print(curTime.month());
  logFile.print(", ");
  logFile.print(curTime.day());
  logFile.print(", ");
  logFile.print(curTime.hour());
  logFile.print(", ");
  logFile.print(curTime.minute());
  logFile.print(", ");
  logFile.print(curTime.second());
  logFile.print(", ");


  logFile.println();
  logFile.print(pressureCount);
  logFile.println(" pressure values found: ");
  for (int16_t i = 0; i < pressureCount; i++) {
    logFile.print(pressure[i]);
    logFile.println("Pascal");
  }
  logFile.flush();  // saves the file to the SD card
  delay(10000);
}
