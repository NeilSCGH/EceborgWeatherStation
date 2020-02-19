#include "SoftwareSerial.h"
String ssid ="Bluetooth";
String password="blueblue3";

SoftwareSerial esp(15,14);// RX, TX

String data;
String server = "yourServer"; // www.example.com
String uri = "yourURI";// our example is /esppost.php

void setup() {
  esp.begin(115200);
  Serial.begin(115200);
  Serial.println("hey");
  reset();
  connectWifi();
}

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

void loop () {
  data = "temperature=3&humidity=4";
  Serial.println(data);
  //httppost();
  delay(1000);
}

void httppost () {
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
  
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
  if(esp.find(">")) { 
    Serial.println("Sending.."); esp.print(postRequest);
    if( esp.find("SEND OK")) { 
      Serial.println("Packet sent");
      while (esp.available()) {
        String tmpResp = esp.readString();
        Serial.println(tmpResp);
      }
      // close the connection
      esp.println("AT+CIPCLOSE");
    }
  }
}
