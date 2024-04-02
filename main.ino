#include <Servo.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Ethernet.h>

#define SS_PIN 53
#define RST_PIN 9

int motorPin1 = 31;  // Broche pour le moteur 
int motorPin2 = 33;  // Broche pour le moteur 

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Création d'un objet MFRC522

Servo myServo;  // Création d'un objet de type Servo
//ip arduino 172.16.13.8

int ledPin = 13;  // Pin de la LED
int authorizedBadge1[5] = {197,15,136,42,104};  // Premier badge autorisé
int authorizedBadge2[5] = {6,174,123,161,114};  // Second badge autorisé 
unsigned long lastUsageTime = 0; // Variable pour stocker le temps de la dernière utilisation
unsigned long usageInterval = 5000; // Intervalle minimum entre deux utilisations (en millisecondes)

byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0x6A, 0x12}; // Adresse MAC de la carte Ethernet

int brocheCapteurTemp = 1; // Broche recevant le signal du capteur de température

EthernetServer server(80); // Initialise le serveur Ethernet, port 80 par défaut pour http
String lastUsageDateTime = "Aucun badge scanne"; // Stocke la date et l'heure du dernier scan de badge

void setup() {
  Serial.begin(9600);  // Initialisation de la communication série
  SPI.begin();  // Initialisation de la communication SPI
  mfrc522.PCD_Init();  // Initialisation du lecteur RFID
  pinMode(ledPin, OUTPUT);  // Définit le pin de la LED en sortie
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  digitalWrite(motorPin1, LOW);
  digitalWrite(motorPin2, LOW);
  // Désactivez le moteur au démarrage
  
  Ethernet.begin(mac); // Démarrer la connexion Ethernet et le serveur (version DHCP)
  server.begin();
  Serial.print("Le serveur est à l'adresse "); // Pour afficher l’adresse IP de la carte Arduino
  Serial.println(Ethernet.localIP());
}

void loop() {
  // Vérifie si un badge est détecté
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    boolean isAuthorized = false;
    // Vérifie si le badge détecté est autorisé
    for (int i = 0; i < 4; i++) {
      if (mfrc522.uid.uidByte[i] == authorizedBadge1[i] || mfrc522.uid.uidByte[i] == authorizedBadge2[i]) {
        isAuthorized = true;
      } else {
        isAuthorized = false;
        break;
      }
    }

    if (isAuthorized) {
      unsigned long currentTime = millis();
      if (currentTime - lastUsageTime >= usageInterval) {
        digitalWrite(motorPin1, HIGH);
        digitalWrite(motorPin2, LOW);
        delay(2000); // Temps de rotation du moteur
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, LOW);
        lastUsageTime = currentTime;

        // Mettre ici le code pour allumer le moteur
        digitalWrite(motorPin1, HIGH);
        delay(2000);
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, LOW);
      }
    } else {
      Serial.println("Accès refusé");
    }
    mfrc522.PICC_HaltA();  // Arrête la communication du PICC

    digitalWrite(ledPin, LOW);  // Éteint la LED
    
    // Construction de l'ID du badge scanné
    String scannedBadgeID = "";
    for (int i = 0; i < mfrc522.uid.size; i++) {
      scannedBadgeID += String(mfrc522.uid.uidByte[i], HEX);
    }

    // Affichage de l'ID du badge scanné dans la console série
    Serial.print("Badge scanne : ");
    Serial.println(scannedBadgeID);

    // Affichage de l'ID du badge scanné dans la page HTML
    lastUsageDateTime = "Badge scanne : " + scannedBadgeID;
  }

  // Lecture du capteur de température
  int Ntemp = analogRead(brocheCapteurTemp); // Lecture du capteur de température

  // Ecouter les clients entrants
  EthernetClient client = server.available();
  if (client) {
    Serial.println("New client");
    Serial.print("Adresse IP: ");
    Serial.println(Ethernet.localIP());
    // Une requête HTTP terminée par une ligne vide
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        // Envoyer un entête de réponse standard http
        client.println("HTTP/1.1 200 OK"); // C'est OK
        client.println();
        client.println("<!DOCTYPE html>");
        client.println("<html>");
        client.println("<head>");
        client.println("<meta content=\"utf-8\">");
        client.println("<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\">");
        client.println("<META HTTP-EQUIV=\"Content-Language\" CONTENT=\"fr-FX\">");
        client.println("<title>Connexion au Serveur Arduino</title>");
        client.println("<META HTTP-EQUIV=\"Refresh\" CONTENT=\"3\">");
        client.println("</head>");
        client.print("<body >");
        client.println();
        client.println("<p >************************************************* <br> </p>");
        client.println("<h2 >Pension pour chat!</h2>");
        client.println("<p >************************************************* <br> </p>");
        client.println("<br>"); // Saut de ligne
        // Afficher la date et l'heure du dernier scan de badge
        client.println("<p >");
        client.print("heure de nourrisage : ");
        client.print(lastUsageDateTime);
        client.println("</p>");
        client.println("</body>"); // Fin du body
        client.println("</html>"); // Fin de la page HTML
        break;
      }
    }
    delay(1); // Laisser au navigateur web le temps de recevoir les données
    client.stop(); // Refermer
  }
}
