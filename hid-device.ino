#include <Arduino.h>
#include <Joystick.h>

// Logitech g923 pedals and shifter as a custom HID joystick device
// with additional 4 buttons and 2 (on)-off-(on) toggle switches.

// --- CONFIG ---
const bool USE_INTERNAL_PULLUP_FOR_REV = false; // set false if your reverse switch uses external pull resistors

// --- PIN DEFINITIONS ---
const int PIN_REV       = 2;
const int PIN_SHIFTER_X = A0;
const int PIN_SHIFTER_Y = A1;

const int PIN_GAS       = A2;
const int PIN_BRAKE     = A3;
const int PIN_CLUTCH    = A6; // ensure your Pro Micro variant exposes A6

const int BUTTON_PINS[4] = {3, 5, 6, 7};   // standalone buttons -> USB 1..4
const int TOGGLE_PINS[4] = {8, 9, 10, 16}; // toggles -> USB 5..8

// --- DEBOUNCE TRACKING VARIABLES ---
static int lastGear = 0;               // 0 = Neutral, 9-14 = Gears 1-6 (button indices)
static unsigned long lastChangeTime = 0;
static bool gearChangePending = false;
const unsigned long debounceDelay = 50; // ms

// --- HID TIMING ---
const unsigned long HID_INTERVAL = 8; // ms; try 8 for low latency, 10-16 for lower USB/CPU load
unsigned long lastHid = 0;

// --- JOYSTICK INITIALIZATION ---
// Arguments: Report ID, Type, Button Count, Hat Count,
// X, Y, Z, Rx, Ry, Rz, Rudder, Throttle, Accelerator, Brake, Steering
Joystick_ SimRig(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  15, 0,                  // 15 Buttons allocated (indices 0..14), 0 Hat switches
  true, true, true,       // X (Shifter X), Y (Shifter Y), Z (Gas)
  true, true, false,      // Rx (Brake), Ry (Clutch), Rz disabled
  false, false,           // Rudder, Throttle disabled
  false, false,           // Specialized Accelerator/Brake disabled
  false                   // Steering disabled
);

void setup() {

  if (USE_INTERNAL_PULLUP_FOR_REV) pinMode(PIN_REV, INPUT_PULLUP);
  else pinMode(PIN_REV, INPUT);

  for (int i = 0; i < 4; i++) { pinMode(BUTTON_PINS[i], INPUT_PULLUP); }
  for (int i = 0; i < 4; i++) { pinMode(TOGGLE_PINS[i], INPUT_PULLUP); }

  SimRig.begin();
  lastHid = millis();
  lastChangeTime = millis();
}

void loop() {
  // Read raw inputs
  int rawX = analogRead(PIN_SHIFTER_X);
  int rawY = analogRead(PIN_SHIFTER_Y);
  int gas   = analogRead(PIN_GAS);
  int brake = analogRead(PIN_BRAKE);
  int clutch = analogRead(PIN_CLUTCH);

  // Determine instantaneous gear zone
  int currentGear = 0; // Neutral

  // PRIORITIZE REVERSE: if reverse switch is active, force Neutral for H-pattern
  bool revPressed = (USE_INTERNAL_PULLUP_FOR_REV) ? (digitalRead(PIN_REV) == LOW) : (digitalRead(PIN_REV) == HIGH);
  if (revPressed) {
    currentGear = 0;
  } else {
    if (rawX < 270) {          // LEFT GATE
      if (rawY > 700)      currentGear = 9;  // 1st
      else if (rawY < 300) currentGear = 10; // 2nd
    }
    else if (rawX > 560) {     // RIGHT GATE
      if (rawY > 700)      currentGear = 13; // 5th
      else if (rawY < 300) currentGear = 14; // 6th
    }
    else {                     // CENTER GATE
      if (rawY > 700)      currentGear = 11; // 3rd
      else if (rawY < 300) currentGear = 12; // 4th
    }
  }

  // Debounced H-pattern logic
  if (currentGear != lastGear) {
    if (!gearChangePending) {
      gearChangePending = true;
      lastChangeTime = millis();
    } else {
      if (millis() - lastChangeTime >= debounceDelay) {
        // Turn off previous gear button (if it was a gear)
        if (lastGear >= 9 && lastGear <= 14) SimRig.setButton(lastGear, false);
        // Turn on new gear button (if it is a gear)
        if (currentGear >= 9 && currentGear <= 14) SimRig.setButton(currentGear, true);
        lastGear = currentGear;
        gearChangePending = false;
        lastChangeTime = millis();
      }
    }
  } else {
    gearChangePending = false;
    lastChangeTime = millis();
  }

  // Update other button states (internal state only; will be sent on HID packet)
  // Reverse button (button index 0)
  SimRig.setButton(0, revPressed);

  // Standalone buttons (indices 1..4)
  for (int i = 0; i < 4; i++) {
    SimRig.setButton(i + 1, !digitalRead(BUTTON_PINS[i])); // active low -> USB 1..4
  }

  // Toggle switches (indices 5..8)
  // NOTE: fixed loop bounds and offset so toggles map to USB 5..8 and do not overlap other buttons
  for (int i = 0; i < 4; i++) {
    SimRig.setButton(i + 5, !digitalRead(TOGGLE_PINS[i])); // active low -> USB 5..8
  }

  // HID send block: update axes and send a single coherent report at HID_INTERVAL
  unsigned long now = millis();
  if (now - lastHid >= HID_INTERVAL) {
    SimRig.setXAxis(rawX);
    SimRig.setYAxis(rawY);
    SimRig.setZAxis(gas);
    SimRig.setRxAxis(brake);
    SimRig.setRyAxis(clutch);

    // Send ONE single clean USB packet
    SimRig.sendState();

    lastHid = now;
  }

}
