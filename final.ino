#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <time.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"

//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "Eduspott"
#define WIFI_PASSWORD "saad1234"
#define API_KEY "AIzaSyC_hsb4IcTRWoO3OrSPYz5esXr1GfZmpoM"
#define DATABASE_URL "https://iote-d00e4-default-rtdb.europe-west1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;

//======================================== Millis variable to send/store data to firebase database.
unsigned long sendDataPrevMillis = 0;
const long sendDataIntervalMillis = 10000; //--> Sends/stores data to firebase database every 10 seconds.
//======================================== 


#define SS_PIN 5
#define RST_PIN 4
MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  SPI.begin();
  mfrc522.PCD_Init(); // Initialise le lecteur RFID
  lcd.init(); // Initialise l'écran LCD
  lcd.backlight(); // Allume le rétroéclairage de l'écran LCD

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Assign the api key (required).
  config.api_key = API_KEY;

  // Assign the RTDB URL (required).
  config.database_url = DATABASE_URL;

  // Sign up.
  Serial.println();
  Serial.println("---------------Sign up");
  Serial.print("Sign up new user... ");
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }
  Serial.println("---------------");
  
  // Assign the callback function for the long running token generation task.
  config.token_status_callback = tokenStatusCallback; //--> see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready()) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      processRFIDCard();
    }
  }
  delay(1000); // Ajout d'un petit délai pour stabiliser la boucle
}

void processRFIDCard() {
  String uid = getUID();
  String userName = "Unkown";
  String userPath = "/users/" + uid;
  String attendancePath = "/attendance/" + uid;
  fbdo.clear();

  // Obtenir la date actuelle pour créer ou vérifier un enregistrement unique par jour
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  char dateStamp[11];
  strftime(dateStamp, sizeof(dateStamp), "%Y-%m-%d", &timeinfo);

  //String attendancePath = attendancePath + "/" + String(dateStamp);

  if (!Firebase.RTDB.get(&fbdo, userPath)) {
    // Si l'utilisateur n'existe pas, créez-le
    userName = "User_" + uid;
    Serial.println("--------before set string-------");
    Firebase.RTDB.setString(&fbdo, userPath + "/name", userName);
    Serial.println("--------after set string-------");
    lcdDisplay("New: " + userName);
    
  }

  // Vérifier si l'utilisateur a déjà un enregistrement pour aujourd'hui
  if (!Firebase.RTDB.get(&fbdo, attendancePath)) {

    if (Firebase.RTDB.get(&fbdo, userPath + "/name")) {
      if (fbdo.dataType() == "string") {
        userName = fbdo.stringData();
        Serial.println("User Name: " + userName);
        Serial.println("--------after set username-------");
      }
    }

    // Première scan de la journée, enregistrer time in
    char timeStamp[20];
    strftime(timeStamp, sizeof(timeStamp), "%H:%M:%S", &timeinfo);
    Firebase.RTDB.setString(&fbdo, attendancePath + "/time_in", timeStamp);
    Firebase.RTDB.setString(&fbdo, attendancePath + "/name", userName);
    lcdDisplay("Arrival: " + String(timeStamp));
  } else {
    // Si time_in est présent mais pas time_out, enregistrer time_out
    if (!Firebase.RTDB.get(&fbdo, attendancePath + "/time_out")) {
      char timeStamp[20];
      strftime(timeStamp, sizeof(timeStamp), "%H:%M:%S", &timeinfo);
      Firebase.RTDB.setString(&fbdo, attendancePath + "/time_out", timeStamp);
      lcdDisplay("Departure: " + String(timeStamp));
    } else {
      // Refuser le troisième scan
      lcdDisplay("Already Logged In & Out");
    }
  }
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    char buff[3];
    sprintf(buff, "%02X", mfrc522.uid.uidByte[i]);
    uid += String(buff);
  }
  uid.toUpperCase();
  return uid;
}

void lcdDisplay(String message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  delay(3000); // Affiche le message pendant 3 secondes
}
