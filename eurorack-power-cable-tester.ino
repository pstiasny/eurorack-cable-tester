/*
 * Arduino Micro Pin Connection Tester
 * Tests for shorts and continuity across IDC cable connections
 * 
 * Header 1: D8, D9, D10, D11, D12, D5, D6, D7
 * Header 2: D16, D17, D1, D0, D2
 * RGB LED: D13 (blue), D18 (green), D19 (red)
 */

// Define the pins we're testing
// First header - First 5 elements match positions with header2Pins
const int header1Pins[] = {8, 9, 10, 11, 12, 5, 6, 7};
const int header1Size = sizeof(header1Pins) / sizeof(header1Pins[0]);
const int connectedPinCount = 5; // Number of pins that should be connected

// Second header
const int header2Pins[] = {16, 17, 1, 0, 2};
const int header2Size = sizeof(header2Pins) / sizeof(header2Pins[0]);

// RGB LED pins (connected to cathodes, so LOW turns the LED on)
const int LED_RED = 19;   // For shorts
const int LED_GREEN = 18; // For all tests passed
const int LED_BLUE = 13;  // For missing connections

// Test result flags
bool shortsDetected = false;
bool connectionsOk = true;

void setup() {
  Serial.begin(9600);
  
  // Set up LED pins
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  
  // Turn off all LEDs (HIGH for common cathode)
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  
  Serial.println(F("Arduino Micro Pin Connection Tester"));
  Serial.println(F("--------------------------------------"));
  Serial.println(F("RGB LED Indicators:"));
  Serial.println(F("- GREEN: All tests passed"));
  Serial.println(F("- RED: Shorts detected"));
  Serial.println(F("- BLUE: Missing connections (no shorts)"));
  
  delay(1000);
}

void loop() {
  // Reset test flags
  shortsDetected = false;
  connectionsOk = true;
  
  clearSerialTerminal();

  // Turn off all LEDs before testing
  // digitalWrite(LED_RED, HIGH);
  // digitalWrite(LED_GREEN, HIGH);
  // digitalWrite(LED_BLUE, HIGH);
  
  Serial.println(F("\n=== Starting Tests ==="));
  
  // Test for shorts within each header group
  testGroupForShorts(header1Pins, header1Size, "Header 1");
  testGroupForShorts(header2Pins, header2Size, "Header 2");
  
  // Test for shorts between header groups
  testBetweenGroups();
  
  // Test continuity of connected pins
  testContinuity();
  
  // Set LED based on test results
  updateLED();
  
  //Serial.println(F("Tests complete. Waiting 5 seconds before restarting..."));
  //delay(5000);
  delay(100);
}

void testGroupForShorts(const int pins[], int size, const char* groupName) {
  Serial.print(F("Testing for shorts within "));
  Serial.println(groupName);
  
  for (int i = 0; i < size; i++) {
    // Set all pins to INPUT (high impedance)
    for (int k = 0; k < size; k++) {
      pinMode(pins[k], INPUT);
    }
    
    // Set the test pin to OUTPUT LOW
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
    
    // Test all other pins in this group
    for (int j = 0; j < size; j++) {
      if (i != j) {
        // Enable pull-up on the pin we're checking
        pinMode(pins[j], INPUT_PULLUP);
        delay(10);
        
        // If we read LOW, there's a short to our LOW output pin
        if (digitalRead(pins[j]) == LOW) {
          Serial.print(F("POTENTIAL SHORT DETECTED between D"));
          Serial.print(pins[i]);
          Serial.print(F(" and D"));
          Serial.println(pins[j]);
          shortsDetected = true;
        }
      }
    }
  }
  
  // Reset all pins to INPUT
  for (int i = 0; i < size; i++) {
    pinMode(pins[i], INPUT);
  }
  
  Serial.println(F("Group test complete"));
}

