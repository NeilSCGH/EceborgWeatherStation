/*
 * Code made by Neil Ségard
 * contact : neil.segard@gmail.com
 * Jan 2020
 */


//----------------------------  SETTINGS  ----------------------------
int delai=5*60*1000;  //delai in ms between 2 data record (here 5 minutes in milliseconds)
bool v=true;          //enable verbose or not (serial.print)


//----------------------------  LIBRAIRIES  ----------------------------
//including the self made librairies for sensors and wifi
#include "sensors.h"
#include "wifi.h"
SoftwareSerial esp(18,19);// RX, TX


//----------------------------  DECLARATIONS  ----------------------------
//PINS declarations
const byte REFERENCE_3V3 = A3;
const byte STAT_GREEN = 8;
const byte STAT_BLUE = 7;
const byte WSPEED = 3;
const byte LIGHT = A1;
const byte BATT = A2;
const byte RAIN = 2;

//variables
String data;
long lastSecond; //The millis counter to see when a second rolls by



//----------------------------  SETUP  ----------------------------
void setup() {
  Serial.begin(115200);
  if(v) Serial.println("Initializing the system");
  
  //------  ESP8266  -----
  esp.begin(115200);
  reset(esp);
  connectWifi(esp);
  if(v) Serial.println("Esp Ready");

  //------  PINS INITIALIZATION  -----
  pinMode(STAT_BLUE, OUTPUT); //Status LED Blue
  pinMode(STAT_GREEN, OUTPUT); //Status LED Green
  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);
  pinMode(WSPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
  pinMode(RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor
  if(v) Serial.println("Pins Ready");
  
  // attach external interrupt pins to IRQ functions
  attachInterrupt(0, rainIRQ, FALLING);
  attachInterrupt(1, wspeedIRQ, FALLING);
  interrupts();// turn on interrupts
  if(v) Serial.println("Interrupts Ready");
  
  startPressureSensor();
  if(v) Serial.println("Sensors Ready");
  
  lastSecond = millis();//starts the counter
  
  if(v) Serial.println("System Ready!");
}

//--------------------------------------------  LOOP  --------------------------------------------
void loop()
{
  if (millis() - lastSecond >= delai)//runs periodicaly
  {
    if(v) Serial.println("");
    if(v) Serial.println("Routine started...");
    digitalWrite(STAT_BLUE, LOW); //Blink stat LED
    //lastSecond += delai;
    
    if(v) Serial.println("Try to read sensors...");  
    if (readSensors()){
      readWindRain(); //read wind and rain sensors
      calcWeather(); //Go read and calc all the various sensors
      
      if(v) Serial.println("Concatenating Data");  
      data = getData(true);//combine: humidity, temp_h, pressure, light_lvl, winddir, windspeed, windspd_avg5m, winddir_avg5m, rainin AND rainValue
      
      //Serial.println(data);
      if(v) Serial.println("Sending data via http post method");  
      httppost(esp, data);
      if(v) Serial.println("Done");  
    }
    digitalWrite(STAT_BLUE, HIGH); //Turn off stat LED
    if(v) Serial.println("Routine finished...");

    lastSecond=millis();//restart the counter
  }

  delay(100);
}
