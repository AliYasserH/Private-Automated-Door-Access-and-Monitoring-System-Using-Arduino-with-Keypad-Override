#include <LiquidCrystal.h>
#include <Servo.h>
#include <Keypad.h>

// LCD pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Ultrasonic sensor
const int trigPin = 9;
const int echoPin = 10;

// Outputs
const int servoPin = 6;
const int greenLedPin = 8;
const int redLedPin = 13;
const int buzzerPin = A0;

// State variables
long duration;
float distance;
int openCount = 0;
bool isFull = false;

Servo doorServo;

// Keypad setup (4x3)
const byte ROWS = 4;
const byte COLS = 3;

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

// Keypad wiring to available pins
byte rowPins[ROWS] = {A1, A2, A3, A4};
byte colPins[COLS] = {A5, 7, A0};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
String enteredPassword = "";
const String overridePassword = "1245";

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  digitalWrite(buzzerPin, LOW);

  doorServo.attach(servoPin);
  doorServo.write(0); // Closed

  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Door Counter:");
  lcd.setCursor(0, 1);
  lcd.print("Opened: 0 times");

  Serial.begin(9600);
}

void loop() {
  // Keypad password input
  char key = keypad.getKey();
  if (key) {
    if (key == '*') {
      if (enteredPassword == overridePassword) {
        lcd.setCursor(0, 1);
        lcd.print("Override Access   ");
        openDoor();
        delay(1000);
        closeDoor();
      } else {
        lcd.setCursor(0, 1);
        lcd.print("Wrong Password!   ");
        digitalWrite(buzzerPin, HIGH);
        delay(1000);
        digitalWrite(buzzerPin, LOW);
      }
      enteredPassword = "";
    } else if (key == '#') {
      enteredPassword = "";  // cancel
    } else {
      enteredPassword += key;
      lcd.setCursor(0, 1);
      lcd.print("Pass: ");
      lcd.print(enteredPassword);
      lcd.print("        ");
    }
  }

  // Trigger ultrasonic sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  distance = (duration * 0.034) / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if (distance > 0 && distance < 30) {
    if (openCount < 10) {
      openCount++;
      lcd.setCursor(0, 1);
      lcd.print("Opened: ");
      lcd.print(openCount);
      lcd.print(" times ");
      openDoor();
      delay(1000); // open for 1s
      closeDoor();
      isFull = (openCount >= 10);
    } else {
      // Entry denied due to full capacity
      lcd.setCursor(0, 1);
      lcd.print(" FULL - NO ENTRY ");
      digitalWrite(greenLedPin, LOW);
      digitalWrite(redLedPin, HIGH);
      digitalWrite(buzzerPin, HIGH);
      delay(1000);
      digitalWrite(buzzerPin, LOW);
    }
  }

  // LED status
  if (openCount >= 10) {
    isFull = true;
    digitalWrite(greenLedPin, LOW);
    digitalWrite(redLedPin, HIGH);
  } else {
    isFull = false;
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
  }

  delay(100);
}

void openDoor() {
  doorServo.write(90);
}

void closeDoor() {
  doorServo.write(0);
}
