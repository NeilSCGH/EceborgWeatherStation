CREATE TABLE SensorData (
    id INT(6) UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    humidity FLOAT(10),
    temp_h FLOAT(10),
    pressure FLOAT(10),
    light_lvl FLOAT(10),
    winddir INT,
    windspeedmph FLOAT(10),
    windspdmph_avg5m FLOAT(10),
    winddir_avg5m FLOAT(10),
    rainin FLOAT(10),
    rainValue FLOAT(10),
    reading_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
)
