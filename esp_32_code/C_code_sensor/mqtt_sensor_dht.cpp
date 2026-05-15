
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <secrets.h>
#include <PubSubClient.h>
#define DHTPIN 32     // Pin donde conectas DATA
#define DHTTYPE DHT11 // Tipo de sensor
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup_wifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("conectando wifi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n WiFi conectado, IP" + WiFi.localIP().toString());
}

void reconnect()
{
    while (!client.connected())
    {
        String clientId = "esp32-" + String((uint32_t)ESP.getEfuseMac(), HEX);
        Serial.print("conectando a MQTT como");
        Serial.println(clientId);
        if (client.connect(clientId.c_str()))
        {
            Serial.println("Conectado a broker Mqtt");
        }
        else
        {
            Serial.print("fallo , rc=  ");
            Serial.print(client.state());
            Serial.println("reintentando en 2s");
            delay(2000);
        }
    }
}

void setup()
{
    Serial.begin(115200);
    setup_wifi();
    dht.begin();
    Serial.println(F("DHTxx test!"));
    client.setServer(mqtt_server, mqtt_port);
}

void loop()
{
    if (!client.connected())
        reconnect();
    client.loop();
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float hic = dht.computeHeatIndex(t, h, false);
    if (!isnan(h) || !isnan(t))
    {
        char bufT[16];
        dtostrf(t, 1, 2, bufT);
        char bufH[16];
        dtostrf(h, 1, 2, bufH);
        char bufFe[16];
        dtostrf(hic, 1, 2, bufFe);

        if (client.publish(topic_temp, bufT, true))
        {
            Serial.print("Publicado temperatura ºC");
            Serial.print(topic_temp);
            Serial.print(" : ");
            Serial.println(bufT);
        }

        if (client.publish(topic_hum, bufH, true))
        {
            Serial.print("Publicado humedad %");
            Serial.print(topic_hum);
            Serial.print(" : ");
            Serial.println(bufH);
        }

        if (client.publish(topic_sensacion_temp, bufFe, true))
        {
            Serial.print("Publicado sensacion  ºC");
            Serial.print(topic_sensacion_temp);
            Serial.print(" : ");
            Serial.println(bufFe);
        }
    }
}