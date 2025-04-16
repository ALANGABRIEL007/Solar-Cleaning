#include <ESP8266WiFi.h>
#include <espnow.h>

#define Motor 5  // Adjust according to your board
#define LED 4
bool On = false;
unsigned long TurnoffTime = 0;

void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  if (len > 0) {
    int d = data[0];
    int duration = 0;  // Variable to hold motor run time

    // Set motor duration based on the value of d
    if (d == 0 || d == 1) {
      duration = 20;
    } else if (d == 2) {
      duration = 40;
    } else if (d == 3) {
      duration = 30;
    } else if (d == 4) {
      duration = 60;
    } else {
      duration = 60;
    }

    // Start motor and LED
    digitalWrite(Motor, LOW);
    digitalWrite(LED, HIGH);
    On = true;
    TurnoffTime = millis() + (duration * 1000);  // Set turn-off time
    Serial.printf("Turning pump on for %d seconds\n", duration);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(Motor, OUTPUT);
  pinMode(LED, OUTPUT);
  WiFi.mode(WIFI_STA);
  digitalWrite(Motor, HIGH);

  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  if (On && millis() > TurnoffTime) {
    digitalWrite(Motor, HIGH);
    digitalWrite(LED, LOW);  // Turn LED OFF
    On = false;
  }
}
