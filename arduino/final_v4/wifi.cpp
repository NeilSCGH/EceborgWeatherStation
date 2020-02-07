#include "SoftwareSerial.h"
#include <Arduino.h>
bool vw=true;

//credentials
String ssid ="Bluetooth";
String password="blueblue4";

String server = "192.168.43.123"; // www.example.com
String uri = "/esp.php";// our example is /esppost.php

//reset the esp8266 module
void reset(SoftwareSerial esp) {
  esp.println("AT+RST");
  delay(1000);
  
  while (esp.available()){
     String inData = esp.readStringUntil('\n');
     if(vw) Serial.println("Got reponse from ESP8266: " + inData);
  }
  if(vw) Serial.println("ESP reset done");
}

//connect to the wifi network
void connectWifi(SoftwareSerial esp) {
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);
  if(vw) Serial.println("ESP connection done");
}

void httppost (SoftwareSerial esp, String data) {
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
  if(vw) Serial.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");
  
  delay(1000);
  String postRequest =
    "POST " + uri + " HTTP/1.0\r\n" +
    "Host: " + server + "\r\n" +
    "Accept: *" + "/" + "*\r\n" +
    "Content-Length: " + data.length() + "\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "\r\n" + data;
    
  String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent.
  if(vw) Serial.println("Sending.."); 
  esp.print(sendCmd);
  esp.println(postRequest.length());
  delay(500);
  esp.print(postRequest);
  if(vw) Serial.print(sendCmd);
  if(vw) Serial.println(postRequest.length());
  if(vw) Serial.print(postRequest);
  if(vw) Serial.println("Packet sent");
  
  // close the connection
  esp.println("AT+CIPCLOSE");
  //}
  //}
}