void testBetweenGroups() {
  Serial.println(F("Testing for shorts between header groups"));
  
  // Set all pins to INPUT
  for (int i = 0; i < header1Size; i++) {
    pinMode(header1Pins[i], INPUT);
  }
  for (int j = 0; j < header2Size; j++) {
    pinMode(header2Pins[j], INPUT);
  }
  
  // Test each pin in header 1 against all pins in header 2
  for (int i = 0; i < header1Size; i++) {
    pinMode(header1Pins[i], OUTPUT);
    digitalWrite(header1Pins[i], LOW);
    
    for (int j = 0; j < header2Size; j++) {
      // Skip testing expected connections (matching positions)
      if (i == j) {
        continue; // Skip this iteration - this is an expected connection
      }
      
      pinMode(header2Pins[j], INPUT_PULLUP);
      delay(10);
      
      if (digitalRead(header2Pins[j]) == LOW) {
        Serial.print(F("POTENTIAL SHORT DETECTED between D"));
        Serial.print(header1Pins[i]);
        Serial.print(F(" and D"));
        Serial.println(header2Pins[j]);
        shortsDetected = true;
      }
      
      pinMode(header2Pins[j], INPUT);
    }
    
    pinMode(header1Pins[i], INPUT);
  }
  
  Serial.println(F("Cross-group test complete"));
}

void testContinuity() {
  Serial.println(F("Testing continuity of connected pins"));
  
  // Reset all pins to INPUT
  for (int i = 0; i < header1Size; i++) {
    pinMode(header1Pins[i], INPUT);
  }
  for (int j = 0; j < header2Size; j++) {
    pinMode(header2Pins[j], INPUT);
  }
  
  // Test continuity between corresponding pins
  for (int i = 0; i < connectedPinCount; i++) {
    bool forwardPass = false;
    bool reversePass = false;
    
    // Test in forward direction
    Serial.print(F("Testing D"));
    Serial.print(header1Pins[i]);
    Serial.print(F(" -> D"));
    Serial.print(header2Pins[i]);
    Serial.print(F(": "));
    
    pinMode(header1Pins[i], OUTPUT);
    digitalWrite(header1Pins[i], LOW);
    pinMode(header2Pins[i], INPUT_PULLUP);
    delay(10);
    
    if (digitalRead(header2Pins[i]) == LOW) {
      Serial.println(F("PASS"));
      forwardPass = true;
    } else {
      Serial.println(F("FAIL - No connection"));
      connectionsOk = false;
    }
    
    pinMode(header1Pins[i], INPUT);
    pinMode(header2Pins[i], INPUT);
    
    // Test in reverse direction
    Serial.print(F("Testing D"));
    Serial.print(header2Pins[i]);
    Serial.print(F(" -> D"));
    Serial.print(header1Pins[i]);
    Serial.print(F(": "));
    
    pinMode(header2Pins[i], OUTPUT);
    digitalWrite(header2Pins[i], LOW);
    pinMode(header1Pins[i], INPUT_PULLUP);
    delay(10);
    
    if (digitalRead(header1Pins[i]) == LOW) {
      Serial.println(F("PASS"));
      reversePass = true;
    } else {
      Serial.println(F("FAIL - No connection"));
      connectionsOk = false;
    }
    
    pinMode(header1Pins[i], INPUT);
    pinMode(header2Pins[i], INPUT);
  }
  
  Serial.println(F("Continuity test complete"));
}

void clearSerialTerminal() {
  // Send escape sequence to clear the terminal screen
  // This works for most serial monitors
  Serial.write(27);     // ESC character
  Serial.print(F("[2J")); // Clear screen
  Serial.write(27);     // ESC character
  Serial.print(F("[H"));  // Move cursor to home position
}

void updateLED() {
  // Turn off all LEDs first
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  
  // Decide which LED to light based on test results
  if (shortsDetected) {
    // Red: Shorts detected (highest priority)
    digitalWrite(LED_RED, LOW);
    Serial.println(F("TEST RESULT: SHORTS DETECTED"));
  } else if (!connectionsOk) {
    // Blue: Missing connections but no shorts
    digitalWrite(LED_BLUE, LOW);
    Serial.println(F("TEST RESULT: MISSING CONNECTIONS"));
  } else {
    // Green: All tests passed
    digitalWrite(LED_GREEN, LOW);
    Serial.println(F("TEST RESULT: ALL TESTS PASSED"));
  }
}