//--------------------------------------------  UX  --------------------------------------------
#include "SoftwareSerial.h"


//--------------------------------------------  SENSOR  --------------------------------------------
#include <Wire.h> //I2C needed for sensors
#include "SparkFunMPL3115A2.h" //Pressure sensor - Search "SparkFun MPL3115" and install from Library Manager
#include "SparkFun_Si7021_Breakout_Library.h" //Humidity sensor - Search "SparkFun Si7021" and install from Library Manager
MPL3115A2 myPressure; //Create an instance of the pressure sensor
Weather myHumidity;//Create an instance of the humidity sensor
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;
const byte STAT_BLUE = 7;
const byte STAT_GREEN = 8;
float humidity;
float temp_h;
float pressure;
float light_lvl;

long lastSecond; //The millis counter to see when a second rolls by

//--------------------------------------------  ESP8266  --------------------------------------------
SoftwareSerial esp(15,14);// RX, TX
String ssid ="Bluetooth";
String password="blueblue3";

String data;
String server = "192.168.43.84"; // www.example.com
String uri = "/esp.php";// our example is /esppost.php

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
  startPressureSensor();
  lastSecond = millis();
  Serial.println("Sensors Ready");
  
  Serial.println("System ready!");
}

void loop()
{
  Serial.println("");
  //Print readings every second
  if (millis() - lastSecond >= 1000)
  {
    digitalWrite(STAT_BLUE, HIGH); //Blink stat LED

    lastSecond += 1000;

    //Check Humidity Sensor
    humidity = myHumidity.getRH();

    if (humidity == 998) //Humidity sensor failed to respond
    {
      Serial.println("I2C communication to sensors is not working. Check solder connections.");
      startPressureSensor();//Try re-initializing the I2C comm and the sensors
    }
    else
    {      
      Serial.println("Reading sensors ");      
      readSensors();
      
      data = printValues();
      
      //Serial.println(data);
      Serial.println("Sending values via http post method");  
      httppost();
    }

    digitalWrite(STAT_BLUE, LOW); //Turn off stat LED
  }

  delay(100);
}


//--------------------------------------------  SENSOR  --------------------------------------------
void readSensors(){
  temp_h = myHumidity.getTempF();//Check Temperature Sensor
  pressure = myPressure.readPressure();//Check Pressure Sensor
  light_lvl = get_light_level();//Check light sensor
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
  Serial.println("V");

  return ("hum=" + String(humidity) + "&temph=" + String(temp_h) + "&press=" + String(pressure) + "&light=" + String(light_lvl));
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
  Serial.println("Reset done");
}

//connect to the wifi network
void connectWifi() {
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);
  Serial.println("Connection done");
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
