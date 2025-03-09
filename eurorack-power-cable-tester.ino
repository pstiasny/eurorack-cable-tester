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

// Store detected issues
String shortsList = "";
String disconnectsList = "";

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
}

void loop() {
  // Run the test
  runTest();
  
  // Display results if serial is available
  if (Serial) {
    clearSerialTerminal();
    // Only print final result and any detected issues
    if (shortsDetected) {
      Serial.println(F("TEST RESULT: SHORTS DETECTED\r"));
      Serial.print(shortsList);
    } else if (!connectionsOk) {
      Serial.println(F("TEST RESULT: MISSING CONNECTIONS\r"));
      Serial.print(disconnectsList);
    } else {
      Serial.println(F("TEST RESULT: ALL TESTS PASSED\r"));
    }
  }
}

void clearSerialTerminal() {
  // Send escape sequence to clear the terminal screen
  // This works for most serial monitors
  Serial.write(27);     // ESC character
  Serial.print(F("[2J\r")); // Clear screen
  Serial.write(27);     // ESC character
  Serial.print(F("[H\r"));  // Move cursor to home position
}

void runTest() {
  // Clear results
  shortsList = "";
  disconnectsList = "";
  
  // Reset test flags
  shortsDetected = false;
  connectionsOk = true;
  
  // Test for shorts within each header group
  testGroupForShorts(header1Pins, header1Size);
  testGroupForShorts(header2Pins, header2Size);
  
  // Test for shorts between header groups
  testBetweenGroups();
  
  // Test continuity of connected pins
  testContinuity();
  
  // Set LED based on test results
  updateLED();
}

void testGroupForShorts(const int pins[], int size) {
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
          shortsList += F("SHORT: D");
          shortsList += pins[i];
          shortsList += F(" - D");
          shortsList += pins[j];
          shortsList += F("\r\n");
          shortsDetected = true;
        }
      }
    }
  }
  
  // Reset all pins to INPUT
  for (int i = 0; i < size; i++) {
    pinMode(pins[i], INPUT);
  }
}

void testBetweenGroups() {
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
        shortsList += F("SHORT: D");
        shortsList += header1Pins[i];
        shortsList += F(" - D");
        shortsList += header2Pins[j];
        shortsList += F("\r\n");
        shortsDetected = true;
      }
      
      pinMode(header2Pins[j], INPUT);
    }
    
    pinMode(header1Pins[i], INPUT);
  }
}

void testContinuity() {
  // Reset all pins to INPUT
  for (int i = 0; i < header1Size; i++) {
    pinMode(header1Pins[i], INPUT);
  }
  for (int j = 0; j < header2Size; j++) {
    pinMode(header2Pins[j], INPUT);
  }
  
  // Test continuity between corresponding pins
  for (int i = 0; i < connectedPinCount; i++) {
    // Test in forward direction
    pinMode(header1Pins[i], OUTPUT);
    digitalWrite(header1Pins[i], LOW);
    pinMode(header2Pins[i], INPUT_PULLUP);
    delay(10);
    
    if (digitalRead(header2Pins[i]) != LOW) {
      // Record failure
      disconnectsList += F("OPEN: D");
      disconnectsList += header1Pins[i];
      disconnectsList += F(" -> D");
      disconnectsList += header2Pins[i];
      disconnectsList += F("\r\n");
      connectionsOk = false;
    }
    
    pinMode(header1Pins[i], INPUT);
    pinMode(header2Pins[i], INPUT);
    
    // Test in reverse direction
    pinMode(header2Pins[i], OUTPUT);
    digitalWrite(header2Pins[i], LOW);
    pinMode(header1Pins[i], INPUT_PULLUP);
    delay(10);
    
    if (digitalRead(header1Pins[i]) != LOW) {
      // Record failure
      disconnectsList += F("OPEN: D");
      disconnectsList += header2Pins[i];
      disconnectsList += F(" -> D");
      disconnectsList += header1Pins[i];
      disconnectsList += F("\r\n");
      connectionsOk = false;
    }
    
    pinMode(header1Pins[i], INPUT);
    pinMode(header2Pins[i], INPUT);
  }
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
  } else if (!connectionsOk) {
    // Blue: Missing connections but no shorts
    digitalWrite(LED_BLUE, LOW);
  } else {
    // Green: All tests passed
    digitalWrite(LED_GREEN, LOW);
  }
}