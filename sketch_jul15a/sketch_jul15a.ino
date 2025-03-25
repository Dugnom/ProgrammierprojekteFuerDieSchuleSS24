#include "DHT.h"

// Define sensor pins
#define DHTPIN 7         // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11    // DHT 11

// Define analog pins
#define LOUDNESS_PIN A0
#define OXYGEN_PIN A2
#define LIGHT_PIN A3

// Define output pins
#define YELLOW_LED_PIN 2
#define WHITE_LED_PIN 3
#define RED_LED_PIN 4
#define BUZZER_PIN 5
#define VIBRATOR_PIN 6
#define TILT_SENSOR_PIN 8

DHT dht(DHTPIN, DHTTYPE);

// Threshold values
int loudnessThreshold = 600; // Suggested levels: Quiet: 0-200, Normal: 201-600, Loud: 601-1023
int lightThreshold = 300; // Warning for dim light: Dim: 0-300, Bright: 301-1023
int tempThreshold = 30; // Suggested levels: Cool: 0-15°C, Comfortable: 16-25°C, Hot: 26-40°C
float o2Threshold = 19.5; // Percentage threshold for low oxygen concentration
float humidityLowThreshold = 30.0; // Lower limit for acceptable humidity
float humidityHighThreshold = 90.0; // Upper limit for acceptable humidity

void setup() {
  Serial.begin(9600);
  dht.begin();
  
  // Initialize digital pins as outputs
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(WHITE_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR_PIN, OUTPUT);
  
  // Initialize tilt sensor as input
  pinMode(TILT_SENSOR_PIN, INPUT);
  
  // Initialize the white LED to indicate the Arduino is running
  digitalWrite(WHITE_LED_PIN, HIGH);
}

void loop() {
  // Read sensors
  int loudness = analogRead(LOUDNESS_PIN);
  int o2 = analogRead(OXYGEN_PIN);
  int light = analogRead(LIGHT_PIN);
  int tilt = digitalRead(TILT_SENSOR_PIN);
  
  // Read temperature and humidity
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(temp) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Calculate O2 concentration
  float Vout = readO2Vout();
  float O2Concentration = readO2Concentration(Vout);

  // Check if too loud
  if (loudness > loudnessThreshold) {
    digitalWrite(RED_LED_PIN, HIGH); // Turn on red LED
  } else {
    digitalWrite(RED_LED_PIN, LOW); // Turn off red LED
  }
  
  // Check if too dim
  if (light < lightThreshold) {
    digitalWrite(YELLOW_LED_PIN, HIGH); // Turn on yellow LED
  } else {
    digitalWrite(YELLOW_LED_PIN, LOW); // Turn off yellow LED
  }
  
  // Check if too hot
  if (temp > tempThreshold) {
    digitalWrite(VIBRATOR_PIN, HIGH); // Turn on vibrator
  } else {
    digitalWrite(VIBRATOR_PIN, LOW); // Turn off vibrator
  }
  
  // Check if oxygen level is low
  if (O2Concentration < o2Threshold) {
    digitalWrite(BUZZER_PIN, HIGH); // Turn on buzzer
  } else {
    digitalWrite(BUZZER_PIN, LOW); // Turn off buzzer
  }

  // Check if humidity is out of range
  if (humidity < humidityLowThreshold || humidity > humidityHighThreshold) {
    digitalWrite(VIBRATOR_PIN, HIGH); // Short vibrations to warn about humidity
    delay(100);
    digitalWrite(VIBRATOR_PIN, LOW);
    delay(100);
  }
  
  // Check tilt sensor
  if (tilt == HIGH) {
    // Pause the system
    digitalWrite(YELLOW_LED_PIN, HIGH);
    digitalWrite(WHITE_LED_PIN, LOW); 
    delay(5000); 
    digitalWrite(YELLOW_LED_PIN, LOW); 
    digitalWrite(WHITE_LED_PIN, HIGH);

    // Send sensor data over Serial for debugging purposes
    Serial.print("Loudness: ");
    Serial.print(loudness);
    Serial.print(" | Temperature: ");
    Serial.print(temp);
    Serial.print("°C | Humidity: ");
    Serial.print(humidity);
    Serial.print("% | Oxygen Concentration: ");
    Serial.print(O2Concentration);
    Serial.print("% | Light Level: ");
    Serial.print(light);
    Serial.print(" | Tilt: ");
    Serial.println(tilt);
  }
  
  
  // Wait for 1 second before the next loop
  delay(1000);
}

float readO2Vout() {
  long sum = 0;
  for(int i = 0; i < 32; i++) {
    sum += analogRead(OXYGEN_PIN);
  }
  sum >>= 5;
  float Vout = sum * (5.0 / 1023.0);
  return Vout;
}

float readO2Concentration(float Vout) {
  // Calculate concentration based on Vout
  // According to the sensor's documentation:
  // O2 concentration (%) = (Vout / 5V) * 21%
  float O2Concentration = (Vout / 5.0) * 21.0 * 100;
  return O2Concentration;
}
