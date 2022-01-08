#include <ESP8266Wifi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "CTBot.h"

// insert liblary Sensor
#include "DHT.h"
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

char str_hum[16];
char str_temp[16];
char str_hum1[16];
char str_temp1[16];
const int buzzer = 5;
const int led = 0;

#define WLAN_SSID ".." // ssid
#define WLAN_PASS ".." // ssid password

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME ".." // username adafruit account
#define AIO_KEY ".."      // password adafruit account

String token = ".."; // Telegram Bot token

String msg = "value";
int id = "..."; // ID telegram

CTBot mybot;

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp_cfu");
Adafruit_MQTT_Publish chiller1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/max_cfu");
Adafruit_MQTT_Subcribe onoffbutton = Adafruit_MQTT_Subcribe(&mqtt, AIO_USERNAME "/feeds/onoff");

void MQTT_connect();
void Buzzer(val)
{
  tone(buzzer, val);
}

void setup()
{
  Serial.begin(115200);
  delay(10);
  dht.begin();

  punMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);

  Serial.println("Persiapan Loading ... !");
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("Wifi Connected");
  Serial.println("IP address: ");
  Serial.print(WiFi.localIP());

  mqtt.subcribe(&onoffbutton);
  myBot.setTelegramToken(token);
}

void loop()
{
  MQTT_connect();
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readtemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println("failed to read from DHT Sensor");
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatindex(t, h, false);

  Serial.print("Temperature : ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print("Temperature Chiller 1 in Celcius: ");
  Serial.println(String(t).c_str());

  Serial.print(F("\nSending temperature value : "));
  Serial.print(String(t).c_str());
  Serial.print("...");

  if (!temp.publish(String(t).c_str()))
  {
    Serial.println(F("Failed"));
    delay(200);
  }
  else
  {
    Serial.println(F("OK !"));
    delay(1000);
  }

  if (t > 8)
  {
    myBot.sendMessage(id, "Peringatan Suhu di atas 8 derajat");
    Buzzer(2200);
    Serial.println("Suhu di atas ketentuan");
  }
  else if (t < 2)
  {
    myBot.sendMessage(id, "Peringatan suhu di bawah 2 derajat");
    Buzzer(2200);
    Serial.println("Suhu di bawah ketentuan");
  }
  else
  {
    Serial.println("Suhu Aman");
    Buzzer(0);
  }
  delay(5000);
}

void MQTT_connect()
{
  int8_t ret;
  if (mqtt.connected())
  {
    return;
  }
  Serial.print("Conecting to MQTT ..");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("retrying MQTT connection in 5 secconds ..");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0)
    {
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected !");
}