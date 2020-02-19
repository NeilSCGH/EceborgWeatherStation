#include "SoftwareSerial.h"
#include <Arduino.h>

//reset the esp8266 module
void reset(SoftwareSerial esp);

//connect to the wifi network
void connectWifi(SoftwareSerial esp);

void httppost (SoftwareSerial esp, String data) ;
