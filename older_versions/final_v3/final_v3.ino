//--------------------------------------------  UX  --------------------------------------------
#include "SoftwareSerial.h"

//--------------------------------------------  SENSOR  --------------------------------------------
#define WIND_DIR_AVG_SIZE 300
#include <Wire.h> //I2C needed for sensors
#include "SparkFunMPL3115A2.h" //Pressure sensor - Search "SparkFun MPL3115" and install from Library Manager
#include "SparkFun_Si7021_Breakout_Library.h" //Humidity sensor - Search "SparkFun Si7021" and install from Library Manager

MPL3115A2 myPressure; //Create an instance of the pressure sensor
Weather myHumidity;//Create an instance of the humidity sensor

//PINS
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;
const byte STAT_BLUE = 7;
const byte STAT_GREEN = 8;
const byte WDIR = A0;
const byte WSPEED = 3;
const byte RAIN = 2;
const byte RainSensor = A8;

//variables
int rainValue;
float humidity;
float temp_h;
float pressure;
float light_lvl;
long lastWindCheck = 0;
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;
byte seconds; //When it hits 60, increase the current minute
byte seconds_5m; //Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
byte minutes; //Keeps track of where we are in various arrays of data
byte minutes_10m; //Keeps track of where we are in wind gust/dir over last 10 minutes array of data
byte windspdavg[300]; //120 bytes to keep track of 2 minute average

int winddiravg[WIND_DIR_AVG_SIZE]; //120 ints to keep track of 2 minute average
volatile float rainHour[60]; //60 floating numbers to keep track of 60 minutes of rain

//These are all the weather values that wunderground expects:
int winddir = 0; // [0-360 instantaneous wind direction]
float windspeedmph = 0; // [mph instantaneous wind speed]
float windspdmph_avg5m = 0; // [mph 2 minute average wind speed mph]
int winddir_avg5m = 0; // [0-360 2 minute average wind direction]
float rainin = 0; // [rain inches over the past hour)] -- the accumulated rainfall in the past 60 min

// volatiles are subject to modification by IRQs
volatile unsigned long raintime, rainlast, raininterval, rain;
float currentSpeed;
int currentDirection;

long lastSecond; //The millis counter to see when a second rolls by

//--------------------------------------------  ESP8266  --------------------------------------------
SoftwareSerial esp(15,14);// RX, TX

//credentials
String ssid ="Bluetooth";
String password="blueblue4";

String data;
String server = "192.168.43.84"; // www.example.com
String uri = "/esp.php";// our example is /esppost.php

