#define EEPROM_SIZE 64
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include "data.h"

WiFiMulti wifiMulti;

const uint32_t TiempoEsperaWifi = 5000;
WiFiServer servidor(80);
boolean estado = false;
const uint32_t tiempoEsperaWifi = 5000;
unsigned long tiempoActual = 0;
unsigned long tiempoAnterior = 0;
const long tiempoCancelación = 5000;
const char* serverName = "192.168.18.2"; 
const char* guardarMacPath = "/urequestESP32/guardar_mac.php";
const char* registrarAsistenciaPath = "/urequestESP32/registrar_asistencia.php";

void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando multi Wifi");

  wifiMulti.addAP(usu1, contra1);

  WiFi.mode(WIFI_STA);
  Serial.print("Conectando a Wifi ..");
  while (wifiMulti.run(TiempoEsperaWifi) != WL_CONNECTED) {
    Serial.print(".");
  }
  Serial.println(".. Conectado");
  Serial.print("SSID:");
  Serial.print(WiFi.SSID());
  Serial.print(" ID:");
  Serial.println(WiFi.localIP());
  if (!MDNS.begin("server1")) {
    Serial.println("Error configurando mDNS");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS configurado");
  servidor.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  WiFiClient cliente = servidor.available();

  if (cliente) {
    Serial.println("Nuevo cliente");
    tiempoActual = millis();
    tiempoAnterior = tiempoActual;
    String lineaActual = "";
    String peticion = "";

    // Obtener la MAC address del cliente
    IPAddress clienteIP = cliente.remoteIP();
    String macAddress = getMacAddressFromIP(clienteIP);
    enviarMac(macAddress, guardarMacPath);

    while (cliente.connected() && tiempoActual - tiempoAnterior <= tiempoCancelación) {
      if (cliente.available()) {
        tiempoActual = millis();
        char letra = cliente.read();
        if (letra == '\n') {
          if (lineaActual.length() == 0) {
            if (peticion.startsWith("GET /marcar")) {
              registrarAsistencia(cliente);
            } else {
              responderFormulario(cliente);
            }
            break;
          } else {
            peticion += lineaActual + "\n";
            lineaActual = "";
          }
        } else if (letra != '\r') {
          lineaActual += letra;
        }
      }
    }

    cliente.stop();
    Serial.println("Cliente Desconectado");
    Serial.println();
  }
}

void registrarAsistencia(WiFiClient& cliente) {
  IPAddress clienteIP = cliente.remoteIP();
  String macAddress = getMacAddressFromIP(clienteIP);

  // Enviar la MAC al servidor PHP para registrar asistencia
  bool enviado = enviarMac(macAddress, registrarAsistenciaPath);

  // Responder al cliente
  cliente.print("HTTP/1.1 200 OK\n");
  cliente.print("Content-Type: text/html\n\n");
  cliente.print("<html><body><h1>Asistencia registrada</h1></body></html>");
}

void responderFormulario(WiFiClient& cliente) {
  cliente.print("HTTP/1.1 200 OK\n");
  cliente.print("Content-Type: text/html\n\n");
  cliente.print(Pagina);
}

String getMacAddressFromIP(IPAddress ip) {
  // retornamos la IP como string
  return ip.toString();
}

bool enviarMac(String mac, const char* path) {
  WiFiClient cliente;
  Serial.print("Conectando a servidor: ");
  Serial.println(serverName);
  if (cliente.connect(serverName, 80)) {
    Serial.println("Conexión exitosa");
    Serial.println("Enviando MAC: " + mac);
    cliente.print(String("GET ") + path + "?mac=" + mac + " HTTP/1.1\r\n" + "Host: " + serverName + "\r\n" + "Connection: close\r\n\r\n");
    delay(10);
    while (cliente.available()) {
      String line = cliente.readStringUntil('\r');
      Serial.print(line);
    }
    Serial.println("Fin de la respuesta del servidor");
    return true;
  } else {
    Serial.println("Fallo al conectar");
    return false;
  }
}
