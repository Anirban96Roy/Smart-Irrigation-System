#include <Wire.h>
#include <SparkFun_APDS9960.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// Define pins and constants
const int relay_Pin = 8;
const int DHT11_Sensor = 9;
const int moisture_sensor = A0;
const int rain_Sensor = A1;
#define DHTTYPE DHT11

// Initialize sensors and LCD
DHT dht(DHT11_Sensor, DHTTYPE);
SparkFun_APDS9960 apds = SparkFun_APDS9960();
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Global Variables
uint16_t ambient_light = 0;
uint16_t red_light = 0;
uint16_t green_light = 0;
uint16_t blue_light = 0;
int moisture_sensor_value;
int rain_Sensor_value;
float humidity_value, temperature_value;

void setup() {
  // Initialize Serial port
  Serial.begin(9600);
  pinMode(relay_Pin, OUTPUT);
  digitalWrite(relay_Pin, LOW);
  dht.begin();
  
  // Initialize LCD
  lcd.begin();
  lcd.setCursor(0,0);
  lcd.print("Smart Irrigation");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM");
  delay(2000);
  lcd.clear();

  // Initialize APDS-9960 (configure I2C and initial values)
  if (apds.init()) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
    while (1); // Halt execution if sensor initialization fails
  }

  // Start running the APDS-9960 light sensor (no interrupts)
  if (apds.enableLightSensor(false)) {
    Serial.println(F("Light sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during light sensor init!"));
    while (1); // Halt execution if light sensor fails to start
  }

  // Wait for initialization and calibration to finish
  delay(500);
}

void loop() {
  readDHT11_Sensor();
  moisture_level_detected();
  water_motor_start();
  
  // Read the color from the sensor
  if (!apds.readAmbientLight(ambient_light) ||
      !apds.readRedLight(red_light) ||
      !apds.readGreenLight(green_light) ||
      !apds.readBlueLight(blue_light)) {
    Serial.println("Error reading light values");
  } else {
    Serial.print("Ambient:");
    Serial.print(ambient_light);
    Serial.print(" Red: ");
    Serial.print(red_light);
    Serial.print(" Green: ");
    Serial.print(green_light);
    Serial.print(" Blue: ");
    Serial.println(blue_light);

    // Determine the color based on sensor readings
    String color = determineColor(red_light, green_light, blue_light);
    Serial.print("Detected Color: ");
    Serial.println(color);

    // Display the fertilizer name based on the detected color
    displayFertilizer(color);
  }

  // Wait 1 second before next reading
  delay(1000);
}

// Function to determine color based on sensor readings
String determineColor(uint16_t red, uint16_t green, uint16_t blue) {
  // pH 1-5 (Red)
  if (red >70 & red < 190 && green > 12 && green < 85 && blue > 20 && blue < 62) {
    return "pH 1-5";
  // pH 3-4 (orange++)
  } else if (red > 25 && red < 75 && green > 70 && green < 190 && blue > 30 && blue < 80) {
    return "pH 6-7";
  // pH 5-10 (Blue-Green)
  }  else if (red > 2 && red < 55 && green > 3 && green < 65 && blue > 5 && blue < 150) {
    return "pH 8-14";
  } else {
    return "Unknown";
  }
}


void displayFertilizer(String color) {
  lcd.clear();
  if (color == "pH 1-5") {
    lcd.print("Use Fertilizer:");
    lcd.setCursor(0, 1);
    lcd.print("Base Type");
  } else if (color == "pH 6-7") {
    lcd.print("Use Fertilizer:");
    lcd.setCursor(0, 1);
    lcd.print("Organic Type");
  } else if (color == "pH 8-14") {
    lcd.print("Use Fertilizer:");
    lcd.setCursor(0, 1);
    lcd.print("Acid Type");
  } else {
    lcd.print("Unknown ");
    
  }
  delay(2000);
}

// Function to read DHT11 sensor values
void readDHT11_Sensor() {
  humidity_value = dht.readHumidity();
  temperature_value = dht.readTemperature();

  if (isnan(humidity_value) || isnan(temperature_value)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(humidity_value);
  Serial.println("%");
  lcd.clear();
  lcd.print("Humidity %: ");
  lcd.setCursor(0, 1);
  lcd.print(humidity_value);
  delay(1000);

  Serial.print("Temperature: ");
  Serial.print(temperature_value);
  Serial.println("C");
  lcd.clear();
  lcd.print("Temp C: ");
  lcd.setCursor(0, 1);
  lcd.print(temperature_value);
  delay(1000);
}

// Function to detect moisture level
void moisture_level_detected() {
  moisture_sensor_value = analogRead(moisture_sensor);
  Serial.print("Moisture Level: ");
  Serial.println(moisture_sensor_value);
  lcd.clear();
  lcd.print("Moisture Level: ");
  lcd.setCursor(0, 1);
  lcd.print(moisture_sensor_value);
  delay(2000);
}

// Function to control the water motor
void water_motor_start() {
  rain_Sensor_value = analogRead(rain_Sensor);
  Serial.print("Rain sensor value: ");
  Serial.println(rain_Sensor_value);
  delay(1000);

  if (rain_Sensor_value < 700) {  // If it's raining
    digitalWrite(relay_Pin, HIGH);
  } else { // If it's not raining
    if (moisture_sensor_value > 700) {
      digitalWrite(relay_Pin, LOW);
    } else {
      digitalWrite(relay_Pin, HIGH);
    }
  }
}
