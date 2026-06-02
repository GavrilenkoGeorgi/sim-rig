#include <Joystick.h>

// --- PIN DEFINITIONS ---
const int PIN_REV       = 2;   
const int PIN_SHIFTER_X = A0;  
const int PIN_SHIFTER_Y = A1;  

const int PIN_GAS       = A2;  
const int PIN_BRAKE     = A3;  
const int PIN_CLUTCH    = A6;  

const int BUTTON_PINS[4] = {3, 5, 6, 7}; 
const int TOGGLE_PINS[4] = {8, 9, 10, 16}; 

// --- DEBOUNCE TRACKING VARIABLES ---
static int lastGear = 0;               // 0 = Neutral, 9-14 = Gears 1-6
static unsigned long lastChangeTime = 0;
const unsigned long debounceDelay = 50; // Milliseconds stick must stay still

// --- JOYSTICK INITIALIZATION ---
// Arguments: Report ID, Type, Button Count, Hat Count, 
// X, Y, Z, Rx, Ry, Rz, Rudder, Throttle, Accelerator, Brake, Steering
Joystick_ SimRig(
  JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, 
  16, 0,                  // 16 Buttons allocated, 0 Hat switches
  true, true, true,       // X (Shifter X), Y (Shifter Y), Z (Gas)
  true, true, false,      // Rx (Brake), Ry (Clutch), Rz disabled
  false, false,           // Rudder, Throttle disabled
  false, false,           // Specialized Accelerator/Brake disabled (using standard axes instead)
  false                   // Steering disabled
);

void setup() {
  // Enable to debug raw input values
  // Serial.begin(9600);

  pinMode(PIN_REV, INPUT); 
  for (int i = 0; i < 4; i++) { pinMode(BUTTON_PINS[i], INPUT_PULLUP); }
  for (int i = 0; i < 4; i++) { pinMode(TOGGLE_PINS[i], INPUT_PULLUP); }

  SimRig.begin();
}

void loop() {
  int rawX = analogRead(PIN_SHIFTER_X);
  int rawY = analogRead(PIN_SHIFTER_Y);
  int gas   = analogRead(PIN_GAS);
  int brake = analogRead(PIN_BRAKE);
  int clutch = analogRead(PIN_CLUTCH);

  // 1. HANDLE STANDARD JOYSTICK AXES
  SimRig.setXAxis(rawX);
  SimRig.setYAxis(rawY);
  SimRig.setZAxis(gas);
  SimRig.setRxAxis(brake);
  SimRig.setRyAxis(clutch);

  // 2. REVERSE GEAR (Button 1)
  SimRig.setButton(0, digitalRead(PIN_REV) == HIGH); 

  // 3. STANDALONE BUTTONS (Buttons 2-5)
  for (int i = 0; i < 4; i++) {
    SimRig.setButton(i + 1, !digitalRead(BUTTON_PINS[i]));
  }

  // 4. TOGGLE SWITCHES (Buttons 6-9)
  for (int i = 0; i < 4; i++) {
    SimRig.setButton(i + 5, !digitalRead(TOGGLE_PINS[i]));
  }

  // 5. DEBOUNCED H-PATTERN SHIFTER LOGIC (Buttons 10-15)
  int currentGear = 0; // Default to 0 (Neutral)

  // PRIORITIZE REVERSE: If Reverse switch is pressed, skip H-pattern logic entirely
  if (digitalRead(PIN_REV) == HIGH) {
    currentGear = 0; // Forces forward gears to remain in Neutral (0)
  } 
  else {
    // Calculate raw gear zone based on your precise calibrated thresholds
    if (rawX < 270) {          // LEFT GATE
      if (rawY > 700)      currentGear = 9;  // 1st Gear
      else if (rawY < 300) currentGear = 10; // 2nd Gear
    } 
    else if (rawX > 560) {     // RIGHT GATE
      if (rawY > 700)      currentGear = 13; // 5th Gear
      else if (rawY < 300) currentGear = 14; // 6th Gear
    } 
    else {                     // CENTER GATE
      if (rawY > 700)      currentGear = 11; // 3rd Gear
      else if (rawY < 300) currentGear = 12; // 4th Gear
    }
  }

  // Evaluate state change stability
  if (currentGear != lastGear) {
    if (millis() - lastChangeTime > debounceDelay) {
      // 1. Turn off the previous gear button safely (if it wasn't Neutral)
      if (lastGear >= 9 && lastGear <= 14) {
        SimRig.setButton(lastGear, false);
      }
      // 2. Turn on the new gear button safely (if it isn't Neutral)
      if (currentGear >= 9 && currentGear <= 14) {
        SimRig.setButton(currentGear, true);
      }
      // 3. Lock the new gear into stable memory
      lastGear = currentGear;
    }
  } else {
    // If the raw coordinates match our current stable gear, reset the timer
    lastChangeTime = millis();
  }

  // 6. DEBUG PRINTING
  /* static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 150) {
    lastPrint = millis();
    Serial.print("X: "); Serial.print(rawX);
    Serial.print(" | Y: "); Serial.print(rawY);
    Serial.print(" | Locked Gear: "); Serial.println(lastGear == 0 ? "N" : String(lastGear - 8));
  } */

  delay(10);
}
