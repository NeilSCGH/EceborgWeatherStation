# Eceborg-Weather-Station

This project contains the code of the connected weather station. This weather station will record various weather data and send them every 5 minutes to a database by wifi.

## Getting Started

### Prerequisites

You'll need a working web server to host the php file (in the API folder), an SQL server to store data, and all the hardware with sensors etc. 

### Installing

You'll have to change (in the arduino folder):
* SSID and PASSWORD in the wifi.cpp file
* server and uri in the same file
* the key in the sensors.cpp file (have to match the key in the API/esp.php file)

## Running the tests

Make sure the 2 parts are working properly.

### Test the hardware part

If the variable 'v' (for verbose) is set to true in the arduino, you'll have all the data printed through serial. ('vw' in the wifi file and 'vs' in the sensor file).

### Test the sofware part

You can use the test.html file to simulate the post method made by the arduino. Make sure to write the direct ip of the web server, and the path to the php file before testing.

## Authors

* **Neil Segard** - [NeilSCGH](https://github.com/NeilSCGH)

With the help of 
* **Nicolas Chollet**  - [NicolasChollet51](https://github.com/NicolasChollet51)
* **ECEBORG**  - [ECEBORG](https://github.com/ECEBORG)

## Acknowledgments

Voila voila enjoy :)