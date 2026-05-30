#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// --- PIN DEFINITIONS ---

// I2C
#define I2C_SDA 21
#define I2C_SCL 22

// Relays (Active Low)
#define RELAY_SOLAR 26
#define RELAY_WIND  25
#define RELAY_FUEL  27

// Mode Switch (Auto/Manual) - Pull-up
#define SWITCH_MODE 32

// Source Buttons (Manual mode) - Pull-up
#define BUTTON_SOLAR 33
#define BUTTON_WIND  23
#define BUTTON_FUEL  5

// LED Indicators
#define LED_START 2
#define LED_SOLAR 4
#define LED_WIND  18
#define LED_FUEL  19

// --- CONSTANTS ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

#define VOLTAGE_THRESHOLD 5.0f // Threshold to consider source "available"

// --- OBJECTS ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_INA219 inaSolar(0x41);
Adafruit_INA219 inaWind(0x44);
Adafruit_INA219 inaFuel(0x40);

// --- GLOBAL VARIABLES ---
struct SourceData {
  float voltage;
  float current;
  float power;
  bool available;
};

SourceData solar, wind, fuel;
bool isAutoMode = true;
int activeSource = 0; // 0: None, 1: Solar, 2: Wind, 3: Fuel

// --- FUNCTION PROTOTYPES ---
void readSensors();
void updateControlLogic();
void updateDisplay();
void setRelays(int source);
void updateLEDs();

void setup() {
  Serial.begin(115200);
  
  // Initialize Pins
  pinMode(RELAY_SOLAR, OUTPUT);
  pinMode(RELAY_WIND, OUTPUT);
  pinMode(RELAY_FUEL, OUTPUT);
  
  // Ensure all relays are OFF initially (Active Low)
  digitalWrite(RELAY_SOLAR, HIGH);
  digitalWrite(RELAY_WIND, HIGH);
  digitalWrite(RELAY_FUEL, HIGH);

  pinMode(SWITCH_MODE, INPUT_PULLUP);
  pinMode(BUTTON_SOLAR, INPUT_PULLUP);
  pinMode(BUTTON_WIND, INPUT_PULLUP);
  pinMode(BUTTON_FUEL, INPUT_PULLUP);

  pinMode(LED_START, OUTPUT);
  pinMode(LED_SOLAR, OUTPUT);
  pinMode(LED_WIND, OUTPUT);
  pinMode(LED_FUEL, OUTPUT);

  // Blink Start LED
  digitalWrite(LED_START, HIGH);
  delay(500);
  digitalWrite(LED_START, LOW);

  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize Display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("MPC SYSTEM STARTING"));
  display.display();
  delay(1000);

  // Initialize Sensors
  if (!inaSolar.begin() || !inaWind.begin() || !inaFuel.begin()) {
    Serial.println(F("Failed to find INA219 chip"));
    // We continue even if one fails, but log it
  }

  Serial.println(F("MPC Initialized Successfully"));
  digitalWrite(LED_START, HIGH); // System ready LED
}

void loop() {
  readSensors();
  updateControlLogic();
  updateLEDs();
  updateDisplay();
  
  delay(200); // Sampling interval
}

void readSensors() {
  // Read Solar
  solar.voltage = inaSolar.getBusVoltage_V();
  solar.current = inaSolar.getCurrent_mA() / 1000.0f;
  solar.power = inaSolar.getPower_mW() / 1000.0f;
  solar.available = (solar.voltage > VOLTAGE_THRESHOLD);

  // Read Wind
  wind.voltage = inaWind.getBusVoltage_V();
  wind.current = inaWind.getCurrent_mA() / 1000.0f;
  wind.power = inaWind.getPower_mW() / 1000.0f;
  wind.available = (wind.voltage > VOLTAGE_THRESHOLD);

  // Read Fuel
  fuel.voltage = inaFuel.getBusVoltage_V();
  fuel.current = inaFuel.getCurrent_mA() / 1000.0f;
  fuel.power = inaFuel.getPower_mW() / 1000.0f;
  fuel.available = (fuel.voltage > VOLTAGE_THRESHOLD);
}

void updateControlLogic() {
  // Read Mode Switch (LOW = Manual, HIGH = Auto assuming toggle behavior, 
  // but let's check input logic: "Toggle switch (Auto/Manual)" 
  // connected to GND. So pressed/LOW usually means one state.)
  // Let's assume LOW = MANUAL, HIGH = AUTO (Pull-up default).
  isAutoMode = digitalRead(SWITCH_MODE);

  if (isAutoMode) {
    // AUTO MODE Priority: Solar -> Wind -> Fuel
    if (solar.available) {
      activeSource = 1;
    } else if (wind.available) {
      activeSource = 2;
    } else if (fuel.available) {
      activeSource = 3;
    } else {
      activeSource = 0; // OFF
    }
  } else {
    // MANUAL MODE
    // Logic: buttons are pulled up, LOW when pressed
    if (digitalRead(BUTTON_SOLAR) == LOW) {
      activeSource = 1;
    } else if (digitalRead(BUTTON_WIND) == LOW) {
      activeSource = 2;
    } else if (digitalRead(BUTTON_FUEL) == LOW) {
      activeSource = 3;
    } else {
      activeSource = 0; // OFF if no buttons pressed (as per logic)
    }
  }

  setRelays(activeSource);
}

void setRelays(int source) {
  // Only one relay can be active at a time for safety
  // Relays are Active Low
  digitalWrite(RELAY_SOLAR, (source == 1) ? LOW : HIGH);
  digitalWrite(RELAY_WIND,  (source == 2) ? LOW : HIGH);
  digitalWrite(RELAY_FUEL,  (source == 3) ? LOW : HIGH);
}

void updateLEDs() {
  digitalWrite(LED_SOLAR, (activeSource == 1) ? HIGH : LOW);
  digitalWrite(LED_WIND,  (activeSource == 2) ? HIGH : LOW);
  digitalWrite(LED_FUEL,  (activeSource == 3) ? HIGH : LOW);
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0,0);
  
  // Header: Mode and active source
  display.print(isAutoMode ? F("MODE: AUTO  ") : F("MODE: MANUAL"));
  display.setCursor(0, 10);
  
  display.print(F("SRC: "));
  switch(activeSource) {
    case 1: display.print(F("SOLAR")); break;
    case 2: display.print(F("WIND ")); break;
    case 3: display.print(F("FUEL ")); break;
    default: display.print(F("OFF  ")); break;
  }

  // Parameters of ACTIVE source (or cycles through them?)
  // Let's show active or defaults
  float v = 0, a = 0, p = 0;
  if(activeSource == 1) { v = solar.voltage; a = solar.current; p = solar.power; }
  else if(activeSource == 2) { v = wind.voltage; a = wind.current; p = wind.power; }
  else if(activeSource == 3) { v = fuel.voltage; a = fuel.current; p = fuel.power; }

  display.setCursor(0, 22);
  display.print(v, 1); display.print(F("V "));
  display.print(a, 2); display.print(F("A "));
  display.print(p, 1); display.print(F("W"));

  display.display();
}
