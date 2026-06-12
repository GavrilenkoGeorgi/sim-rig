#include <Arduino.h>
#include <Joystick.h>

// Logitech g923 pedals as a custom HID joystick device
// with X axis for Rudder (clutch and accelerator), brake pedal,
// 4 buttons and 2 (on)-off-(on) toggle switches.

// Define your analog pins
const int ACCEL_PIN = A2;
const int BRAKE_PIN = A3;
const int CLUTCH_PIN = A6;

// Buttons and Toggles
const int BUTTON_PINS[4] = {3, 5, 6, 7};
const int TOGGLE_PINS[4] = {8, 9, 10, 16};

// HID timing (tune for latency vs USB load)
const unsigned long HID_INTERVAL = 8; // ms; try 8 for low latency, 10-16 if you need stability
unsigned long lastHid = 0;

// Custom HID Joystick configuration
Joystick_ FlightPedals(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_JOYSTICK,
  8, 0,                   // 8 Buttons allocated, 0 Hat switches
  true, false, false,     // ENABLE X Axis (Rudder) and disable internal Y and Z axes
  false, false, false,    // Disable Rx, Ry, and Rz Rotation
  false,                  // Disable Simulation Rudder
  false,                  // Disable Throttle
  false,                  // Disable Accelerator
  true,                   // ENABLE Simulation Brake
  false                   // Disable Steering
);

// Optional: small smoothing factor to reduce jitter (0 = no smoothing, 1..255 small smoothing)
const uint8_t SMOOTHING = 0; // set to 0 to disable; try 4 or 8 if you see jitter
int smoothAnalog(int prev, int raw) {
  if (SMOOTHING == 0) return raw;
  // simple exponential moving average (integer)
  return prev + ((raw - prev) >> SMOOTHING);
}

void setup() {

  pinMode(CLUTCH_PIN, INPUT);
  pinMode(BRAKE_PIN, INPUT);
  pinMode(ACCEL_PIN, INPUT);

  for (int i = 0; i < 4; i++) { pinMode(BUTTON_PINS[i], INPUT_PULLUP); }
  for (int i = 0; i < 4; i++) { pinMode(TOGGLE_PINS[i], INPUT_PULLUP); }

  // Set ranges explicitly for X and Brake descriptors
  FlightPedals.setXAxisRange(0, 1023);
  FlightPedals.setBrakeRange(0, 1023);

  // Start USB HID emulation in manual packet mode (false = manual)
  FlightPedals.begin(false);

  lastHid = millis();
}

void loop() {
  // Read raw voltage values from the potentiometers
  static int prevClutch = 0;
  static int prevBrake = 0;
  static int prevAccel = 0;

  int clutchRaw = analogRead(CLUTCH_PIN);
  int brakeRaw  = analogRead(BRAKE_PIN);
  int accelRaw  = analogRead(ACCEL_PIN);

  int clutchValue = smoothAnalog(prevClutch, clutchRaw);
  int brakeValue  = smoothAnalog(prevBrake, brakeRaw);
  int accelValue  = smoothAnalog(prevAccel, accelRaw);

  prevClutch = clutchValue;
  prevBrake  = brakeValue;
  prevAccel  = accelValue;

  // 2. STANDALONE BUTTONS (Mapped to USB Buttons 0-3)
  for (int i = 0; i < 4; i++) {
    FlightPedals.setButton(i, !digitalRead(BUTTON_PINS[i])); // active low
  }

  // 3. TOGGLE SWITCHES (Mapped to USB Buttons 4-7)
  for (int i = 0; i < 4; i++) {
    FlightPedals.setButton(i + 4, !digitalRead(TOGGLE_PINS[i])); // active low
  }

  // 4. Process the combined Rudder Axis
  // accelValue and clutchValue are 0..1023, so delta is -1023..1023
  int rudderDelta = accelValue - clutchValue;
  // Map delta to 0..1023 for X axis
  int finalRudder = map(rudderDelta, -1023, 1023, 0, 1023);
  // Clamp just in case
  finalRudder = constrain(finalRudder, 0, 1023);

  // HID send block: update axes/buttons and send a single coherent report at HID_INTERVAL
  unsigned long now = millis();
  if (now - lastHid >= HID_INTERVAL) {
    // 5. Assign states to the clean generic descriptors
    FlightPedals.setXAxis(finalRudder); // X Axis now handles your Rudder
    FlightPedals.setBrake(brakeValue);  // Brake pedal as itself

    // 6. Send ONE single clean USB packet
    FlightPedals.sendState();

    lastHid = now;
  }

  // Non-blocking loop continues; other tasks can be added here
}
