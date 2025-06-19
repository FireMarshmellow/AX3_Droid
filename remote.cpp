#include <esp_now.h>
#include <WiFi.h>

// Joystick pins (adjust if necessary for your hardware)
#define JOYSTICK_X 4
#define JOYSTICK_Y 5
#define JOYSTICK_SW 23

// Button pins
#define BUTTON_1 19
#define BUTTON_2 20
#define BUTTON_3 7
#define BUTTON_4 21
#define BUTTON_5 22

// Constants for joystick calibration
const int REST_X = 32767;
const int REST_Y = 32767;

// Replace this with the MAC address of your driver device (found via Serial print of WiFi.macAddress() on the driver)
uint8_t driverMAC[] = {0x, 0x, 0x, 0x, 0x, 0x}; 

// Structure to hold data to send
struct DataPacket {
  int x;
  int y;
  int sw;
  int btn1;
  int btn2;
  int btn3;
  int btn4;
  int btn5;
};

DataPacket data;

void sendData();

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize input pins
  pinMode(JOYSTICK_X, INPUT);
  pinMode(JOYSTICK_Y, INPUT);
  pinMode(JOYSTICK_SW, INPUT_PULLUP);

  pinMode(BUTTON_1, INPUT_PULLDOWN);
  pinMode(BUTTON_2, INPUT_PULLDOWN);
  pinMode(BUTTON_3, INPUT_PULLDOWN);
  pinMode(BUTTON_4, INPUT_PULLDOWN);
  pinMode(BUTTON_5, INPUT_PULLDOWN);

  // Initialize Wi-Fi in station mode and ensure it's disconnected from any AP
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // ensure not connected to any Wi-Fi network
  // Set the ESP-NOW channel. Both devices must use the same channel.
  WiFi.setChannel(6);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Add the driver device's MAC address as a peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, driverMAC, 6);
  peerInfo.channel = 6;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("ESP-NOW Remote ready on Channel 6");
}

void loop() {
  // Read joystick values
  data.x = analogRead(JOYSTICK_X);
  data.y = analogRead(JOYSTICK_Y);
  data.sw = digitalRead(JOYSTICK_SW);

  // Read button states (inverting because they are INPUT_PULLUP)
  data.btn1 = digitalRead(BUTTON_1);
  data.btn2 = digitalRead(BUTTON_2);
  data.btn3 = digitalRead(BUTTON_3);
  data.btn4 = digitalRead(BUTTON_4);
  data.btn5 = digitalRead(BUTTON_5);

  // Print data to Serial Monitor for debugging
  Serial.print("Joystick X: ");
  Serial.print(data.x);
  Serial.print(" | Y: ");
  Serial.print(data.y);
  Serial.print(" | SW: ");
  Serial.print(data.sw);
  Serial.print(" | BTN1: ");
  Serial.print(data.btn1);
  Serial.print(" | BTN2: ");
  Serial.print(data.btn2);
  Serial.print(" | BTN3: ");
  Serial.print(data.btn3);
  Serial.print(" | BTN4: ");
  Serial.print(data.btn4);
  Serial.print(" | BTN5: ");
  Serial.println(data.btn5);

  // Send data to the driver device
  sendData();

  delay(50); // small delay to avoid spamming
}

void sendData() {
  // Send data to the known peer (driverMAC)
  esp_err_t result = esp_now_send(driverMAC, (uint8_t *)&data, sizeof(data));

  if (result == ESP_OK) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Error sending data");
  }
}
