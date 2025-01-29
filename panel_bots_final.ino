#include <ESP8266WiFi.h>
#include <espnow.h>

// Define LED and motor driver pins
#define LED_PIN 5       // LED indicator
#define MOTOR_IN1 4     // GPIO for Motor IN1
#define MOTOR_IN2 14    // GPIO for Motor IN2

// Define input buttons for End1 and End2
#define END1_BUTTON_PIN 12 // GPIO for End1 button
#define END2_BUTTON_PIN 13 // GPIO for End2 button

// Variables for motor state and timing
bool motorRunning = false;
unsigned long lastActionTime = 0;
unsigned long timeoutPeriod = 10000; // 10-second timeout

int currentCycle = 0; // Current cycle count
int totalCycles = 0;  // Total cycles to perform
bool forwardPhase = true; // Indicates forward or reverse motor state

// Callback function to handle incoming ESP-NOW data
void onDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  if (len > 0) {
    int receivedData = data[0];
    Serial.print("Data received: ");
    Serial.println(receivedData);

    // Determine total cycles based on received data
    if (receivedData >= 5) {
      totalCycles = 5;
    } else {
      totalCycles = max(receivedData, 1);
    }

    // Start first cycle
    currentCycle = 1;
    motorRunning = true;
    lastActionTime = millis();

    // Start motor forward
    digitalWrite(MOTOR_IN1, HIGH);
    digitalWrite(MOTOR_IN2, LOW);
    forwardPhase = true;
    Serial.println("Motor started forward for cycle 1");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(END1_BUTTON_PIN, INPUT_PULLUP);
  pinMode(END2_BUTTON_PIN, INPUT_PULLUP);

  // Initialize Wi-Fi and ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onDataRecv);
}

void loop() {
  unsigned long currentTime = millis();

  if (motorRunning) {
    // Check for timeout
    if (currentTime - lastActionTime > timeoutPeriod) {
      Serial.println("Timeout: Motor stop due to inactivity");
      motorRunning = false;
      stopMotor();
      return;
    }

    // Check for End1 button press (Reverse direction)
    if (digitalRead(END1_BUTTON_PIN) == LOW) {
      if (forwardPhase) {
        Serial.println("End1 button pressed: Motor reverse");
        forwardPhase = false;
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        delay(750);
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, HIGH);
      } else {
        Serial.println("End1 button pressed: No action (already reversing)");
      }
      lastActionTime = currentTime;
      delay(100); // Basic debounce
    }

    // Check for End2 button press (Stop or move to next cycle)
    if (digitalRead(END2_BUTTON_PIN) == LOW) {
      if (forwardPhase) {
        Serial.println("End2 button pressed: Motor continues forward");
        // Continue forward
        digitalWrite(MOTOR_IN1, LOW);
        digitalWrite(MOTOR_IN2, LOW);
        delay(750);        
        digitalWrite(MOTOR_IN1, HIGH);
        digitalWrite(MOTOR_IN2, LOW);
      } else {
        Serial.println("End2 button pressed: Motor stopped for this cycle");
        forwardPhase = true; // Reset for the next cycle
        currentCycle++;
        if (currentCycle > totalCycles) {
          motorRunning = false;
          Serial.println("All cycles completed. Motor stopped.");
          stopMotor();
        } else {
          Serial.print("Starting next cycle: ");
          Serial.println(currentCycle);
          // Start forward for next cycle
          digitalWrite(MOTOR_IN1, LOW);
          digitalWrite(MOTOR_IN2, LOW);
          delay(750);
          digitalWrite(MOTOR_IN1, HIGH);
          digitalWrite(MOTOR_IN2, LOW);
        }
      }
      lastActionTime = currentTime;
      delay(100); // Basic debounce
    }
  }

  // LED indicator (ON during motor operation)
  digitalWrite(LED_PIN, motorRunning);
}

// Function to stop the motor
void stopMotor() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  motorRunning = false;
  digitalWrite(LED_PIN, LOW);
  Serial.println("Motor completely stopped.");
}
