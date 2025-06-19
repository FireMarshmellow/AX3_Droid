#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>
#include <HardwareSerial.h>

// *****************************
// Pins and Constants
// *****************************

// Define motor pins
#define MOTOR1_IN1 32
#define MOTOR1_IN2 33
#define MOTOR2_IN1 12
#define MOTOR2_IN2 14

// Define the regular servo pin
#define REGULAR_SERVO_PIN 13

// Define RGB LED pins
#define LED_RED_PIN 27
#define LED_GREEN_PIN 26
#define LED_BLUE_PIN 25

// Define UART pins for DFRobot Voice Prompt Module
#define RXD2 16
#define TXD2 17

// Data packet structure
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

// Create servo object for the regular (non-continuous) servo
Servo regularServo;

// *****************************
// DFRobot Voice Prompt Functions
// *****************************

HardwareSerial mySerial(1); // Serial2 for voice module

void send_command(uint8_t command[], size_t length) {
  for (size_t i = 0; i < length; i++) {
    mySerial.write(command[i]);
  }
}

void play_track(uint8_t track_number) {
  // Adjust command as needed for your DFRobot module
  uint8_t command[] = {0x7E, 0x03, 0x00, 0x02, 0x00, track_number, 0xEF};
  send_command(command, sizeof(command));
}

void set_volume(uint8_t level) {
  if (level > 30) {
    Serial.println("Volume level must be between 0 and 30.");
    return;
  }
  uint8_t command[] = {0x7E, 0x06, 0x00, 0x02, 0x00, level, 0xEF};
  send_command(command, sizeof(command));
}

// *****************************
// Motor Control Functions
// *****************************

/**
 * controlMotor() - Basic forward/reverse/stop control for one DC motor
 *  direction > 0 => forward
 *  direction < 0 => reverse
 *  direction = 0 => stop
 */
void controlMotor(int pin1, int pin2, int direction) {
  if (direction > 0) {
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);   // Forward
  } else if (direction < 0) {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);  // Reverse
  } else {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);   // Stop
  }
}

/**
 * controlMotors() - Helper function to control both motors at once
 */
void controlMotors(int motor1Direction, int motor2Direction) {
  controlMotor(MOTOR1_IN1, MOTOR1_IN2, motor1Direction);
  controlMotor(MOTOR2_IN1, MOTOR2_IN2, motor2Direction);
}

// *****************************
// Utility Functions
// *****************************

// Set RGB LED color (HIGH=ON or LOW=OFF on each pin)
void setColor(bool red, bool green, bool blue) {
  digitalWrite(LED_RED_PIN, red);
  digitalWrite(LED_GREEN_PIN, green);
  digitalWrite(LED_BLUE_PIN, blue);
}

// Control a regular servo (0-180° as angle)
void controlRegularServo(int angle) {
  regularServo.write(angle);
}

// *****************************
// Joystick Handling
// *****************************

