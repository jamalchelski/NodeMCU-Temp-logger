#include <ESP8266WiFi.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>

#include <Wire.h>
#include <CTBot.h>
#include <DHT.h>

// Configurasi IO

char str_hum[16];
char str_temp[16];
char str_hum1[16];
char str_temp1[16];
const int buzzer = 13;
const int led = 12;

// Configure Network
#define WLAN_SSID "..." // ssid
#define WLAN_PASS "..." // password
WiFiClient client;

// Configure Adafruit IO
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "..." // Adafruit username
#define AIO_KEY "..."      // Adafruit Key

// configurasi Screen
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Configure DHT Sensor
#define DHTPIN 14
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// configurasi Telegram
String token = "..."; // Token Telegram
String msg = "Value";
int id = ...; // ID Telegram
CTBot myBot;

// Configurasi publish & subcribe
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp_cfu");
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");
void MQTT_connect();

void setup()
{
  Serial.begin(115200);
  delay(10);
  dht.begin();

  pinMode(buzzer, OUTPUT);
  pinMode(led, OUTPUT);

  Serial.println(F("Persiapan Koneksi .."));

  // Memulai koneksi ke jaringan
  Serial.println();
  Serial.print("Menyambungkan ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected");
  Serial.println("IP ADDRESS: ");
  Serial.println(WiFi.localIP());
  digitalWrite(led, HIGH);

  // SetUp MQTT Subription untuk on off feed
  mqtt.subscribe(&onoffbutton);

  // bot telegram up
  myBot.setTelegramToken(token);

  // Set Up display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop()
{
  MQTT_connect();
  delay(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t))
  {
    Serial.println("Gagal membaca sencor");
    return;
  }
  // clear display
  display.clearDisplay();

  // display temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature: ");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(t);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);
  display.setTextSize(2);
  display.print("C");

  // display humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity: ");
  display.setTextSize(2);
  display.setCursor(0, 45);
  display.print(h);
  display.print(" %");

  display.display();

  // publish temperature ke server
  Serial.print(F("\nMengirim nilai Suhu "));
  Serial.print(String(t).c_str());
  Serial.print("...");

  if (!temp.publish(String(t).c_str()))
  {
    Serial.print(F("Gagal"));
    delay(200);
  }
  else
  {
    Serial.println(F("OK !"));
    delay(5000);
  }

  if (t > 8)
  {
    //myBot.sendMessage(id, "Peringatan Suhu saat ini " + String(t) + " derajat");
    tone(buzzer, 2200);
    Serial.println(" peringatan Suhu diatas 8 Derajat");
  }
  else if (t < 2)
  {
    //myBot.sendMessage(id, "Peringatan Suhu saat ini " + String(t) + " derajat");
    tone(buzzer, 2200);
    Serial.println(" peringatan Suhu dibawah 2 Derajat");
  }
  else
  {
    Serial.println("Suhu Amana");
    noTone(buzzer);
  }
  delay(5000);
}
void MQTT_connect()
{
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); // wait 5 seconds
    retries--;
    if (retries == 0)
    {
      // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected!");
}
