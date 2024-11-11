#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>
#include <memory>  // Pour std::unique_ptr

const char* WIFI_SSID = "*********";  // Remplacez par votre SSID
const char* WIFI_PASS = "*********"; // Remplacez par votre mot de passe

const int buttonPin = 12;  // Pin du bouton poussoir
const int ledPin = 4;      // Pin de la LED intégrée

WebServer server(80);
static auto hiRes = esp32cam::Resolution::find(800, 600);

bool buttonPressed = false;
bool imageAvailable = false;  // Indicateur pour savoir si une image a été capturée
std::unique_ptr<esp32cam::Frame> lastFrame = nullptr;  // Image capturée

void setup() {
  Serial.begin(115200);
  Serial.println();

  // Configurer le bouton et la LED
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);  // Assurez-vous que la LED est éteinte au démarrage

  // Configurer la caméra
  using namespace esp32cam;
  Config cfg;
  cfg.setPins(pins::AiThinker);
  cfg.setResolution(hiRes);
  cfg.setBufferCount(2);
  cfg.setJpeg(80);

  bool ok = Camera.begin(cfg);
  Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");

  // Configurer le Wi-Fi
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.print("http://");
  Serial.println(WiFi.localIP());

  // Configurer le serveur HTTP
  server.on("/capture", handleImageRequest);
  server.begin();
}

void loop() {
  server.handleClient();

  // Vérifier si le bouton est appuyé
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    buttonPressed = true;
    captureAndSendImage();
  } else if (digitalRead(buttonPin) == HIGH) {
    buttonPressed = false;
  }
}

void captureAndSendImage() {
  
  // Capturer une image
  lastFrame = esp32cam::capture();
  if (!lastFrame) {
    Serial.println("CAPTURE FAIL");
    digitalWrite(ledPin, LOW);  // Éteindre la LED
    return;
  }
  Serial.println("CAPTURE OK");
  imageAvailable = true;  // Marquer l'image comme disponible

  digitalWrite(ledPin, LOW);  // Éteindre la LED après capture
  delay(100);
}

void handleImageRequest() {
  if (!imageAvailable || !lastFrame) {
    server.send(200, "text/html", "<html><body><h2>Aucune image disponible</h2></body></html>");
    return;
  }

  // Envoyer l'image capturée au client
  server.sendHeader("Content-Type", "image/jpeg");
  server.sendHeader("Content-Length", String(lastFrame->size()));
  server.send(200);

  WiFiClient client = server.client();
  lastFrame->writeTo(client);

  Serial.println("Image envoyée au client");
  imageAvailable = false;  // Remettre l'indicateur à false après envoi
}
