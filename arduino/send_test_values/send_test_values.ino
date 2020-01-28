#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif

//Credentials for the wifi connection
const char* ssid     = "Bluetooth";
const char* password = "blueblue2";

// target url for the POST method
const char* serverName = "http://example.com/post-esp-data.php";

// API key
String apiKeyValue = "f45vS8zK5r";

void setup() {
  Serial.begin(115200);
  
  Serial.println("Connecting to WIFI network");
  WiFi.begin(ssid, password);
  
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(serverName);
    http.addHeader("Content-Type", "text/plain");
    String httpRequestData = "api_key=" + apiKeyValue + "&value1=101" + "&value2=102" + "&value3=103";
    
    // Send the request
    int httpResponseCode = http.POST(httpRequestData);

    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  //Send an HTTP POST request every 10 seconds
  delay(10000);  
}
