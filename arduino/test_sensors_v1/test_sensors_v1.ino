//Libraries 
#include <DHT.h>
#include <DHT_U.h>

//Soil moisture sensor calibrations variables
float wet_value = 367; //value mesured in water
float dry_value = 750; //value mesured in the open-air

//Sensors pins definition
#define DHTPIN 4 //DHT22 pin
#define DHTTYPE DHT22 //there are multiple kinds of DHT sensors
const int SOIL_HUMIDITY_SENSOR_PIN = A0;

//Creation of the DHT object
DHT dht(DHTPIN, DHTTYPE);

float air_humidity,temperature,soil_humidity,heat_index;

void setup()
{
  //Serial initialization
  Serial.begin(9600);
}

void loop()
{
  air_humidity = dht.readHumidity();
  temperature = dht.readTemperature();//in Celsius
  
  if (isnan(air_humidity) || isnan(temperature)) // Check if any reads failed and exit early (to try again).
  {
    Serial.println("Failed to read from DHT sensor!");
    //return;
  }
  
  soil_humidity = analogRead(SOIL_HUMIDITY_SENSOR_PIN);
  soil_humidity = 100 - (soil_humidity - wet_value)/(dry_value - wet_value)*100; //formula using calibrations variables of the sensor to have a percentage instead of a simple value 
  //soil_humidity = map(soil_humidity, wet_value, dry_value, 100, 0);//plus propre il me semble, A COMPARER
  
  heat_index = dht.computeHeatIndex(temperature, air_humidity, false);
  
  Serial.println(air_humidity);
  Serial.println(soil_humidity);
  Serial.println(temperature);
  Serial.println(heat_index);
  delay(1000);
}
