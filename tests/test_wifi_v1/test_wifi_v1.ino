#include "SoftwareSerial.h"
String ssid ="Bluetooth";
String password="blueblue";

SoftwareSerial esp(15,14);// RX, TX


void setup() {
  esp.begin(115200);
  Serial.begin(115200);
  Serial.println("hey");
  delay(2000);
  connectWifi();
  Serial.println("finish");
}

void connectWifi() {
  esp.println("AT+RST");
  delay(4000);
  if(esp.find("OK")) {
    Serial.println("ok!");
  }
  else {
    Serial.println("no ok"); 
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}
