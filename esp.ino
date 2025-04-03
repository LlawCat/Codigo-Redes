#include <WiFi.h>

// Configuración WiFi
const char* ssid = "Lanita";
const char* password = "lanita16ali";

// Servidor web en el ESP32
WiFiServer server(80);

// Variables de estado
bool releEstado = false;  // false = apagado, true = encendido

void setup() {
  Serial.begin(115200);
  delay(1000); // Delay inicial para estabilidad
  
  Serial.println("\nIniciando sistema de control de relé...");
  
  // Conexión WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Conectando a WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("\n¡Error al conectar al WiFi!");
    return;
  }
  
  Serial.println("\nWiFi conectado");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
  
  // Inicio del servidor
  server.begin();
  if(server) {
    Serial.println("Servidor HTTP iniciado correctamente");
    Serial.print("Accede desde: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("¡Error al iniciar el servidor!");
  }
}

void loop() {
  // Verificación de conexión WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("¡WiFi desconectado! Reconectando...");
    WiFi.reconnect();
    delay(2000);
    return;
  }
  
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("\nNuevo cliente conectado");
    String currentLine = "";
    String requestType = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        
        if (c == '\n') {
          // Procesar comando
          if (currentLine.startsWith("GET /H")) {
            requestType = "ENCENDER";
            releEstado = true;
          } 
          else if (currentLine.startsWith("GET /L")) {
            requestType = "APAGAR";
            releEstado = false;
          }
          else if (currentLine.startsWith("GET /T")) {
            requestType = "TOGGLE";
            releEstado = !releEstado;
          }
          
          if (currentLine.length() == 0) {
            sendHTMLResponse(client, requestType);
            break;
          }
          currentLine = "";
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    
    // Enviar comando al Pico W si hubo solicitud
    if (requestType != "") {
      String comando = releEstado ? "HIGH" : "LOW";
      enviarComandoAlPico(comando);
      Serial.print("Comando enviado al Pico W: ");
      Serial.println(comando);
    }
    
    client.stop();
    Serial.println("Cliente desconectado");
  }
  delay(10);
}

void sendHTMLResponse(WiFiClient &client, String action) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  
  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<title>Control Relé</title>");
  client.println("<style>");
  client.println("body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; }");
  client.println(".btn { padding: 15px 25px; font-size: 18px; margin: 10px; border: none; border-radius: 5px; cursor: pointer; }");
  client.println(".on { background-color: #4CAF50; color: white; }");  // Verde
  client.println(".off { background-color: #f44336; color: white; }"); // Rojo
  client.println(".toggle { background-color: #2196F3; color: white; }"); // Azul
  client.println(".status { font-size: 24px; margin: 20px; }");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>Control de Rele</h1>");
  
  // Mostrar estado actual
  client.print("<div class='status'>Estado actual: ");
  client.print(releEstado ? "<span style='color:green'>ENCENDIDO</span>" : "<span style='color:red'>APAGADO</span>");
  client.println("</div>");
  
  // Botones de control
  client.println("<div>");
  client.println("<a href=\"/H\"><button class=\"btn on\">ENCENDER</button></a>");
  client.println("<a href=\"/L\"><button class=\"btn off\">APAGAR</button></a>");
  client.println("<a href=\"/T\"><button class=\"btn toggle\">TOGGLE</button></a>");
  client.println("</div>");
  
  // Mostrar última acción
  if (action != "") {
    client.print("<p>Ultima accion: ");
    client.print(action);
    client.println("</p>");
  }
  
  client.println("</body></html>");
}

void enviarComandoAlPico(String comando) {
  WiFiClient picoClient;
  
  if (picoClient.connect("192.168.62.134", 80)) {
    picoClient.println(comando);
    delay(10); // Pequeña espera para asegurar transmisión
    
    // Opcional: leer respuesta del Pico W
    while(picoClient.available()) {
      Serial.write(picoClient.read());
    }
    
    picoClient.stop();
  } else {
    Serial.println("Error al conectar con el Pico W");
  }
}