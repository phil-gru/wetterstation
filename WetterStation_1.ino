#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SERVER_IP "homepi:80"
#define SEALEVELPRESSURE_HPA (1013.25)
#define uS_TO_S_FACTOR 1000000

ADC_MODE(ADC_VCC);
WiFiClient wifiClient;
Adafruit_BME280 bme;

ESP8266WebServer server(80);

const char* ssid = "wlan";
const char* password = "passwort";

//int sleeptime = 1200;
int sleeptime = 1800; // 1800 s = 30 min

String sensorname = "BME280";
String sensorloc = "Ort";
//String apikey = "abc";

//const char* dns_name = "esp8266";

float temperature, humidity, pressure, altitude;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP Gestartet");
  Serial.println("Waking up...");

  bme.begin(0x76);

  WiFi.begin(ssid, password);

  Serial.print("Verbindung wird hergestellt ...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Verbunden! IP-Adresse: ");
  Serial.println(WiFi.localIP());

  /*if (MDNS.begin(dns_name)) {
    Serial.println("DNS gestartet, erreichbar unter: ");
    Serial.println("http://" + String(dns_name) + ".local/");
  }*/

  server.onNotFound([]() {
    server.send(404, "text/plain", "Der Link wurde nicht gefunden!");
  });

  server.on("/", []() {
    server.send(200, "text/plain", "ESP-Startseite!");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  // wait for WiFi connection
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    // Your Domain name with URL path or IP address with path
    http.begin(client, "http://" SERVER_IP "/wetterstation/post_esp_data.php");

    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Prepare your HTTP POST request data
    String httpRequestData = "sensorname=" + sensorname
                             + "&location=" + sensorloc + "&value1=" + String(bme.readTemperature())
                             + "&value2=" + String(bme.readHumidity())
                             + "&value3=" + String(bme.readPressure() / 100.0F)
                             + "&value4=" + String(WiFi.RSSI())
                             //+ "&value5=" + String(((float)ESP.getVcc() / 1024.0f), 3)
                             + "&value5=" + String(String(ESP.getVcc() / 1024.0, 3))
                             + "";
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    //httppost wird geschickt
    int httpResponseCode = http.POST(httpRequestData);
    //Sobald der Response Code >0 ist, müsste alles geklappt haben
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    //Ansosnten Ausgabe des Fehlercodes
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    //HTTP Verbindung beenden
    http.end();
  }

  
  //delay(1200000);
  getBmeData();
  startDeepSleep();
  //delay(60000);
}

void getBmeData() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print("Temperatur: ");
  Serial.println(temperature);
  Serial.print("Luftfeuchtigkeit: ");
  Serial.println(humidity);
  Serial.print("Druck: ");
  Serial.println(pressure);
  Serial.print("Höhe: ");
  Serial.println(altitude);
  Serial.print("Vcc: ");
  Serial.println(String(ESP.getVcc() / 1024.0, 3));
  delay(500);
}

void startDeepSleep(){
  Serial.println("Going to deep sleep...");
  ESP.deepSleep(sleeptime * uS_TO_S_FACTOR);
  yield();
}
      
