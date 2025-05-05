#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <esp_wifi.h>
#include <esp_netif.h>

#define LED 2

const char *ssid = "ESP32-Gateway";
const char *password = "12345678";

IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);
String pcServer = ""; // IP do servidor Flask (PC)
bool serverFound = false;
bool findPCServer()
{
  wifi_sta_list_t staList;
  tcpip_adapter_sta_list_t ipList;
  WiFiClient client;
  HTTPClient http;

  if (esp_wifi_ap_get_sta_list(&staList) != ESP_OK ||
      tcpip_adapter_get_sta_list(&staList, &ipList) != ESP_OK)
  {
    Serial.println("Erro ao obter lista de clientes.");
    return false;
  }

  Serial.printf("Encontrados %d clientes conectados\n", ipList.num);

  for (int i = 0; i < ipList.num; i++)
  {
    tcpip_adapter_sta_info_t info = ipList.sta[i];
    String ipStr = IPAddress(info.ip.addr).toString();
    String url = "http://" + ipStr + ":5000/data";

    Serial.println("A testar: " + url);

    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST("{\"ping\":true}");
    http.end();

    if (code == 200)
    {
      pcServer = url;
      Serial.println("Servidor Flask encontrado em: " + pcServer);
      return true;
    }
  }

  Serial.println("Nenhum servidor Flask encontrado.");
  return false;
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password);
  Serial.println("AP iniciado");

  delay(3000); // Esperar que o PC se ligue ao AP

  while (serverFound == false)
  {
    delay(3000);
    bool result = false;
    result = findPCServer();
    if (result == true)
    {
      serverFound = true;
      break;
    }
  }
  // Tenta descobrir o servidor

  server.on("/sensor", HTTP_POST, []()
            {
    digitalWrite(LED, HIGH);
    String body = server.arg("plain");
    Serial.println("Recebido do nó: " + body);

    if (pcServer != "") {
      HTTPClient http;
      WiFiClient client;
      http.begin(client, pcServer);
      http.addHeader("Content-Type", "application/json");
      int code = http.POST(body);
      Serial.println("Reencaminhado para PC: HTTP " + String(code));
      http.end();
    } else {
      Serial.println("Servidor PC não encontrado. Ignorar envio.");
    }

    digitalWrite(LED, LOW);
    server.send(200, "text/plain", "OK"); });

  server.begin();
}

void loop()
{
  server.handleClient();
}
