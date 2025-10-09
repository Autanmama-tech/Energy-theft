#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

// WiFi credentials
#define WIFI_SSID "AUTANMAMA"
#define WIFI_PASSWORD "namamana"

// Firebase credentials
#define API_KEY "AIzaSyAU6N4dgVls7PvgV72BUePa08VKyfNE8xY"
#define DATABASE_URL "https://smart-meter-f7b07-default-rtdb.firebaseio.com/"
#define USER_EMAIL "tariqalmansur2019@gmail.com"
#define USER_PASSWORD "Almansur"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;

// PZEM setup for Node A
SoftwareSerial pzemSerial(14, 12); // RX = GPIO14, TX = GPIO12 (formerly D5, D6)
PZEM004Tv30 pzem(pzemSerial);


void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);
  Firebase.setDoubleDigits(5);
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();

    Serial.println("------ NODE A (Main Meter) ------");

    // Print and push local readings
    if (!isnan(voltage)) {
      Serial.print("Voltage: "); Serial.println(voltage);
      Firebase.setFloat(fbdo, "/nodes/nodeA/voltage", voltage);
    } else {
      Serial.println("Voltage: No Data");
    }

    if (!isnan(current)) {
      Serial.print("Current: "); Serial.println(current);
      Firebase.setFloat(fbdo, "/nodes/nodeA/current", current);
    } else {
      Serial.println("Current: No Data");
    }

    if (!isnan(power)) {
      Serial.print("Power: "); Serial.println(power);
      Firebase.setFloat(fbdo, "/nodes/nodeA/power", power);
    } else {
      Serial.println("Power: No Data");
    }

    if (!isnan(energy)) {
      Serial.print("Energy: "); Serial.println(energy);
      Firebase.setFloat(fbdo, "/nodes/nodeA/energy", energy);
    } else {
      Serial.println("Energy: No Data");
    }

    // Compare with Node B
    float nodeBPower = Firebase.getFloat(fbdo, "/nodes/nodeB/power") ? fbdo.floatData() : 0.0;
    float nodeBEnergy = Firebase.getFloat(fbdo, "/nodes/nodeB/energy") ? fbdo.floatData() : 0.0;

    Serial.print("Node B Power: "); Serial.println(nodeBPower);
    Serial.print("Node B Energy: "); Serial.println(nodeBEnergy);

    // Simple bypass detection logic
    if (power > 5.0 && nodeBPower < 1.0) {
      Serial.println("⚠️  Possible Bypass Detected!");
      Firebase.setString(fbdo, "/alerts/bypass", "Bypass Detected");
    } else {
      Serial.println("✅ No bypass detected.");
      Firebase.setString(fbdo, "/alerts/bypass", "No bypass");
    }

    Serial.println("---------------------------------");
  }
}
