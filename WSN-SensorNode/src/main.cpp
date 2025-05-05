#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char *ssid = "ESP32-Gateway";
const char *password = "12345678";

const char *gateway_ip = "192.168.4.1"; // IP fixo do gateway ESP32 (modo AP)

float lastValue = 25.0;
int sleepTime = 10; // segundos

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Ligando ao Wi-Fi...");
  }

  Serial.println("Conectado!");

  // Simular leitura de sensor
  float value = lastValue + random(-5, 5) * 0.1;
  float delta = abs(value - lastValue);

  if (delta < 0.5)
  {
    sleepTime = 30; // ambiente estável -> dorme mais
  }
  else
  {
    sleepTime = 10; // mudanças -> envia com mais frequência
  }

  lastValue = value;

  // Enviar dados
  HTTPClient http;
  String url = "http://" + String(gateway_ip) + "/sensor";
  WiFiClient client;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"value\":" + String(value, 2) + "}";
  int code = http.POST(json);
  Serial.println("HTTP Response: " + String(code));
  http.end();

  Serial.println("A dormir por " + String(sleepTime) + " segundos...");

  delay(100);
  ESP.deepSleep(sleepTime * 1000000UL);
}

void loop()
{
  // Nunca chega aqui por causa do deep sleep
}
