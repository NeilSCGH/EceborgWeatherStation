#include <Wire.h> //I2C needed for sensors
#include <Arduino.h>
#include "SparkFunMPL3115A2.h" //Pressure sensor - Search "SparkFun MPL3115" and install from Library Manager
#include "SparkFun_Si7021_Breakout_Library.h" //Humidity sensor - Search "SparkFun Si7021" and install from Library Manager

#define WIND_DIR_AVG_SIZE 300

const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;
const byte RainSensor = A8;
long lastWindCheck = 0;
volatile byte windClicks = 0;
float windspeedmph = 0; // [mph instantaneous wind speed]
int currentDirection;
float humidity;
float temp_h;
float pressure;
float light_lvl;
const byte WDIR = A0;
byte windspdavg[300]; //120 bytes to keep track of 2 minute average
int winddiravg[WIND_DIR_AVG_SIZE]; //120 ints to keep track of 2 minute average
volatile unsigned long raintime, rainlast, raininterval;
volatile float rainHour[60]; //60 floating numbers to keep track of 60 minutes of rain
byte minutes; //Keeps track of where we are in various arrays of data
volatile long lastWindIRQ = 0;

//These are all the weather values that wunderground expects:
float rainin = 0; // [rain inches over the past hour)] -- the accumulated rainfall in the past 60 min
int winddir = 0; // [0-360 instantaneous wind direction]
float windspdmph_avg5m = 0; // [mph 2 minute average wind speed mph]
int winddir_avg5m = 0; // [0-360 2 minute average wind direction]

MPL3115A2 myPressure; //Create an instance of the pressure sensor
Weather myHumidity;//Create an instance of the humidity sensor

//Read the wind direction sensor, return heading in degrees
int get_wind_direction()
{
  unsigned int adc;

  adc = analogRead(WDIR); // get the current reading from the sensor

  // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
  // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
  // Note that these are not in compass degree order! See Weather Meters datasheet for more information.

  if (adc < 380) return (113);
  if (adc < 393) return (68);
  if (adc < 414) return (90);
  if (adc < 456) return (158);
  if (adc < 508) return (135);
  if (adc < 551) return (203);
  if (adc < 615) return (180);
  if (adc < 680) return (23);
  if (adc < 746) return (45);
  if (adc < 801) return (248);
  if (adc < 833) return (225);
  if (adc < 878) return (338);
  if (adc < 913) return (0);
  if (adc < 940) return (293);
  if (adc < 967) return (315);
  if (adc < 990) return (270);
  return (-1); // error, disconnected?
}


//Returns the instataneous wind speed
float get_wind_speed()
{
  float deltaTime = millis() - lastWindCheck; //750ms

  deltaTime /= 1000.0; //Covert to seconds

  float windSpeed = (float)windClicks / deltaTime; //3 / 0.750s = 4

  windClicks = 0; //Reset and start watching for new wind
  lastWindCheck = millis();

  windSpeed *= 1.492; //4 * 1.492 = 5.968MPH

  /* Serial.println();
    Serial.print("Windspeed:");
    Serial.println(windSpeed);*/

  return (windSpeed);
}

void readWindRain(int rainValue, int seconds_5m){
  rainValue = analogRead(RainSensor);
  
  //------  WIND AND RAIN  -----
  //Take a speed and direction reading every second for 2 minute average
  if (++seconds_5m > 5*60-1) seconds_5m = 0;

  //Calc the wind speed and direction every second for 120 second to get 2 minute average
  float currentSpeed = get_wind_speed();
  windspeedmph = currentSpeed;//update global variable for windspeed when using the printWeather() function
  //float currentSpeed = random(5); //For testing
  currentDirection = get_wind_direction();
  windspdavg[seconds_5m] = (int)currentSpeed;
  winddiravg[seconds_5m] = currentDirection;
  //if(seconds_2m % 10 == 0) displayArrays(); //For testing
}

void startPressureSensor(){
  //Configure the pressure sensor
  myPressure.begin(); // Get sensor online
  myPressure.setModeBarometer(); // Measure pressure in Pascals from 20 to 110 kPa
  myPressure.setOversampleRate(7); // Set Oversample to the recommended 128
  myPressure.enableEventFlags(); // Enable all three pressure and temp event flags
  myHumidity.begin();//Configure the humidity sensor
}

