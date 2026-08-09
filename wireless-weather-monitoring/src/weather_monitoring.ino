#define BLYNK_TEMPLATE_ID "TMPL2gYQd_hHg"
#define BLYNK_TEMPLATE_NAME "WEATHER MONITORING SYSTEM"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define VIRTUAL_TEMP V0
#define VIRTUAL_HUM V1
#define VIRTUAL_LDR V2
#define VIRTUAL_MODE V3
#define VIRTUAL_BULB V4
#define VIRTUAL_FAN V5

#define ORANGE_LED 25
#define GREEN_LED 18
#define RED_LED 5

#define LAMP_RELAY 17
#define FAN_RELAY 16

#define LDR_PIN 34
#define LIGHT_LED_PIN 27

int mode = 0;
int manualBulb = 0;
int manualFan = 0;

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");

  pinMode(ORANGE_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(LAMP_RELAY, OUTPUT);
  pinMode(FAN_RELAY, OUTPUT);
  pinMode(LIGHT_LED_PIN, OUTPUT);

  dht.begin();
  Blynk.begin(auth, ssid, pass);

  delay(2000);
  lcd.clear();
}

void loop() {
  Blynk.run();
  sendSensor();
  checkLightLevel();
  delay(2000);
}

void sendSensor() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read from DHT sensor");
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error ");
    return;
  }

  Blynk.virtualWrite(VIRTUAL_TEMP, temp);
  Blynk.virtualWrite(VIRTUAL_HUM, hum);

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temp);
  lcd.print((char)223);
  lcd.print("C ");

  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(hum);
  lcd.print("% ");

  if (mode == 0) {
    if (temp < 23) {
      digitalWrite(ORANGE_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, LOW);
      digitalWrite(LAMP_RELAY, HIGH);
      digitalWrite(FAN_RELAY, LOW);
    }
    else if (temp >= 23 && temp <= 28) {
      digitalWrite(ORANGE_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      digitalWrite(LAMP_RELAY, LOW);
      digitalWrite(FAN_RELAY, LOW);
    }
    else {
      digitalWrite(ORANGE_LED, LOW);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      digitalWrite(LAMP_RELAY, LOW);
      digitalWrite(FAN_RELAY, HIGH);
    }
  }
  else {
    digitalWrite(ORANGE_LED, LOW);
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(LAMP_RELAY, manualBulb);
    digitalWrite(FAN_RELAY, manualFan);
  }
}

void checkLightLevel() {
  int ldrValue = analogRead(LDR_PIN);
  int brightnessPercent = map(ldrValue, 0, 4095, 0, 100);

  Blynk.virtualWrite(VIRTUAL_LDR, brightnessPercent);

  if (brightnessPercent < 70) {
    digitalWrite(LIGHT_LED_PIN, LOW);
  }
  else {
    digitalWrite(LIGHT_LED_PIN, HIGH);
  }
}

BLYNK_WRITE(VIRTUAL_MODE) {
  mode = param.asInt();
  Serial.print("Mode: ");
  Serial.println(mode == 0 ? "Automatic" : "Manual");
}

BLYNK_WRITE(VIRTUAL_BULB) {
  manualBulb = param.asInt();

  if (mode == 1) {
    digitalWrite(LAMP_RELAY, manualBulb);
    Serial.print("Manual Bulb: ");
    Serial.println(manualBulb ? "ON" : "OFF");
  }
}

BLYNK_WRITE(VIRTUAL_FAN) {
  manualFan = param.asInt();

  if (mode == 1) {
    digitalWrite(FAN_RELAY, manualFan);
    Serial.print("Manual Fan: ");
    Serial.println(manualFan ? "ON" : "OFF");
  }
}
