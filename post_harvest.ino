#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// ==========================================
// 1. PIN DEFINITIONS (Matches Wiring Guide)
// ==========================================
#define DHTPIN 4           // DHT22 DATA to GPIO 4
#define DHTTYPE DHT22      // DHT 22 (AM2302)
#define MOISTURE_PIN 34    // Moisture Sensor Analog Out to GPIO 34
#define RELAY_PIN 5        // 5V Relay Module IN to GPIO 5
#define BUZZER_PIN 18      // Active Buzzer Positive to GPIO 18

// ==========================================
// 2. SPOILAGE PREVENTION THRESHOLDS
// ==========================================
// Adjust these based on the specific crop you are storing
const float TEMP_HIGH_THRESHOLD = 30.0;    // Temp limit in Celsius
const float HUMID_HIGH_THRESHOLD = 70.0;   // Humidity limit percentage
const int MOISTURE_HIGH_THRESHOLD = 60;    // Crop moisture limit percentage

// ==========================================
// 3. MOISTURE SENSOR CALIBRATION
// ==========================================
// ESP32 ADC reads from 0 to 4095. 
// Air (dry) is usually high (~3500+). Water (wet) is low (~1500).
const int DRY_VALUE = 3500; 
const int WET_VALUE = 1500; 

// Initialize sensor and display objects
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); // Standard I2C address is usually 0x27

void setup() {
  Serial.begin(115200);
  
  // Set output pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Start with fan and buzzer OFF
  digitalWrite(RELAY_PIN, LOW); 
  digitalWrite(BUZZER_PIN, LOW);

  // Start DHT22
  dht.begin();
  
  // Start LCD
  lcd.init();
  lcd.backlight();
  
  // Startup Message
  lcd.setCursor(0, 0);
  lcd.print("STORAGE MONITOR");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM STARTING.");
  delay(2000);
  lcd.clear();
}

void loop() {
  // --- A. READ SENSORS ---
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); 
  
  int rawMoisture = analogRead(MOISTURE_PIN);
  
  // Convert raw analog value to 0-100 percentage
  int moisturePercent = map(rawMoisture, DRY_VALUE, WET_VALUE, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  // Check for DHT22 read errors
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.setCursor(0, 0);
    lcd.print("DHT SENSOR ERROR");
    delay(2000);
    return;
  }

  // --- B. PRINT TO SERIAL MONITOR (For Debugging) ---
  Serial.print("Temp: "); Serial.print(temperature); Serial.print("C | ");
  Serial.print("Hum: "); Serial.print(humidity); Serial.print("% | ");
  Serial.print("Moist: "); Serial.print(moisturePercent); Serial.println("%");

  // --- C. UPDATE LCD DISPLAY ---
  // Row 1: Temp & Humidity
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temperature, 1); lcd.print("C ");
  lcd.setCursor(9, 0);
  lcd.print("H:"); lcd.print(humidity, 1); lcd.print("% ");

  // Row 2: Moisture & Status
  lcd.setCursor(0, 1);
  lcd.print("M:"); lcd.print(moisturePercent); lcd.print("%  ");

  // --- D. EVALUATE THRESHOLDS & TRIGGER ACTUATORS ---
  if (temperature > TEMP_HIGH_THRESHOLD || 
      humidity > HUMID_HIGH_THRESHOLD || 
      moisturePercent > MOISTURE_HIGH_THRESHOLD) {
    
    // DANGER STATE
    digitalWrite(RELAY_PIN, HIGH); // Turn ON Exhaust Fan
    
    // Flash warning on LCD
    lcd.setCursor(8, 1);
    lcd.print("FAN ON! ");
    
    // Pulse the buzzer
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(1700); // Wait remainder of the 2-second cycle
    
  } else {
    // SAFE STATE
    digitalWrite(RELAY_PIN, LOW);  // Turn OFF Exhaust Fan
    digitalWrite(BUZZER_PIN, LOW); // Turn OFF Buzzer
    
    // Show safe status on LCD
    lcd.setCursor(8, 1);
    lcd.print("SAFE    ");
    
    delay(2000); // Standard 2-second delay between DHT22 readings
  }
}