//Returns the voltage of the light sensor based on the 3.3V rail
//This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
float get_light_level()
{
  float operatingVoltage = analogRead(REFERENCE_3V3);

  float lightSensor = analogRead(LIGHT);

  operatingVoltage = 3.3 / operatingVoltage; //The reference voltage is 3.3V

  lightSensor = operatingVoltage * lightSensor;

  return (lightSensor);
}

bool readSensors(){  
  humidity = myHumidity.getRH();//Check Humidity Sensor
  if (humidity == 998) //Humidity sensor failed to respond
  {
    Serial.println("I2C communication to sensors is not working. Check solder connections.");
    startPressureSensor();//Try re-initializing the I2C comm and the sensors
    return false;
  }
  else
  {
    temp_h = myHumidity.getTempF();//Check Temperature Sensor
    pressure = myPressure.readPressure();//Check Pressure Sensor
    light_lvl = get_light_level();//Check light sensor
    return true;
  }
}

//Returns the voltage of the raw pin based on the 3.3V rail
//This allows us to ignore what VCC might be (an Arduino plugged into USB has VCC of 4.5 to 5.2V)
//Battery level is connected to the RAW pin on Arduino and is fed through two 5% resistors:
//3.9K on the high side (R1), and 1K on the low side (R2)
float get_battery_level()
{
  float operatingVoltage = analogRead(REFERENCE_3V3);

  float rawVoltage = analogRead(BATT);

  operatingVoltage = 3.30 / operatingVoltage; //The reference voltage is 3.3V

  rawVoltage = operatingVoltage * rawVoltage; //Convert the 0 to 1023 int to actual voltage on BATT pin

  rawVoltage *= 4.90; //(3.9k+1k)/1k - multiple BATT voltage by the voltage divider to get actual system voltage

  return (rawVoltage);
}

void rainIRQ()
// Count rain gauge bucket tips as they occur
// Activated by the magnet and reed switch in the rain gauge, attached to input D2
{
  raintime = millis(); // grab current time
  raininterval = raintime - rainlast; // calculate interval between this and last event

  if (raininterval > 10) // ignore switch-bounce glitches less than 10mS after initial edge
  {
    rainHour[minutes] += 0.011; //Increase this minute's amount of rain

    rainlast = raintime; // set up for next event
  }
}

void wspeedIRQ()
// Activated by the magnet in the anemometer (2 ticks per rotation), attached to input D3
{
  if (millis() - lastWindIRQ > 10) // Ignore switch-bounce glitches less than 10ms (142MPH max reading) after the reed switch closes
  {
    lastWindIRQ = millis(); //Grab the current time
    windClicks++; //There is 1.492MPH for each click per second.
  }
}


//Calculates each of the variables that wunderground is expecting
void calcWeather()
{
  //Calc winddir
  winddir = get_wind_direction();

  //Calc windspdmph_avg5m
  float temp = 0;
  for (int i = 0 ; i < 60*5 ; i++)
    temp += windspdavg[i];
  temp /= 60*5;
  windspdmph_avg5m = temp;

  //Calc winddir_avg2m, Wind Direction
  //You can't just take the average. Google "mean of circular quantities" for more info
  //We will use the Mitsuta method because it doesn't require trig functions
  //And because it sounds cool.
  //Based on: http://abelian.org/vlf/bearings.html
  //Based on: http://stackoverflow.com/questions/1813483/averaging-angles-again
  long sum = winddiravg[0];
  int D = winddiravg[0];
  for (int i = 1 ; i < WIND_DIR_AVG_SIZE ; i++)
  {
    int delta = winddiravg[i] - D;

    if (delta < -180)
      D += delta + 360;
    else if (delta > 180)
      D += delta - 360;
    else
      D += delta;

    sum += D;
  }
  winddir_avg5m = sum / WIND_DIR_AVG_SIZE;
  if (winddir_avg5m >= 360) winddir_avg5m -= 360;
  if (winddir_avg5m < 0) winddir_avg5m += 360;

  //Total rainfall for the day is calculated within the interrupt
  //Calculate amount of rainfall for the last 60 minutes
  rainin = 0;
  for (int i = 0 ; i < 60 ; i++)
    rainin += rainHour[i];
}
