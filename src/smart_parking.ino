#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address might be 0x27 or 0x3F
Servo gateServo;

// Pin definitions
const int irSensorPin = 2;         // IR sensor for gate
const int trigPin1 = 3;            // Ultrasonic sensor 1 (spot 1)
const int echoPin1 = 4;
const int trigPin2 = 5;            // Ultrasonic sensor 2 (spot 2)
const int echoPin2 = 6;
const int servoPin = 13;           // Servo motor for gate
const int relayPin1 = 7;           // Relay Channel 1 for Spot 1
const int relayPin2 = 8;           // Relay Channel 2 for Spot 2

// Variables
int availableSpots = 2;
const int distanceThreshold = 15;  // Distance in cm to detect a vehicle

void setup() {
  // Initialize serial and LCD
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();

  // Pin modes
  pinMode(irSensorPin, INPUT);
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);

  gateServo.attach(servoPin);

  // Initial gate position (open) and display
  gateServo.write(0);  // Open position (Reversed)
  digitalWrite(relayPin1, LOW); // Relay OFF initially
  digitalWrite(relayPin2, LOW); // Relay OFF initially
  updateDisplay();
}

void loop() {
  int spot1 = isSpotOccupied(trigPin1, echoPin1);
  int spot2 = isSpotOccupied(trigPin2, echoPin2);

  // Update available spots
  availableSpots = 2 - (spot1 + spot2);
  updateDisplay();

  // Relay Control for Spot 1
  if (spot1 == 1) {
    digitalWrite(relayPin1, HIGH);  // Turn ON Relay 1 (LED 1 ON)
  } else {
    digitalWrite(relayPin1, LOW);   // Turn OFF Relay 1 (LED 1 OFF)
  }

  // Relay Control for Spot 2
  if (spot2 == 1) {
    digitalWrite(relayPin2, HIGH);  // Turn ON Relay 2 (LED 2 ON)
  } else {
    digitalWrite(relayPin2, LOW);   // Turn OFF Relay 2 (LED 2 OFF)
  }

  // Gate control logic
  if (availableSpots == 0) {
    closeGate();  // Keep gate closed if no spots are available
  } else {
    if (digitalRead(irSensorPin) == HIGH) {
      closeGate();  // Close gate if vehicle is detected at IR
    } else {
      openGate();   // Open gate if no vehicle is detected at IR
      
      // Added safety delay so the gate stays open long enough for the car 
      // to pass through safely without hitting it prematurely
      delay(3000); 
    }
  }

  delay(500);
}

// Function to check if a spot is occupied
bool isSpotOccupied(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  int distance = duration * 0.034 / 2;

  return (distance > 0 && distance < distanceThreshold);  // True if occupied
}

// Function to update the LCD display
void updateDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);

  if (availableSpots == 0) {
    lcd.print("No spots available");  // Display message when full
  } else {
    lcd.print("Total Spots: 2");
    lcd.setCursor(0, 1);
    lcd.print("Available: ");
    lcd.print(availableSpots);
  }
}

// Function to open the gate (Reversed)
void openGate() {
  gateServo.write(0);  // Open position (Reversed)
}

// Function to close the gate (Reversed)
void closeGate() {
  gateServo.write(90);   // Closed position (Reversed)
}
