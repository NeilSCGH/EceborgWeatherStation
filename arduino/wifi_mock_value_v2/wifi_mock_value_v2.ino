#include "SoftwareSerial.h"
String ssid ="Bluetooth";
String password="blueblue3";

SoftwareSerial esp(15,14);// RX, TX

String data;
String server = "yourServer"; // www.example.com
String uri = "yourURI";// our example is /esppost.php
int DHpin = 8;//sensor pin
byte dat [5];
String temp ,hum;

void setup() {
  pinMode (DHpin, OUTPUT);
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
}

//connect to the wifi network
void connectWifi() {
  String cmd = "AT+CWJAP=\"" +ssid+"\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);
}

void loop () {
  
  //start_test ();
  // convert the bit data to string form
  hum = "4";//String(dat[0]);
  temp= "3";String(dat[2]);
  data = "temperature=" + temp + "&humidity=" + hum;// data sent must be under this form //name1=value1&name2=value2.
  
  httppost();
  delay(1000);
}

void httppost () {
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");//start a TCP connection.
  if( esp.find("OK")) {
    Serial.println("TCP connection ready");
  } 
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