//--------------------------------------------  SETUP  --------------------------------------------
void setup() {
  //------  UX  -----
  Serial.begin(115200);
  Serial.println("Initializing the system");
  
  //------  ESP8266  -----
  esp.begin(115200);
  reset();
  connectWifi();
  Serial.println("Esp Ready");

  //------  SENSORS  -----
  pinMode(STAT_BLUE, OUTPUT); //Status LED Blue
  pinMode(STAT_GREEN, OUTPUT); //Status LED Green
  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);
  pinMode(WSPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
  pinMode(RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor
  
  // attach external interrupt pins to IRQ functions
  attachInterrupt(0, rainIRQ, FALLING);
  attachInterrupt(1, wspeedIRQ, FALLING);
  interrupts();// turn on interrupts
  
  startPressureSensor();
  Serial.println("Sensors Ready");
  
  lastSecond = millis();
  seconds = 0;
  
  Serial.println("System ready!");
}

//--------------------------------------------  LOOP  --------------------------------------------
void loop()
{
  Serial.println("");
  if (millis() - lastSecond >= 3000)//every 3 second
  {
    digitalWrite(STAT_BLUE, LOW); //Blink stat LED
    lastSecond += 1000;
    
    Serial.println("Reading sensors ");  
    if (readSensors()){
      readWindRain();
      calcWeather(); //Go calc all the various sensors
      data = printValues();
      
      Serial.println(data);
      Serial.println("Sending values via http post method");  
      httppost();
    }
    digitalWrite(STAT_BLUE, HIGH); //Turn off stat LED
  }

  delay(100);
}


//--------------------------------------------  SENSOR  --------------------------------------------
void readWindRain(){
  rainValue = analogRead(RainSensor);
  
  //------  WIND AND RAIN  -----
  //Take a speed and direction reading every second for 2 minute average
  if (++seconds_5m > 5*60-1) seconds_5m = 0;

  //Calc the wind speed and direction every second for 120 second to get 2 minute average
  currentSpeed = get_wind_speed();
  windspeedmph = currentSpeed;//update global variable for windspeed when using the printWeather() function
  //float currentSpeed = random(5); //For testing
  currentDirection = get_wind_direction();
  windspdavg[seconds_5m] = (int)currentSpeed;
  winddiravg[seconds_5m] = currentDirection;
  //if(seconds_2m % 10 == 0) displayArrays(); //For testing
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


//--------------------------------------------  UX  --------------------------------------------
String printValues(){
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.print("%,");
  
  Serial.print(" temp_h = ");
  Serial.print(temp_h, 2);
  Serial.print("F,");
  
  //Check Pressure Sensor
  Serial.print(" Pressure = ");
  Serial.print(pressure);
  Serial.print("Pa,");
  
  //Check light sensor
  Serial.print(" light_lvl = ");
  Serial.print(light_lvl);
  Serial.print("V,");
  
  Serial.print(" winddir = ");
  Serial.print(winddir);
  Serial.print(",");
  
  Serial.print(" windspeedmph = ");
  Serial.print(windspeedmph, 2);
  Serial.print(",");
  
  Serial.print(" windspdmph_avg5m = ");
  Serial.print(windspdmph_avg5m, 2);
  Serial.print(",");
  
  Serial.print(" winddir_avg5m = ");
  Serial.print(winddir_avg5m);
  Serial.print(",");
  
  Serial.print(" rainin = ");
  Serial.print(rainin,2);
  Serial.print(",");
  
  Serial.print(" rainValue = ");
  Serial.print(rainValue);
  Serial.println("");

  return ("hum=" + String(humidity) + "&temph=" + String(temp_h) + "&press=" + String(pressure) + "&light=" + String(light_lvl) + "&winddir=" + String(winddir) + "&windspeedmph=" + String(windspeedmph) + "&windspdmph_avg5m=" + String(windspdmph_avg5m) + "&winddir_avg5m=" + String(winddir_avg5m) + "&rainin=" + String(rainin) + "&rainValue=" + String(rainValue));
}


//--------------------------------------------  ESP8266  --------------------------------------------

//reset the esp8266 module
void reset() {
  esp.println("AT+RST");
  delay(1000);
  
  while (esp.available()){
     String inData = esp.readStringUntil('\n');
     Serial.println("Got reponse from ESP8266: " + inData);
  }
  Serial.println("ESP reset done");
}

//connect to the wifi network
void connectWifi() {
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);
  Serial.println("ESP connection done");
}

void httppost () {
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
  Serial.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");
  
  delay(1000);
  String postRequest =
    "POST " + uri + " HTTP/1.0\r\n" +
    "Host: " + server + "\r\n" +
    "Accept: *" + "/" + "*\r\n" +
    "Content-Length: " + data.length() + "\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "\r\n" + data;
    
  String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.
  esp.print(sendCmd);
  esp.println(postRequest.length());
  delay(500);
  //if(esp.find(">")) { 
    Serial.println("Sending.."); esp.print(postRequest);
    //if( esp.find("SEND OK")) { 
      Serial.println("Packet sent");
      while (esp.available()) {
        String tmpResp = esp.readString();
        Serial.println(tmpResp);
      }
      // close the connection
      esp.println("AT+CIPCLOSE");
    //}
  //}
}
