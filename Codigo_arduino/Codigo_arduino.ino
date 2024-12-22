#include <WiFi.h>
#include <WebServer.h>
#include <esp32cam.h>

// Datos de la red WiFi
const char* ssid = "FAMILIA ROJAS QUIJANO";
const char* password = "7176960106";

// Configuración del pin del LED azul y el buzzer
const int ledPin = 13;  // GPIO 13 (LED azul)
const int buzzer = 12;  // GPIO 12
const int ledverde = 14; 
const int ledrojo = 15;
const int botonfuncion = 2;

// Servidor Web
WebServer server(80);

// Configuración de resoluciones de la cámara
static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);

// Función para capturar y enviar imagen
void serveJpg() {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "text/plain", "Error: No se pudo capturar la imagen");
    return;
  }

  // Enciende el LED azul para indicar éxito en la captura
  digitalWrite(ledPin, HIGH);

  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);

  // Apaga el LED azul después de un breve retraso
  delay(100);  // Mantén el LED encendido por 100 ms
  digitalWrite(ledPin, LOW);
}

// Rutas para imágenes en diferentes resoluciones
void handleJpgLo() {
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void handleJpgMid() {
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}

void handleJpgHi() {
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}

// Configuración inicial
void setup() {
  // Inicializar el puerto serial
  Serial.begin(115200);

  // Conectar a la red WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Conectando al WiFi...");
  }
  Serial.println("Conectado a WiFi");
  Serial.println(WiFi.localIP());

  // Configurar pines de LED y buzzer
  //pinMode(botonfuncion, INPUT);
  pinMode(botonfuncion, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ledverde,OUTPUT);
  pinMode(ledrojo,OUTPUT);
  // Configuración de la cámara
  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(hiRes);  // Resolución inicial (alta)
  cfg.setBufferCount(2);
  cfg.setJpeg(80);  // Calidad JPEG
  bool ok = Camera.begin(cfg);
  Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  
  server.on("/leido/on", HTTP_GET, [](){
    digitalWrite(ledverde, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(500);

    digitalWrite(ledverde, LOW);
    digitalWrite(buzzer,LOW);
    delay(500);

    digitalWrite(ledverde, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(500);
    digitalWrite(ledverde, LOW);
    digitalWrite(buzzer,LOW);
    server.send(200, "text/plain", "LED Encendido");
    
  });
  server.on("/error/on", HTTP_GET, [](){
    digitalWrite(ledrojo, HIGH);
    digitalWrite(buzzer, HIGH);
    delay(1000);
       
    digitalWrite(ledrojo, LOW);
    digitalWrite(buzzer,LOW);
    server.send(200, "text/plain", "LED Encendido");
    
  });

  server.on("/botonfuncion/on", HTTP_GET, []() {
    if (digitalRead(botonfuncion) == LOW) {
        server.send(200, "text/plain", "1");
    } else {
        server.send(200, "text/plain", "0");
    }
  });

  // Definir rutas para capturas de cámara
  server.on("/cam-lo.jpg", handleJpgLo);
  server.on("/cam-mid.jpg", handleJpgMid);
  server.on("/cam-hi.jpg", handleJpgHi);

  // Iniciar el servidor
  server.begin();
}

// Bucle principal
void loop() {
  server.handleClient();  // Maneja las solicitudes HTTP
}
