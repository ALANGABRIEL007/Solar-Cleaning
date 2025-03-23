//By Alan Gabriel, alangabriel071@gmail.com

#include <esp_now.h>
#include <WiFi.h>
#include "max6675.h"

// MAX6675 sensor configuration
int ktcSO = 21;
int ktcCS = 23;
int ktcCLK = 22;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

int cycle = 0;
int threshold = 35; // Default threshold value
int threshintv = 0;
unsigned long lastBroadcastTime = 0; // Stores the last time a broadcast was sent
unsigned long delayDuration = 0; // Duration to wait before the next broadcast
unsigned long lastTempRead = 0;
float temperature = 0;

struct Data {
  int ID;
  int cyc;
  int thres;
  int intv;
};
Data received;

// ESP-NOW addresses
uint8_t panelbots[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
uint8_t pump[] = {0x08, 0xF9, 0xE0, 0x6C, 0x4B, 0x07};  

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("ESP-NOW Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP-NOW ESP Initialized");
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  pinMode(ktcCS, OUTPUT);
  digitalWrite(ktcCS, HIGH);

  delay(500); // Give the MAX6675 time to stabilize

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(onDataSent);

  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, panelbots, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add broadcast peer");
  }

  memcpy(peerInfo.peer_addr, pump, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add unicast peer");
  }
}

void readTemperature() {
  if (millis() - lastTempRead > 500) {  // Ensure reading occurs every 500ms
    digitalWrite(ktcCS, LOW);
    delay(10);
    temperature = ktc.readCelsius();
    digitalWrite(ktcCS, HIGH);
    lastTempRead = millis();
    //Serial.println(temperature);
  }
}

void sendBroadcastAndUnicast(int cycleValue) {
  uint8_t broadcastMessage[1] = { (uint8_t)cycleValue };
  esp_now_send(panelbots, broadcastMessage, sizeof(broadcastMessage));
  Serial.println("Broadcast signal sent to panelbots.");

  uint8_t unicastMessage[1] = { (uint8_t)cycleValue };
  esp_now_send(pump, unicastMessage, sizeof(unicastMessage));
  Serial.println("Unicast signal sent to water pump.");
}

void loop() {
  readTemperature();

  if (Serial2.available()) {
    Serial2.readBytes((uint8_t*)&received, sizeof(received));

    if (received.ID == 0) {
      if (temperature < 0) {
        Serial.println("Failed to read from MAX6675 sensor");
      } else {
        Serial2.println(temperature);  
        Serial.print("Temperature sent to Blynk ESP: "); 
        Serial.println(temperature);
      }
    } else if (received.ID == 1) {
      cycle = received.cyc;
      threshold = received.thres;
      Serial.print("Cycle received: ");
      Serial.println(cycle);
      Serial.print("Threshold updated to: ");
      Serial.println(threshold);

      // Send immediate broadcast and unicast message
      sendBroadcastAndUnicast(cycle);
    } else if (received.ID == 2) {
      cycle = received.cyc;
      Serial.print("Cycle received: ");
      Serial.println(cycle);
    } else if (received.ID == 3) {
      threshold = received.thres;
      Serial.print("Threshold updated to: ");
      Serial.println(threshold);
    } else if (received.ID == 4) {
      threshintv = received.intv;
      Serial.print("Threshold-Interval Updated and Saved: ");
      Serial.println(threshintv);
    }
  }

  // Periodically check the temperature against the threshold
  readTemperature();
  if (temperature >= 0) {
    if (temperature > threshold && threshold != 0) {
      unsigned long currentMillis = millis();
      if (currentMillis - lastBroadcastTime >= delayDuration) {
        Serial.println("Temperature exceeded threshold!");
        sendBroadcastAndUnicast(cycle);
        lastBroadcastTime = currentMillis;

        // Set delay duration based on threshintv
        switch (threshintv) {
          case 0: delayDuration = 30L * 60 * 1000; break; // 30 minutes
          case 1: delayDuration = 60L * 60 * 1000; break; // 1 hour
          case 2: delayDuration = 90L * 60 * 1000; break; // 1.5 hours
          case 3: delayDuration = 120L * 60 * 1000; break; // 2 hours
          case 4: delayDuration = 180L * 60 * 1000; break; // 3 hours
          default: delayDuration = 30L * 60 * 1000; break; // Default to 30 minutes
        }
      }
    }
  } else {
    Serial.println("Failed to read from MAX6675 sensor");
  }
}
