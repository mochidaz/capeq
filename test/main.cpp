#include <Arduino.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#define DHT_SENSOR_PIN 17 // ESP32 pin GIOP21 connected to DHT11 sensor
#define DHT_SENSOR_TYPE DHT11

DHT dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

const char *ssid = "Dyware 3";
const char *password = "p@ssw0rd";
const char *mqtt_server = "broker.emqx.io";
uint16_t mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
float temperature = 0;

void callback(char *topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/temperatur, you check if the message is either "on" or "off".
  // Changes the temperatur state according to the message
  if (String(topic) == "esp32wian/output")
  {
    Serial.print("Got publish: ");
    // if (messageTemp == "on")
    // {
    Serial.println(messageTemp);
    // }
    // else if (messageTemp == "off")
    // {
    //   Serial.println("off");
    // }
  }
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp32"))
    {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/temperatur");
    }
    else
    {
      Serial.print("fasiled, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  dht_sensor.begin();
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{

  float humi = dht_sensor.readHumidity();
  float tempC = dht_sensor.readTemperature();
  if (isnan(tempC) || isnan(humi))
  {
    Serial.println("Failed to read from DHT sensor!");
  }
  else
  {
    Serial.print("Humidity: ");
    Serial.print(humi);
    Serial.print(",");

    Serial.print("Temperature: ");
    Serial.println(tempC);
  }
  delay(2000);
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;

    // Temperature in Celsius
    temperature = random(0, 2) / 100;
    // Uncomment the next line to set temperature in Fahrenheit
    // (and comment the previous temperature line)
    // temperature = 1.8 * bme.readTemperature() + 32; // Temperature in Fahrenheit

    // Convert the value to a char array
    char tempString[8];
    dtostrf(tempC, 1, 2, tempString);
    Serial.print("Temperature: ");
    Serial.println(tempString);
    client.publish("esp32wian/temperature", tempString);
  }
}