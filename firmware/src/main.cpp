#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>

#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASS       "YOUR_WIFI_PASS"

#define MQTT_BROKER     "192.168.1.100"
#define MQTT_PORT       1883
#define MQTT_TOPIC      "sensors/esp32/bme280"
#define MQTT_CLIENT_ID  "esp32-sensor-01"

#define READ_INTERVAL   10000
#define BME280_ADDR     0x76

WiFiClient    wifiClient;
PubSubClient  mqtt(wifiClient);
Adafruit_BME280 bme;

unsigned long lastRead = 0;

void connectWiFi()
{
    Serial.printf("WiFi: connecting to %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
        Serial.printf("\nWiFi: connected, IP = %s\n", WiFi.localIP().toString().c_str());
    else
        Serial.println("\nWiFi: FAILED");
}

void connectMQTT()
{
    while (!mqtt.connected()) {
        Serial.printf("MQTT: connecting to %s\n", MQTT_BROKER);
        if (mqtt.connect(MQTT_CLIENT_ID)) {
            Serial.println("MQTT: connected");
        } else {
            Serial.printf("MQTT: failed, rc=%d\n", mqtt.state());
            delay(2000);
        }
    }
}

void readAndPublish()
{
    float temp     = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;

    Serial.printf("T=%.1f°C  H=%.1f%%  P=%.1fhPa\n", temp, humidity, pressure);

    StaticJsonDocument<200> doc;
    doc["temperature"] = round(temp * 10.0) / 10.0;
    doc["humidity"]    = round(humidity * 10.0) / 10.0;
    doc["pressure"]    = round(pressure * 10.0) / 10.0;
    doc["uptime_s"]    = millis() / 1000;

    char payload[200];
    serializeJson(doc, payload, sizeof(payload));

    if (mqtt.publish(MQTT_TOPIC, payload))
        Serial.println("MQTT: published");
    else
        Serial.println("MQTT: publish failed");
}

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n=== ESP32 Sensor Monitor ===");

    Wire.begin();
    if (!bme.begin(BME280_ADDR)) {
        Serial.println("ERROR: BME280 not found");
        while (1) delay(1000);
    }
    Serial.println("BME280: OK");

    connectWiFi();
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
        connectWiFi();

    if (!mqtt.connected())
        connectMQTT();

    mqtt.loop();

    unsigned long now = millis();
    if (now - lastRead >= READ_INTERVAL) {
        lastRead = now;
        readAndPublish();
    }
}
