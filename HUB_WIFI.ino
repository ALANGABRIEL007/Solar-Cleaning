//By Alan Gabriel, alangabriel071@gmail.com
#define BLYNK_TEMPLATE_ID "TMPL3O23XiYhT"
#define BLYNK_TEMPLATE_NAME "TEST"
#define BLYNK_AUTH_TOKEN "l_R4crrvU78DLGUl9UgMnbyn2VqBBDCO"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <time.h> 
#include <WiFiUdp.h>
#include <NTPClient.h>

char ssid[] = "AlaniPhone";        
char password[] = "asdf1234";     

// Define NTP Client settings
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // GMT+5:30 (IST), update every minute

unsigned long lastTriggeredTime = 0;  
unsigned long debounceInterval = 60000; 

int seg_sw = 0;     
int menu = 0;     
String slot1 = "";   
String slot2 = "";  
int cycle = 1;     
int threshold = 0;   
unsigned long lastExecutionTime = 0; 
unsigned long menuInterval = 1000;  

int led = 23  ;
bool wifiConnected = false;
unsigned long previousMillis = 0;
const long blinkInterval = 500; // Blink interval in ms
bool ledState = false;
unsigned long wifiConnectedMillis = 0;
bool ledOnFor3Sec = false;

struct Data{
  int ID;
  int cyc;
  int thres;
  int intv;
};
Data senddata;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  
  pinMode(led, OUTPUT);

  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  timeClient.begin();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
}

void handleWiFi() {
  unsigned long currentMillis = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (wifiConnected) {
      wifiConnected = false;
      Serial.println("\nWi-Fi Connection Lost! Blinking LED...");
    }
    
    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      ledState = !ledState;  
      digitalWrite(led, ledState);
    }
  } else {
    if (!wifiConnected) {
      wifiConnected = true;
      wifiConnectedMillis = millis();
      ledOnFor3Sec = true;
      Serial.println("\nConnected to Wi-Fi!");
      digitalWrite(led, HIGH);
    }

    if (ledOnFor3Sec && (currentMillis - wifiConnectedMillis >= 3000)) {
      digitalWrite(led, LOW);
      ledOnFor3Sec = false;
    }
  }
}

bool isTimeMatch(String targetTime) {
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int timevalue = (currentHour * 60 * 60 + currentMinute * 60);
  String currentTime = String(timevalue);
  Serial.println("Comparing Time: " + currentTime + " with " + targetTime);
  return currentTime == targetTime;
}

BLYNK_WRITE(V0) {
  int forcestart = param.asInt();  
  if (forcestart == 1) {
    sendBroadcast();
  }
}

BLYNK_WRITE(V1) {
  seg_sw = param.asInt();
  Serial.println("seg_sw Updated: " + String(seg_sw));
}

BLYNK_WRITE(V2) {
  menu = param.asInt();
  Serial.println("menu Updated: " + String(menu));
}

BLYNK_WRITE(V3) {
  slot1 = param.asString();
  Serial.println("slot1 Updated: " + slot1);
}

BLYNK_WRITE(V4) {
  slot2 = param.asString();
  Serial.println("slot2 Updated: " + slot2);
}

BLYNK_WRITE(V7) {
  senddata.cyc = param.asInt() + 1;
  Serial.print("Cycle count Updated: ");Serial.println(senddata.cyc);
  senddata.ID = 2;
  Serial2.write((uint8_t*)&senddata,sizeof(senddata)); delay(100);
  senddata.ID = 0;
}

BLYNK_WRITE(V8) {
  senddata.thres = param.asInt();
  Serial.print("Threshold Updated: ");Serial.println(senddata.thres);
  senddata.ID = 3;
  Serial2.write((uint8_t*)&senddata,sizeof(senddata)); delay(100);
  senddata.ID = 0;
}

BLYNK_WRITE(V9) {
  senddata.intv = param.asInt();
  Serial.print("Threshold-Interval Updated: ");Serial.println(senddata.intv);
  senddata.ID = 4;
  Serial2.write((uint8_t*)&senddata,sizeof(senddata)); delay(100);
  senddata.ID = 0;
}

BLYNK_WRITE(V5) {
  Serial.println("Blynk requests temperature");
  senddata.ID = 0;
  Serial2.write((uint8_t*)&senddata,sizeof(senddata)); delay(100);

  if (Serial2.available()) {
    String receivedTemperature = Serial2.readStringUntil('\n');
    Serial.println("Temperature received: " + receivedTemperature);
    receivedTemperature.trim();
    float temperature = receivedTemperature.toFloat();
    Blynk.virtualWrite(V6, receivedTemperature);
  }
}

void sendBroadcast() {
  senddata.ID = 1;
  Serial2.write((uint8_t*)&senddata,sizeof(senddata)); delay(100);
  senddata.ID = 0;
  Serial.print("Sent via UART cycle & threshold = ");Serial.print(senddata.cyc);Serial.print(" & ");Serial.println(senddata.thres);
}

void checkLogic() {
  if (seg_sw == 0) {
    if ((millis() - lastTriggeredTime) >= debounceInterval) {
      if (isTimeMatch(slot1) || isTimeMatch(slot2)) {
        Serial.println("Specified Time Reached, sending signal");
        sendBroadcast();
        lastTriggeredTime = millis();
      }
    }
  } else if (seg_sw == 1) {
    if (millis() - lastExecutionTime >= menuInterval) {
      lastExecutionTime = millis();

      switch (menu) {
        case 0: menuInterval = 15 * 60 * 1000; Serial.println("15m interval completed, sending signal to bots"); break;
        case 1: menuInterval = 30 * 60 * 1000; Serial.println("30m interval completed, sending signal to bots"); break;
        case 2: menuInterval = 1 * 60 * 60 * 1000; Serial.println("1hr interval completed, sending signal to bots"); break;
        case 3: menuInterval = 90 * 60 * 1000; Serial.println("1.5hr interval completed, sending signal to bots"); break;
        case 4: menuInterval = 2 * 60 * 60 * 1000; Serial.println("2hr interval completed, sending signal to bots"); break;
        case 5: menuInterval = 3 * 60 * 60 * 1000; Serial.println("3hr interval completed, sending signal to bots"); break;
        default: return;
      }
      sendBroadcast();
    }
  }
}

void loop() {
  handleWiFi();

  if (wifiConnected) {
    Blynk.run();
    timeClient.update();
    checkLogic();
  }

  delay(10); // Small delay to prevent excessive CPU usage
}
