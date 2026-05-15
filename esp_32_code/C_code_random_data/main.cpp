
#include "secrets3.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <DHT.h>
#include "esp_sleep.h"
#define DHTPIN 32     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
#include <Arduino.h>


#define AWS_IOT_PUBLISH_TOPIC "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
float level;
float h;
float t;
float th_sen;
double lon, lat;

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

    // Configure WiFiClientSecure to use the AWS IoT device credentials
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    // Connect to the MQTT broker on the AWS endpoint we defined earlier
    client.setServer(AWS_IOT_ENDPOINT, 8883);
    client.setKeepAlive(1200); // Perfecto
    // Create a message handler
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

    // Subscribe to a topic
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
    doc["nivel"] = level;
    char jsonBuffer[512];
    serializeJson(doc, jsonBuffer); // print to client

    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}

void setup()
{
    Serial.begin(115200);
    connectAWS();
    dht.begin();
    Serial.println("Arrancando...");
    randomSeed(micros());
}

void loop()
{
    level= random(0, 5100) / 100.0;
    //lat=11.229086;
    //lon=-74.203226;
    lat=11.233703;
    lon=-74.137920;
    h = dht.readHumidity();
    t = dht.readTemperature();
    th_sen = dht.computeHeatIndex(t, h, false);
    if (isnan(h) || isnan(t)) // Check if any reads failed and exit early (to try again).
    {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }
    Serial.print(F("level of wather: "));
    Serial.print(level);
    Serial.print(F("  Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));
    Serial.print(th_sen);
    Serial.println(F(" Sensacion termica"));
    publishMessage();
    client.loop();
    delay(2000);

    Serial.println("Entrando en Deep Sleep...");
    esp_sleep_enable_timer_wakeup(100 * 10000); // cambiar el tiempo maximo 1000000
    client.disconnect();
    WiFi.disconnect(true);
    esp_deep_sleep_start();
}