void joystickOutput(int x, int y, int sw, 
                    int btn1, int btn2, int btn3, 
                    int btn4, int btn5) 
{
  // Approx. midpoints based on your stated ranges
  //  X: 0 .. 3800 => midpoint ~1900
  //  Y: 0 .. 3700 => midpoint ~1850
  const int X_CENTER   = 2433;
  const int Y_CENTER   = 2480;
  // Dead-zone threshold => how far from center before you move
  const int THRESHOLD  = 300;

  // If the joystick’s SW is pressed => use "regular servo" logic
  // (the motors stop whenever SW is pressed)
  if (sw == 0) {
    // ***********************
    // SERVO CONTROL LOGIC
    // ***********************
    
    // You can map X from [0..3800] to [0..180] for servo
    // so left is 0°, right is 180°
    int rawAngle = map(x, 0, 3800, 0, 180);
    // Adjust if you need to offset the center to 90
    // This just ensures we start near 90 if x ~1900
    int angle = constrain(rawAngle, 0, 180);
    controlRegularServo(angle);

    // Simple LED feedback: turn on some color depending on direction
    // You can tune the color logic if desired
    if (x > X_CENTER) {
      setColor(1, 1, 1);  // White if joystick is to the right half
    } else {
      setColor(1, 1, 0);  // Yellow if joystick is to the left half
    }

    // Stop motors
    controlMotors(0, 0);
  
  } else {
    // ***********************
    // DC MOTOR (TANK DRIVE) LOGIC
    // ***********************
    
    // Keep the regular servo at neutral (90°) when not in servo mode
    controlRegularServo(90);

    // Decide movement based on joystick X/Y
    // Forward/Backward => Y axis
    // Left/Right => X axis

    // Check Y first
    if (y < (Y_CENTER - THRESHOLD)) {
      // Forward => both motors forward
      setColor(0, 1, 0);           
      controlMotors(1, 1);
    }
    else if (y > (Y_CENTER + THRESHOLD)) {
      // Backward => both motors backward
      setColor(1, 0, 0);           
      controlMotors(-1, -1);
    }
    // If Y is near center, check X for turning
    else if (x < (X_CENTER - THRESHOLD)) {
      // Turn left => left motor backward, right motor forward
      setColor(1, 0, 1);          
      controlMotors(-1, 1);
    }
    else if (x > (X_CENTER + THRESHOLD)) {
      // Turn right => left motor forward, right motor backward
      setColor(0, 1, 1);         
      controlMotors(1, -1);
    }
    else {
      // If within thresholds, do nothing => STOP
      controlMotors(0, 0);
      setColor(0, 0, 0);  // LED off
    }
  }

  // Play sounds if buttons are pressed
  if (btn1 == 1) play_track(1);
  if (btn2 == 1) play_track(2);
  if (btn3 == 1) play_track(3);
  if (btn4 == 1) play_track(4);
  if (btn5 == 1) play_track(5);
}

// *****************************
// ESP-NOW Callback
// *****************************

void onDataReceive(const esp_now_recv_info_t *recvInfo, 
                   const uint8_t *incomingData, int len) 
{
  if (len == sizeof(data)) {
    memcpy(&data, incomingData, sizeof(data));

    // Print received data for debugging
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

    joystickOutput(data.x, data.y, data.sw,
                   data.btn1, data.btn2, data.btn3,
                   data.btn4, data.btn5);
  } else {
    Serial.println("Received data length mismatch.");
  }
}

// *****************************
// setup()
// *****************************

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Initialize Wi-Fi in Station mode so we can get a valid MAC address.
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Print the MAC address at the start
  Serial.println("Hello, world!");
  Serial.print("Driver device MAC (at the start): ");
  Serial.println(WiFi.macAddress());

  // Optionally set a channel if you already use a specific one (like 6)
  WiFi.setChannel(6);

  // Initialize DC motor pins as outputs
  pinMode(MOTOR1_IN1, OUTPUT);
  pinMode(MOTOR1_IN2, OUTPUT);
  pinMode(MOTOR2_IN1, OUTPUT);
  pinMode(MOTOR2_IN2, OUTPUT);

  // Initialize the regular servo (pin 13)
  regularServo.attach(REGULAR_SERVO_PIN);
  controlRegularServo(90);  // Start at 90°

  // Initialize RGB LED pins
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  setColor(0, 0, 0);  // Turn off LED initially

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Register the receive callback
  esp_now_register_recv_cb(onDataReceive);

  // Initialize the DFRobot Voice Prompt Module serial
  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(1000); // Give the module some time to initialize

  // Optionally set initial volume
  set_volume(20);

  // Print confirmation of initialization
  Serial.println("ESP-NOW Driver is ready on Channel 6 and listening for data...");
}

// *****************************
// loop()
// *****************************
void loop() {
  // Nothing to do here; data is handled in the callback
}
