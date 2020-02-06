#include "sensors.h"
#include "wifi.h"
bool v=true;

//PINS
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;
const byte STAT_BLUE = 7;
const byte STAT_GREEN = 8;
const byte WSPEED = 3;
const byte RAIN = 2;

//variables
String data;
byte seconds; //When it hits 60, increase the current minute
byte seconds_5m; //Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
long lastSecond; //The millis counter to see when a second rolls by
int delai=3000;

//--------------------------------------------  ESP8266  --------------------------------------------
SoftwareSerial esp(15,14);// RX, TX

//--------------------------------------------  SETUP  --------------------------------------------
void setup() {
  //------  UX  -----
  Serial.begin(115200);
  if(v) Serial.println("Initializing the system");
  
  //------  ESP8266  -----
  esp.begin(115200);
  reset(esp);
  connectWifi(esp);
  if(v) Serial.println("Esp Ready");

  //------  SENSORS  -----
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
  
  lastSecond = millis();
  seconds = 0;
  
  if(v) Serial.println("System Ready!");
}

//--------------------------------------------  LOOP  --------------------------------------------
void loop()
{
  if(v) Serial.println("");
  if (millis() - lastSecond >= delai)//every 3 second
  {
    if(v) Serial.println("Routine started...");
    digitalWrite(STAT_BLUE, LOW); //Blink stat LED
    lastSecond += delai;
    
    if(v) Serial.println("Reading sensors...");  
    if (readSensors()){
      readWindRain(seconds_5m);
      calcWeather(); //Go calc all the various sensors
      
      if(v) Serial.println("Combining Data");  
      data = getData(true);//combine: humidity, temp_h, pressure, light_lvl, winddir, windspeedmph, windspdmph_avg5m, winddir_avg5m, rainin AND rainValue
      
      //Serial.println(data);
      if(v) Serial.println("Sending data via http post method");  
      httppost(esp, data);
      if(v) Serial.println("Done");  
    }
    digitalWrite(STAT_BLUE, HIGH); //Turn off stat LED
    if(v) Serial.println("Routine finished...");
  }

  delay(100);
}
