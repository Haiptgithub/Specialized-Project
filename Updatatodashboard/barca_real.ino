#include <DHTesp.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Arduino_MQTT_Client.h>
#include <Wire.h>

#define ESP32_ADDRESS 0x06
#define WIFI_AP "The Coffee House"
#define WIFI_PASS "thecoffeehouse"

#define TB_SERVER "thingsboard.cloud"
#define TOKEN "RvSoHM9FdM4XNfJFbTII"
constexpr uint16_t MAX_MESSAGE_SIZE = 128U;

WiFiClient espClient;
Arduino_MQTT_Client mqttClient(espClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

int lastTemp = 0;
int lastHum = 0;

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  int attempts = 0;
  
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    WiFi.begin(WIFI_AP, WIFI_PASS, 6);
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nFailed to connect to WiFi.");
  } else {
    Serial.println("\nConnected to WiFi");
  }
}

void connectToThingsBoard() {
  if (!tb.connected()) {
    Serial.println("Connecting to ThingsBoard server");
    
    if (!tb.connect(TB_SERVER, TOKEN)) {
      Serial.println("Failed to connect to ThingsBoard");
    } else {
      Serial.println("Connected to ThingsBoard");
    }
  }
}

void sendDataToThingsBoard(int temp, int hum) {
  String jsonData = "{\"temperature\":" + String(temp) + ", \"humidity\":" + String(hum) + "}";
  tb.sendTelemetryJson(jsonData.c_str());
  // Serial.println("Data for Machine 1 sent");
}

void setup() {
  Wire.begin(ESP32_ADDRESS);
  Wire.onReceive(receiveEvent); // Đăng ký hàm callback khi nhận dữ liệu
  Serial.begin(115200);
  connectToWiFi();
  connectToThingsBoard();
}

void loop() {
  tb.loop();
}

void receiveEvent(int howMany) {
  while (Wire.available() >= 2) {
    int identifier = Wire.read(); // Đọc mã định danh
    int value = Wire.read();      // Đọc giá trị dữ liệu

    if (identifier == 1) {
      lastTemp = value;
      Serial.printf("\r\n Temperature: %d °C", lastTemp);
    } else if (identifier == 2) {
      lastHum = value;
      Serial.printf("\r\n Humidity: %d  %%", lastHum);
    } else {
      Serial.printf("\r\n Unknown data received: %d", value);
    }
  }
  
  if (lastTemp != 0 && lastHum != 0) {
    sendDataToThingsBoard(lastTemp, lastHum);
  }
}