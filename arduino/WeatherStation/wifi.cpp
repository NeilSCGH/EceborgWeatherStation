/*
 * Code made by Neil Ségard
 * contact : neil.segard@gmail.com
 * Jan 2020
 */

#include <Arduino.h>
//----------------------------  SETTINGS  ----------------------------
//wifi credentials
String ssid ="Bluetooth";
String password="blueblue4";

//server with the .php file
String server = "192.168.43.123"; // ip of the web server
String uri = "/esp.php";// url to the php file

bool vw=true; //enable verbose or not (serial.print)


//----------------------------  LIBRAIRIES  ----------------------------
#include "SoftwareSerial.h"



//----------------------------  METHODS  ----------------------------
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
}

//send a post method containing all the data, to the php file on the web server
//The web server will transmit the data to the mysql database after that
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
    
  //determine the number of caracters to be sent.
  String sendCmd = "AT+CIPSEND="; 
  esp.print(sendCmd);
  esp.println(postRequest.length());
  if(vw) Serial.print(sendCmd);
  if(vw) Serial.println(postRequest.length());
  delay(500);

  //send the data
  if(vw) Serial.println("Sending..");
  if(vw) Serial.println(postRequest);
  esp.print(postRequest);
  
  if(vw) Serial.println("Packet sent");
  
  // close the connection
  esp.println("AT+CIPCLOSE");
}
