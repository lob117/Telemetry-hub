
/*
void setup() {
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  Serial.println("ESP32 arrancó");
}

void loop() {
  Serial.println("encendido");
  digitalWrite(2, LOW);
  delay(1000);
  digitalWrite(2, HIGH);
  delay(1000);
}*/
#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
TinyGPSPlus gps;
HardwareSerial GPS(2);
float num_sat, lat,lon;
void setup()
{
  Serial.begin(115200);
  GPS.begin(9600, SERIAL_8N1, 16, 17); // uso de rx tx 2 evitando 1 memoria flash
  Serial.println("Esperando datos GPS...");
}

/* esto es para ver si el dato esta mandando
void loop() {
  while (GPS.available()) {
    char c = GPS.read();
    Serial.write(c);
  }
}*/
void loop()
{
  while (GPS.available())
  {
    gps.encode(GPS.read());
  }

  if (gps.location.isUpdated())
  {
    lat=gps.location.lat();
    lon=gps.location.lng();
    Serial.print("Lat: ");
    Serial.println(lat, 6);
    Serial.print("Lon: ");
    Serial.println(lon, 6);
  }
  if (gps.satellites.isValid())
  {
      Serial.print("Satelites: ");
      Serial.println(gps.satellites.value());
      num_sat = gps.satellites.value();
  }
}
/*
TinyGPSPlus gps;
#define gpsSerial Serial2
void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);
  Serial.println("Waiting for GPS fix and satellites...");
}
void loop() {
  while (gpsSerial.available() > 0)
    if (gps.encode(gpsSerial.read()))
      displayLocationInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
  delay(1000);
}
void displayLocationInfo() {
  Serial.println(F("-------------------------------------"));
  Serial.println("\n Location Info:");

  Serial.print("Latitude:  ");
  Serial.print(gps.location.lat(), 6);
  Serial.print(" ");
  Serial.println(gps.location.rawLat().negative ? "S" : "N");

  Serial.print("Longitude: ");
  Serial.print(gps.location.lng(), 6);
  Serial.print(" ");
  Serial.println(gps.location.rawLng().negative ? "W" : "E");

  Serial.print("Fix Quality: ");
  Serial.println(gps.location.isValid() ? "Valid" : "Invalid");

  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());

  Serial.print("Altitude:   ");
  Serial.print(gps.altitude.meters());
  Serial.println(" m");

  Serial.print("Speed:      ");
  Serial.print(gps.speed.kmph());
  Serial.println(" km/h");

  Serial.print("Course:     ");
  Serial.print(gps.course.deg());
  Serial.println("°");

  Serial.print("Date:       ");
  if (gps.date.isValid()) {
    Serial.printf("%02d/%02d/%04d\n", gps.date.day(), gps.date.month(), gps.date.year());
  } else {
    Serial.println("Invalid");
  }

  Serial.print("Time (UTC): ");
  if (gps.time.isValid()) {
    Serial.printf("%02d:%02d:%02d\n", gps.time.hour(), gps.time.minute(), gps.time.second());
  } else {
    Serial.println("Invalid");
  }

  Serial.println(F("-------------------------------------"));
}*/