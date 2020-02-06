//--------------------------------------------  UX  --------------------------------------------

#include "sensors.h"
#include "wifi.h"


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
int rainValue;
byte seconds; //When it hits 60, increase the current minute
byte seconds_5m; //Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
byte minutes_10m; //Keeps track of where we are in wind gust/dir over last 10 minutes array of data

// volatiles are subject to modification by IRQs
volatile unsigned long rain;

long lastSecond; //The millis counter to see when a second rolls by

//--------------------------------------------  ESP8266  --------------------------------------------
SoftwareSerial esp(15,14);// RX, TX



//--------------------------------------------  SETUP  --------------------------------------------
void setup() {
  //------  UX  -----
  Serial.begin(115200);
  Serial.println("Initializing the system");
  
  //------  ESP8266  -----
  esp.begin(115200);
  reset(esp);
  connectWifi(esp);
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
      readWindRain(rainValue,seconds_5m);
      calcWeather(); //Go calc all the various sensors
      data = "A CORRIGER";//printValues();
      /////////////////////////////////////////////////////////////////////////////////////A CORRIGER
      
      Serial.println(data);
      Serial.println("Sending values via http post method");  
      httppost(esp, data);
    }
    digitalWrite(STAT_BLUE, HIGH); //Turn off stat LED
  }

  delay(100);
}


//--------------------------------------------  SENSOR  --------------------------------------------



//--------------------------------------------  UX  --------------------------------------------
/*String printValues(){
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
*/

//--------------------------------------------  ESP8266  --------------------------------------------
