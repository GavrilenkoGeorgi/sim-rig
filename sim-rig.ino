// --- PIN CONFIGURATION ---
// Pedals (Analog Inputs)
const int PIN_GAS    = A2;
const int PIN_BRAKE  = A3;
const int PIN_CLUTCH = A6;

// Shifter Inputs
const int PIN_REVERSE   = 2;  // Digital input for reverse microswitch
const int PIN_SHIFTER_X = A0; // Analog input for side-to-side gate
const int PIN_SHIFTER_Y = A1; // Analog input for up-and-down gear rows (Arduino Digital 4 is A6)

// Custom Dashboard Buttons
const int PIN_BTN1 = 3;
const int PIN_BTN2 = 5;
const int PIN_BTN3 = 6;
const int PIN_BTN4 = 7;

// E-TEN Toggle Switch
const int PIN_TOGGLE_UP   = 9;
const int PIN_TOGGLE_DOWN = 8;
const int PIN_TOGGLE_2_UP   = 10;
const int PIN_TOGGLE_2_DOWN = 16;

void setup() {
  // Initialize USB serial communication at 9600 bits per second
  Serial.begin(9600);
  
  // This line forces the Pro Micro to wait until the Serial Monitor window 
  // is opened on your PC before running the code.
  while (!Serial) {
    ;
  }
  
  Serial.println("========================================");
  Serial.println("  SIM RIG INTERFACE - INITIALIZED OK!   ");
  Serial.println("========================================");

  // Configure digital pins to use internal pull-up resistors.
  // This removes the need for physical resistors on your perfboard!
  pinMode(PIN_REVERSE, INPUT_PULLUP);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);
  pinMode(PIN_BTN3, INPUT_PULLUP);
  pinMode(PIN_BTN4, INPUT_PULLUP);
  pinMode(PIN_TOGGLE_UP, INPUT_PULLUP);
  pinMode(PIN_TOGGLE_DOWN, INPUT_PULLUP);
  pinMode(PIN_TOGGLE_2_UP, INPUT_PULLUP);
  pinMode(PIN_TOGGLE_2_DOWN, INPUT_PULLUP);
}

void loop() {
  // 1. Read Analog Sensors (Values will range from 0 to 1023)
  int gasValue     = analogRead(PIN_GAS);
  int brakeValue   = analogRead(PIN_BRAKE);
  int clutchValue  = analogRead(PIN_CLUTCH);
  int shifterX     = analogRead(PIN_SHIFTER_X);
  int shifterY     = analogRead(PIN_SHIFTER_Y);

  // 2. Read Digital Inputs
  // Because we use INPUT_PULLUP, the pin reads HIGH at rest.
  // When you press a button to bridge it to Ground, the state drops to LOW.


  bool revPressed    = (digitalRead(PIN_REVERSE) == HIGH);
  
  bool btn1Pressed   = (digitalRead(PIN_BTN1) == LOW);
  bool btn2Pressed   = (digitalRead(PIN_BTN2) == LOW);
  bool btn3Pressed   = (digitalRead(PIN_BTN3) == LOW);
  bool btn4Pressed   = (digitalRead(PIN_BTN4) == LOW);
  bool toggleUp      = (digitalRead(PIN_TOGGLE_UP) == LOW);
  bool toggleDown    = (digitalRead(PIN_TOGGLE_DOWN) == LOW);
  bool toggle2Up     = (digitalRead(PIN_TOGGLE_2_UP) == LOW);
  bool toggle2Down   = (digitalRead(PIN_TOGGLE_2_DOWN) == LOW);

  // 3. Print out results to the Serial Monitor screen
  Serial.println("\n--- LIVE HARDWARE TELEMETRY ---");
  
  // Pedals data
  Serial.print("PEDALS  -> Gas: "); Serial.print(gasValue);
  Serial.print(" | Brake: "); Serial.print(brakeValue);
  Serial.print(" | Clutch: "); Serial.println(clutchValue);

  // Shifter data
  Serial.print("SHIFTER -> X-Axis: "); Serial.print(shifterX);
  Serial.print(" | Y-Axis: "); Serial.print(shifterY);
  Serial.print(" | Reverse: "); Serial.println(revPressed ? "[ENGAGED]" : "[OFF]");

  // Dashboard buttons data
  Serial.print("BUTTONS -> B1: "); Serial.print(btn1Pressed ? "[X]" : "[ ]");
  Serial.print(" | B2: "); Serial.print(btn2Pressed ? "[X]" : "[ ]");
  Serial.print(" | B3: "); Serial.print(btn3Pressed ? "[X]" : "[ ]");
  Serial.print(" | B4: "); Serial.println(btn4Pressed ? "[X]" : "[ ]");

  // Toggle switch data
  Serial.print("TOGGLE  -> Up (Wipers): "); Serial.print(toggleUp ? "[ON]" : "[OFF]");
  Serial.print(" | Down (Lights): "); Serial.print(toggleDown ? "[ON]" : "[OFF]");
  Serial.print(" | 2nd Up: "); Serial.print(toggle2Up ? "[ON]" : "[OFF]");
  Serial.print(" | 2nd Down: "); Serial.println(toggle2Down ? "[ON]" : "[OFF]");
  
  Serial.println("--------------------------------------------------");
  
  // Pause for 500 milliseconds so the text doesn't scroll blindingly fast
  delay(500);
}
