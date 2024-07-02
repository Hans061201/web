#include <WiFi.h>
#include <WiFiMulti.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h> // Asegurate de tener esta biblioteca instalada
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
const char* registrarAsistenciaPath = "/urequestESP32/registrar_asistencia.php";
const char* listarAsistenciaPath = "/urequestESP32/listar_asistencia.php"; // Ruta del PHP para listar asistencia

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

    while (cliente.connected() && tiempoActual - tiempoAnterior <= tiempoCancelación) {
      if (cliente.available()) {
        tiempoActual = millis();
        char letra = cliente.read();
        if (letra == '\n') {
          if (lineaActual.length() == 0) {
            if (peticion.startsWith("GET /marcar")) {
              registrarAsistencia(cliente);
            } else if (peticion.startsWith("GET /listar")) {
              mostrarListaAsistencia(cliente);
            } else if (peticion.startsWith("GET /")) {
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

void mostrarListaAsistencia(WiFiClient& cliente) {
  WiFiClient client;
  if (client.connect(serverName, 80)) {
    // Conectar con el servidor a traves del puerto 80   
    client.print(String("GET ") + listarAsistenciaPath + " HTTP/1.1\r\n" + "Host: " + serverName + "\r\n" + "Connection: close\r\n\r\n");
    delay(10);
    String response = "";
    // El response es la información Json serializada
    bool jsonStarted = false;
    while (client.connected()) {
      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") { // El fin de los encabezados HTTP es una línea en blanco seguida por '\r'
          jsonStarted = true;
          continue;
        }
        if (jsonStarted) {
          response += line + '\n';
        }
      }
    }

    Serial.println("Respuesta del servidor: ");
    Serial.println(response); // Para depuración

    String html = "<html><body><h1>Lista de Asistencia</h1><table border='1'><tr><th>ID</th><th>Nombre</th><th>Apellido</th><th>Hora</th></tr>";
    
    // Convertir el JSON a una tabla HTML
    // Creamos un documento json y entre parentésis va el tamaño
    DynamicJsonDocument doc(4096);
    DeserializationError error = deserializeJson(doc, response);
    if (!error) {
      JsonArray array = doc.as<JsonArray>();
      for (JsonObject obj : array) {
        html += "<tr>";
        html += "<td>" + String(obj["id"].as<const char*>()) + "</td>";
        html += "<td>" + String(obj["nombre"].as<const char*>()) + "</td>";
        html += "<td>" + String(obj["apellido"].as<const char*>()) + "</td>";
        html += "<td>" + String(obj["hora"].as<const char*>()) + "</td>";
        html += "</tr>";
      }
    } else {
      html += "<tr><td colspan='4'>Error al procesar los datos: ";
      html += error.c_str();
      html += "</td></tr>";
    }
    
    html += "</table></body></html>";
    cliente.print("HTTP/1.1 200 OK\n");
    cliente.print("Content-Type: text/html\n\n");
    cliente.print(html);
  } else {
    cliente.print("HTTP/1.1 500 Internal Server Error\n");
    cliente.print("Content-Type: text/plain\n\n");
    cliente.print("Fallo al conectar");
  }
}

void responderFormulario(WiFiClient& cliente) {
  String html = Pagina;
  cliente.print("HTTP/1.1 200 OK\n");
  cliente.print("Content-Type: text/html\n\n");
  cliente.print(html);
}

String getMacAddressFromIP(IPAddress ip) {
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
    // Enviamos la solicitud mediante el metodo cliente.print
    delay(10);
    // Leemos todas las lineas que nos responde el servidor y se imprime por pantalla
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
