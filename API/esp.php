<?php

// For the Database :
$servername = "localhost";//ip to connect to the database server
$dbname = "StationMeteo";//database's name
$username = "root";
$password = "burobot2019";

//api key, have to match with the one on the arduino
$api_key_value = "neil";

$api_key= $humidity = $temp_h = $pressure = $light_lvl = $winddir = $windspeedmph = $windspdmph_avg5m = $winddir_avg5m = $rainin = $rainValue = "";

//echo $_POST;

if ($_SERVER["REQUEST_METHOD"] == "POST") {
	//get the key
    $api_key = extract($_POST["api_key"]);
    if($api_key == $api_key_value) {
    	//get all the values
        $humidity = extract($_POST["humidity"]);
        $temp_h = extract($_POST["temp_h"]);
        $pressure = extract($_POST["pressure"]);
        $light_lvl = extract($_POST["light_lvl"]);
        $winddir = extract($_POST["winddir"]);
        $windspeedmph = extract($_POST["windspeedmph"]);
        $windspdmph_avg5m = extract($_POST["windspdmph_avg5m"]);
        $winddir_avg5m = extract($_POST["winddir_avg5m"]);
        $rainin = extract($_POST["rainin"]);
        $rainValue = extract($_POST["rainValue"]);
        
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        //send the data to the database
        $sql = "INSERT INTO SensorData (humidity, temp_h, pressure, light_lvl, winddir, windspeedmph, windspdmph_avg5m, winddir_avg5m, rainin, rainValue) VALUES ('" . $humidity . "', '" . $temp_h . "', '" . $pressure . "', '" . $light_lvl . "', '" . $winddir . "', '" . $windspeedmph . "', '" . $windspdmph_avg5m . "', '" . $winddir_avg5m . "', '" . $rainin . "', '" . $rainValue . "')";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }

}
else {
    echo "No data posted with HTTP POST.";
}

function extract($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}
