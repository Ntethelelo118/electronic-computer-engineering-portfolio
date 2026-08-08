#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "CONSTRUCTION SITE ROBOTIC ARM"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ---------- Blynk Auth & Wi-Fi ----------
char auth[] = "YOUR_BLYNK_AUTH_TOKEN";
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// ---------- Servo Setup ----------
Servo servoBase;
Servo servoShoulder;
Servo servoElbow;
Servo servoGripper;

#define BASE_PIN 13
#define SHOULDER_PIN 12
#define ELBOW_PIN 14
#define GRIPPER_PIN 27

// ---------- Ultrasonic Setup ----------
#define TRIG_PIN 5
#define ECHO_PIN 18

long duration;
int distanceCm;

// ---------- LEDs Setup ----------
#define LED_AUTO 21
#define LED_MANUAL 22
#define LED_BRICK 23

// ---------- Mode Flag ----------
bool isAuto = false;

// ---------- Blynk Virtual Pin Handlers ----------

// Mode switch (V0)
BLYNK_WRITE(V0) {
  int mode = param.asInt(); // 1 = Auto, 0 = Manual
  isAuto = (mode == 1);

  if (isAuto) {
    digitalWrite(LED_AUTO, HIGH);
    digitalWrite(LED_MANUAL, LOW);
  } else {
    digitalWrite(LED_AUTO, LOW);
    digitalWrite(LED_MANUAL, HIGH);
  }
}

// Manual control for Base (V1)
BLYNK_WRITE(V1) {
  if (!isAuto) {
    int angle = param.asInt();
    servoBase.write(angle);
  }
}

// Manual control for Shoulder (V2)
BLYNK_WRITE(V2) {
  if (!isAuto) {
    int angle = param.asInt();
    servoShoulder.write(angle);
  }
}

// Manual control for Elbow (V3)
BLYNK_WRITE(V3) {
  if (!isAuto) {
    int angle = param.asInt();
    servoElbow.write(angle);
  }
}

// Manual control for Gripper (V4)
BLYNK_WRITE(V4) {
  if (!isAuto) {
    int angle = param.asInt();
    servoGripper.write(angle);
  }
}

// ---------- Functions ----------

// Measure distance with HC-SR04
int readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30 ms
  int distance = duration * 0.034 / 2;       // distance in cm

  return distance;
}

// Auto Mode: Pick & Place sequence
void autoPickAndPlace() {
  // Example sequence (adjust angles as needed)
  servoBase.write(80);
  delay(500);

  servoShoulder.write(60);
  delay(500);

  servoElbow.write(60);
  delay(500);

  servoGripper.write(15); // close gripper
  delay(500);

  delay(1000);

  servoShoulder.write(50);
  delay(500);

  servoBase.write(150);
  delay(500);

  servoElbow.write(70);
  delay(500);

  servoGripper.write(90); // open gripper
  delay(500);
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  // Attach servos
  servoBase.attach(BASE_PIN);
  servoShoulder.attach(SHOULDER_PIN);
  servoElbow.attach(ELBOW_PIN);
  servoGripper.attach(GRIPPER_PIN);

  // Setup ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Setup LEDs
  pinMode(LED_AUTO, OUTPUT);
  pinMode(LED_MANUAL, OUTPUT);
  pinMode(LED_BRICK, OUTPUT);

  // Default LEDs: Manual ON at start
  digitalWrite(LED_AUTO, LOW);
  digitalWrite(LED_MANUAL, HIGH);
  digitalWrite(LED_BRICK, LOW);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass);
}

// ---------- Main Loop ----------
void loop() {
  Blynk.run();

  // Read ultrasonic distance
  distanceCm = readDistanceCM();
  Blynk.virtualWrite(V5, distanceCm);

  // Brick detection LED
  if (distanceCm > 0 && distanceCm <= 10) {
    digitalWrite(LED_BRICK, HIGH);

    if (isAuto) {
      autoPickAndPlace();
    }
  } else {
    digitalWrite(LED_BRICK, LOW);
  }
}
