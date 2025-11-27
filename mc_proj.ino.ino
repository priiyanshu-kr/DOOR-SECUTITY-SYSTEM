#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// --- Pin definitions ---
const int relayPin = 12;         // Relay controlling solenoid (active HIGH to unlock)
bool fingerprintEnabled = false; // Start with fingerprint sensor disabled

// --- Keypad setup ---
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Keypad wiring
byte rowPins[ROWS] = {4,5,6,7};   // Rows connected to Arduino pins
byte colPins[COLS] = {8,9,10,11};   // Columns connected to Arduino pins

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// --- Fingerprint sensor setup ---
SoftwareSerial fingerSerial(2,3); // RX, TX for fingerprint sensor
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// --- PIN setup ---
const String correctPIN = "1234";  // Change to your desired PIN
String inputPIN = "";
bool pinEntryActive = false;

void setup() {
  Serial.begin(9600);

  // Relay pin setup
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Relay off (locked)

  // Initialize fingerprint sensor serial
  fingerSerial.begin(57600);
  delay(100);
  Serial.println("Initializing R307 fingerprint sensor...");

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor connected successfully.");
  } else {
    Serial.println("Fingerprint sensor not found or incorrect password.");
    // Optional: comment out halt to allow keypad-only mode
    // while (1); // Halt here if sensor not found
  }

  Serial.println("System Ready.");
  Serial.println("Fingerprint sensor is DISABLED by default.");
  Serial.println("Press '#' to toggle fingerprint ON/OFF.");
  Serial.println("Enter PIN using keypad and press '*' to submit.");
}

void loop() {
  // Handle keypad input
  char key = keypad.getKey();

  if (key) {
    Serial.print("Key Pressed: ");
    Serial.println(key);

    if (key == '#') {
      fingerprintEnabled = !fingerprintEnabled;
      Serial.print("Fingerprint sensor is now ");
      Serial.println(fingerprintEnabled ? "ENABLED" : "DISABLED");
    }
    else if (key == '*') {
      if (pinEntryActive) {
        if (inputPIN == correctPIN) {
          Serial.println("Correct PIN! Unlocking solenoid...");
          unlockSolenoid();
          Serial.println("888");
          lockSolenoid();

        } else {
          Serial.println("Incorrect PIN.");
        }
        inputPIN = "";
        pinEntryActive = false;
      }
    }
    else if (key == 'D') {
      inputPIN = "";
      pinEntryActive = false;
      Serial.println("PIN input cleared.");
    }
    else {
      if (key >= '0' && key <= '9') {
        inputPIN += key;
        pinEntryActive = true;
        Serial.print("PIN so far: ");
        Serial.println(inputPIN);
      }
    }
  }

  // Fingerprint check
  if (fingerprintEnabled) {
    uint8_t result = getFingerprintID();
    if (result == FINGERPRINT_OK) {
      Serial.println("Fingerprint authorized! Unlocking solenoid...");
      unlockSolenoid();
      Serial.println("888");
      lockSolenoid();
    }
  }

  delay(100);
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) return p;

  if (p != FINGERPRINT_OK) {
    Serial.println("Error reading finger");
    return p;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Failed to convert image");
    return p;
  }

  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence "); Serial.println(finger.confidence);
  } else {
    Serial.println("No match found");
  }

  return p;
}

void unlockSolenoid() {
  digitalWrite(relayPin, HIGH);
  Serial.println("Solenoid unlocked! Waiting 5 seconds...");
  delay(5000);
  
}
void lockSolenoid() {
  digitalWrite(relayPin, LOW);
  Serial.println("Solenoid locked.");
}

