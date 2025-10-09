#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi credentials
#define WIFI_SSID "AUTANMAMA"
#define WIFI_PASSWORD "namamana"

// Firebase credentials
#define API_KEY "AIzaSyAU6N4dgVls7PvgV72BUePa08VKyfNE8xY"
#define DATABASE_URL "https://smart-meter-f7b07-default-rtdb.firebaseio.com/"
#define USER_EMAIL "tariqalmansur2019@gmail.com"
#define USER_PASSWORD "Almansur"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;

// PZEM setup (D2 = GPIO4, D1 = GPIO5)
SoftwareSerial pzemSerial(D5, D4);  // RX, TX
PZEM004Tv30 pzem(pzemSerial);

// Relay
#define RELAY_PIN D6

// Credit tracking
float credit = 0;
String relayOverride = "auto";  // Default behavior

// PIN-Credit mapping
struct CreditPin {
  const char* pin;
  float value;
};

CreditPin pins[] = {
  {"12345", 0.05}, {"23456", 0.10}, {"34567", 0.15}, {"45678", 0.20}
};

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Default OFF

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10); 
  display.println("OLED Active");
  display.display();
  Serial.println("OLED Initialized Successfully");
  delay(2000);

  // WiFi connect
  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("Connecting WiFi...");
  display.display();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection failed!");
    display.clearDisplay();
    display.println("WiFi Failed");
    display.display();
    while (true);
  }

  display.clearDisplay();
  display.println("WiFi Connected");
  display.display();
  delay(1000);

  // Firebase init
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);
  Firebase.setDoubleDigits(5);

  Firebase.setString(fbdo, "/nodes/nodeB/vendPin", "0");
  Firebase.setString(fbdo, "/nodes/nodeB/relayOverride", "auto");
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 8000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Read from PZEM
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();

    Serial.println("------ PZEM Readings ------");
    if (!isnan(voltage)) {
      Serial.print("Voltage: "); Serial.println(voltage);
      Firebase.setFloat(fbdo, "/nodes/nodeB/voltage", voltage);
    } else Serial.println("Voltage: No Data");

    if (!isnan(current)) {
      Serial.print("Current: "); Serial.println(current);
      Firebase.setFloat(fbdo, "/nodes/nodeB/current", current);
    } else Serial.println("Current: No Data");

    if (!isnan(power)) {
      Serial.print("Power: "); Serial.println(power);
      Firebase.setFloat(fbdo, "/nodes/nodeB/power", power);
    } else Serial.println("Power: No Data");

    if (!isnan(energy)) {
      Serial.print("Energy: "); Serial.println(energy);
      Firebase.setFloat(fbdo, "/nodes/nodeB/energy", energy);
    } else Serial.println("Energy: No Data");
    Serial.println("---------------------------");

    // 🔁 Check vend pin
    if (Firebase.getString(fbdo, "/nodes/nodeB/vendPin")) {
      String pin = fbdo.stringData();
      if (pin != "0" && pin.length() > 0) {
        Serial.print("Received vend pin: ");
        Serial.println(pin);

        bool validPin = false;
        for (CreditPin p : pins) {
          if (pin == p.pin) {
            credit += p.value;
            validPin = true;
            break;
          }
        }
        if (!validPin) Serial.println("Invalid PIN entered!");

        Firebase.setString(fbdo, "/nodes/nodeB/vendPin", "0");
      }
    }

    // 🔄 Credit deduction based on power
    if (!isnan(power) && power > 1.0) {
      float energyUsed = (power / 1000.0) * (8.0 / 3600.0);  // kWh over 8s
      credit -= energyUsed;
      if (credit < 0) credit = 0;
    }

    Firebase.setFloat(fbdo, "/nodes/nodeB/credit", credit);
    Serial.printf("Remaining Credit: %.4f kWh\n", credit);

    // 🚦 Firebase Relay Override Logic
    if (Firebase.getString(fbdo, "/nodes/nodeB/relayOverride")) {
      relayOverride = fbdo.stringData();
      Serial.print("Relay Override Status: ");
      Serial.println(relayOverride);
    }

    if (relayOverride == "off") {
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Relay forced OFF from app");
    } else if (relayOverride == "on") {
      digitalWrite(RELAY_PIN, LOW);
      Serial.println("Relay forced ON from app");
    
      // Default behavior: based on credit
      if (credit <= 0) {
        digitalWrite(RELAY_PIN, HIGH);
        Serial.println("Relay OFF - No Credit");
      } else {
        digitalWrite(RELAY_PIN, LOW);
        Serial.println("Relay ON - Power Supplied");
      }
    }

    // 🖥 OLED Update
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE);
    display.printf("P: %.1fW\n", power);
    display.println();
    display.printf("Cr: %.4f\n", credit);
    display.display();
  }
}
