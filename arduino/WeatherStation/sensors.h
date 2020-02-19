/*
 * Code made by Neil Ségard
 * contact : neil.segard@gmail.com
 * Jan 2020
 */
 
//prototypes
void readWindRain();
bool readSensors();
void startPressureSensor();
float get_light_level();
float get_battery_level();
void rainIRQ();
void wspeedIRQ();
void calcWeather() ;
float get_wind_speed();
int get_wind_direction();
String getData(bool doPrint);
