#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <DHT.h>
#include "esp_sleep.h"
#define DHTPIN 32
#define DHTTYPE DHT11
#include <Arduino.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>

TinyGPSPlus gps;
HardwareSerial GPS(2);

#define AWS_IOT_PUBLISH_TOPIC "esp32/pub1"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub1"

float h, t, th_sen, lat, lon, num_sat;
DHT dht(DHTPIN, DHTTYPE);
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char *topic, byte *payload, unsigned int length)
{
    Serial.print("incoming: ");
    Serial.println(topic);
    JsonDocument doc;
    deserializeJson(doc, payload);
    const char *message = doc["message"];
    Serial.println(message);
}

void connectAWS()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);
    client.setServer(AWS_IOT_ENDPOINT, 8883);
    client.setKeepAlive(1200);
    client.setCallback(messageHandler);

    Serial.println("Connecting to AWS IOT");
    while (!client.connect(THINGNAME))
    {
        Serial.print(".");
        Serial.print("Error state: ");
        Serial.println(client.state());
        delay(500);
    }

    if (!client.connected())
    {
        Serial.println("AWS IoT Timeout!");
        return;
    }
    client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
    Serial.println("AWS IoT Connected!");
}

void publishMessage()
{
    JsonDocument doc;
    doc["humidity"] = h;
    doc["temperature"] = t;
    doc["thermal_sensation"] = th_sen;
    doc["Latitud"] = lat;
    doc["Longitud"] = lon;
    doc["satelite_num"] = num_sat;
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer);
    Serial.println("JSON a enviar:");
    Serial.println(jsonBuffer);
    bool ok = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
    Serial.println(ok ? "Publicado OK" : "Publish fallo");
}

void setup()
{
    Serial.begin(115200);
    connectAWS();
    dht.begin();
    GPS.begin(9600, SERIAL_8N1, 16, 17); //  rx2 tx2
    Serial.println("Esperando datos GPS...");
}

void loop()
{
    // ✅ Leer GPS sin timeout fijo — igual que el original que funciona
    unsigned long gpsStart = millis();
    while (millis() - gpsStart < 30000) // 30 segundo leyendo GPS
    {
        while (GPS.available())
        {
            gps.encode(GPS.read());
        }
        client.loop(); // ✅ mantiene MQTT vivo mientras lee GPS
    }

    if (gps.location.isUpdated())
    {
        lat = gps.location.lat();
        lon = gps.location.lng();
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

    h = dht.readHumidity();
    t = dht.readTemperature();
    th_sen = dht.computeHeatIndex(t, h, false);

    if (isnan(h) || isnan(t))
    {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C  Sensacion: "));
    Serial.println(th_sen);

    // ✅ Publicar ANTES de desconectar — igual que el original
    publishMessage();
    client.loop();
    delay(1000); // pequeña pausa para que el broker confirme recepción

    Serial.println("Entrando en Deep Sleep...");
    client.disconnect();
    WiFi.disconnect(true);
    delay(100);
    esp_sleep_enable_timer_wakeup(30ULL * 1000000); // 3600ULL * 1000000)  1 hora
    esp_deep_sleep_start();
}