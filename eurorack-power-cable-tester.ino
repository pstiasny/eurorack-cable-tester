/*
 * Vibe-coded with Claude
 * 
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

// Track which pins have problems
bool pinShorts[8][5] = {false}; // Track shorts between headers
bool header1InternalShorts[8][8] = {false}; // Track shorts within header1
bool header2InternalShorts[5][5] = {false}; // Track shorts within header2
bool pinOpens[5] = {false};     // Track opens in expected connections

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
    
    // Print ASCII art representation first
    printHeadersAsciiArt();
    
    // Then print the text output
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

void resetTestState() {
  // Clear results
  shortsList = "";
  disconnectsList = "";
  
  // Reset test flags
  shortsDetected = false;
  connectionsOk = true;
  
  // Reset pin problem tracking
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 5; j++) {
      pinShorts[i][j] = false;
    }
  }
  
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      header1InternalShorts[i][j] = false;
    }
  }
  
  for (int i = 0; i < 5; i++) {
    for (int j = 0; j < 5; j++) {
      header2InternalShorts[i][j] = false;
    }
  }
  
  for (int i = 0; i < 5; i++) {
    pinOpens[i] = false;
  }
}

void runTest() {
  // Reset all test state
  resetTestState();
  
  // Test for shorts within each header group
  testHeader1ForShorts();
  testHeader2ForShorts();
  
  // Test for shorts between header groups
  testBetweenGroups();
  
  // Test continuity of connected pins
  testContinuity();
  
  // Set LED based on test results
  updateLED();
}

void testHeader1ForShorts() {
  for (int i = 0; i < header1Size; i++) {
    // Set all pins to INPUT (high impedance)
    for (int k = 0; k < header1Size; k++) {
      pinMode(header1Pins[k], INPUT);
    }
    
    // Set the test pin to OUTPUT LOW
    pinMode(header1Pins[i], OUTPUT);
    digitalWrite(header1Pins[i], LOW);
    
    // Test all other pins in this group
    for (int j = 0; j < header1Size; j++) {
      if (i != j) {
        // Enable pull-up on the pin we're checking
        pinMode(header1Pins[j], INPUT_PULLUP);
        delay(10);
        
        // If we read LOW, there's a short to our LOW output pin
        if (digitalRead(header1Pins[j]) == LOW) {
          shortsList += F("SHORT: D");
          shortsList += header1Pins[i];
          shortsList += F(" - D");
          shortsList += header1Pins[j];
          shortsList += F("\r\n");
          shortsDetected = true;
          
          // Track internal shorts in header1
          header1InternalShorts[i][j] = true;
        }
      }
    }
  }
  
  // Reset all pins to INPUT
  for (int i = 0; i < header1Size; i++) {
    pinMode(header1Pins[i], INPUT);
  }
}

void testHeader2ForShorts() {
  for (int i = 0; i < header2Size; i++) {
    // Set all pins to INPUT (high impedance)
    for (int k = 0; k < header2Size; k++) {
      pinMode(header2Pins[k], INPUT);
    }
    
    // Set the test pin to OUTPUT LOW
    pinMode(header2Pins[i], OUTPUT);
    digitalWrite(header2Pins[i], LOW);
    
    // Test all other pins in this group
    for (int j = 0; j < header2Size; j++) {
      if (i != j) {
        // Enable pull-up on the pin we're checking
        pinMode(header2Pins[j], INPUT_PULLUP);
        delay(10);
        
        // If we read LOW, there's a short to our LOW output pin
        if (digitalRead(header2Pins[j]) == LOW) {
          shortsList += F("SHORT: D");
          shortsList += header2Pins[i];
          shortsList += F(" - D");
          shortsList += header2Pins[j];
          shortsList += F("\r\n");
          shortsDetected = true;
          
          // Track internal shorts in header2
          header2InternalShorts[i][j] = true;
        }
      }
    }
  }
  
  // Reset all pins to INPUT
  for (int i = 0; i < header2Size; i++) {
    pinMode(header2Pins[i], INPUT);
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
      // But only skip if i is within the connected range
      if (i < connectedPinCount && i == j) {
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
        
        // Track cross-header shorts
        pinShorts[i][j] = true;
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
    bool forwardFailed = false;
    bool reverseFailed = false;
    
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
      forwardFailed = true;
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
      reverseFailed = true;
    }
    
    pinMode(header1Pins[i], INPUT);
    pinMode(header2Pins[i], INPUT);
    
    // If either direction failed, mark this pin as open for ASCII art
    if (forwardFailed || reverseFailed) {
      pinOpens[i] = true;
    }
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

void printHeadersAsciiArt() {
  // Top edges
  Serial.println(F("+--+    +--+"));
  
  // Based on the schematic, the connected pins should be displayed in this order:
  // Header1 (top to bottom): D12, D11, D10, D9, D8
  // Header2 (top to bottom): D16, D17, D1, D0, D2
  
  // For our arrays:
  // header1Pins[] = {8, 9, 10, 11, 12, 5, 6, 7};  // First 5 are connected
  // header2Pins[] = {16, 17, 1, 0, 2};            // All 5 are connected
  
  // Therefore we need to display them in reverse order
  
  // Print each pin row for connectedPinCount
  for (int i = connectedPinCount - 1; i >= 0; i--) {
    // Start of line for header 1
    Serial.print(F("|"));
    
    // Check for shorts on this pin (prioritized)
    bool hasShort1 = false;
    
    // First check for cross-header shorts
    for (int j = 0; j < header2Size; j++) {
      if (pinShorts[i][j]) {
        hasShort1 = true;
        break;
      }
    }
    
    // Then check for internal shorts within header1
    if (!hasShort1) {
      for (int j = 0; j < header1Size; j++) {
        if (header1InternalShorts[i][j] || header1InternalShorts[j][i]) {
          hasShort1 = true;
          break;
        }
      }
    }
    
    // Pin state for header 1 - shorts take priority
    if (hasShort1) {
      Serial.print(F("!!")); // Short to another pin
    } else if (pinOpens[i]) {
      Serial.print(F("XX")); // Open connection
    } else {
      Serial.print(F("oo")); // Normal pin
    }
    
    // Connection indicator - only based on open status, not shorts
    if (pinOpens[i]) {
      Serial.print(F("|    |"));
    } else {
      Serial.print(F("|====|"));
    }
    
    // Check for shorts on this pin (prioritized)
    bool hasShort2 = false;
    
    // First check for cross-header shorts
    for (int j = 0; j < header1Size; j++) {
      if (pinShorts[j][i]) {
        hasShort2 = true;
        break;
      }
    }
    
    // Then check for internal shorts within header2
    if (!hasShort2) {
      for (int j = 0; j < header2Size; j++) {
        if (header2InternalShorts[i][j] || header2InternalShorts[j][i]) {
          hasShort2 = true;
          break;
        }
      }
    }
    
    // Pin state for header 2 - shorts take priority
    if (hasShort2) {
      Serial.print(F("!!")); // Short to another pin
    } else if (pinOpens[i]) {
      Serial.print(F("XX")); // Open connection
    } else {
      Serial.print(F("oo")); // Normal pin
    }
    
    // End of line for header 2
    Serial.println(F("|"));
  }
  
  // Print unconnected pins in order: D7, D6, D5
  // In the header1Pins array, these are at indices 7, 6, 5
  
  // Print first unconnected pin (D7) with the bottom edge of header 2
  int i = 7; // D7 index
  
  // Start of line for header 1
  Serial.print(F("|"));
  
  // Check for shorts on this pin (prioritized)
  bool hasShort = false;
  
  // First check for cross-header shorts
  for (int j = 0; j < header2Size; j++) {
    if (pinShorts[i][j]) {
      hasShort = true;
      break;
    }
  }
  
  // Then check for internal shorts within header1
  if (!hasShort) {
    for (int j = 0; j < header1Size; j++) {
      if (header1InternalShorts[i][j] || header1InternalShorts[j][i]) {
        hasShort = true;
        break;
      }
    }
  }
  
  // Pin state - show shorts if any exist
  if (hasShort) {
    Serial.print(F("!!"));
  } else {
    Serial.print(F("oo"));
  }
  
  // End of line for header 1 and bottom of header 2
  Serial.println(F("|    +--+"));
  
  // Print remaining pins (D6, D5) - going from index 6 down to 5
  for (i = 6; i >= 5; i--) {
    // Start of line
    Serial.print(F("|"));
    
    // Check for shorts on this pin (prioritized)
    bool hasShort = false;
    
    // First check for cross-header shorts
    for (int j = 0; j < header2Size; j++) {
      if (pinShorts[i][j]) {
        hasShort = true;
        break;
      }
    }
    
    // Then check for internal shorts within header1
    if (!hasShort) {
      for (int j = 0; j < header1Size; j++) {
        if (header1InternalShorts[i][j] || header1InternalShorts[j][i]) {
          hasShort = true;
          break;
        }
      }
    }
    
    // Pin state - show shorts if any exist
    if (hasShort) {
      Serial.print(F("!!"));
    } else {
      Serial.print(F("oo"));
    }
    
    // End of line
    Serial.println(F("|"));
  }
  
  // Bottom edge of header 1
  Serial.println(F("+--+"));
  
  Serial.println();
}