# DIY USB Sim Shifter & Pedal Interface

A lightweight, high-performance Arduino sketch that converts a standard analog H-pattern shifter (such as a Logitech shifter) and standalone pedals into a native, plug-and-play USB HID gaming controller.

This project allows you to run your legacy or standalone shifter and pedals completely independently alongside high-end direct-drive ecosystems (like Simagic wheelbases) without needing to route them through their original proprietary wheelbases.

---

## Features

- **Native USB HID Support:** Recognizes automatically as a standard game controller in Windows via the ATmega32U4 architecture—no external drivers or emulation software required.
- **H-Pattern Coordinate Mapping:** Translates 2D analog X/Y joystick coordinates into 6 distinct, stable virtual button inputs for gears 1 through 6.
- **Advanced Software Debouncing:** Employs a temporal state-machine filter (50ms window) to completely eliminate electrical noise and button flickering at gate thresholds.
- **Reverse Gear Absolute Priority:** Software-enforced isolation that prioritizes the dedicated reverse microswitch, blocking overlapping analog gear registers when slamming into reverse.
- **Triple Axis Inputs:** Direct 10-bit resolution tracking for independent Gas, Brake, and Clutch pedal potentiometers.

---

## Prerequisites

### Hardware

- **Microcontroller:** Arduino Pro Micro (or any ATmega32U4-based board with native USB capability).
- **Peripherals:** Analog H-pattern shifter module and/or 3-axis pedal potentiometers.

### Software & Libraries

1. **Arduino IDE** (v2.0 or higher recommended).
2. **Arduino Joystick Library (v2.0)** by Matthew Heironimus.
   - Download the ZIP from GitHub and install it via `Sketch` -> `Include Library` -> `Add .ZIP Library...` in the IDE.

---

## Master Hardware Pinout

To map your physical wiring to this codebase, use the following configuration layout:

| Peripheral Component            | Arduino Pin         | Input Type / Notes                                   |
| :------------------------------ | :------------------ | :--------------------------------------------------- |
| **Reverse Gear Switch**         | `Pin 2`             | Digital Input (Requires physical Pull-Down resistor) |
| **Shifter X-Axis (Left/Right)** | `Pin A0`            | Analog Input                                         |
| **Shifter Y-Axis (Up/Down)**    | `Pin A1`            | Analog Input                                         |
| **Gas Pedal**                   | `Pin A2`            | Analog Input                                         |
| **Brake Pedal**                 | `Pin A3`            | Analog Input                                         |
| **Clutch Pedal**                | `Pin A6`            | Analog Input (Physical Pin 4 on Pro Micro board)     |
| **Standalone Buttons (1-4)**    | `Pins 3, 5, 6, 7`   | Digital Input (`INPUT_PULLUP` - Connect to GND)      |
| **Toggle Switches (1-2)**       | `Pins 8, 9, 10, 16` | Digital Input (`INPUT_PULLUP` - Connect to GND)      |

---

## Verification in Windows

Once uploaded, the board will immediately initialize as a standard gaming device. Press Win + R, type joy.cpl, and press Enter. Select your device from the list and click Properties.

Verify that your pedal axes sweep smoothly and your gear shifts confidently activate buttons 10 through 15 without flickering.

Open your simulation software of choice (e.g., Assetto Corsa, iRacing) and map the controls using the game's standard in-game calibration settings.

Calibration Tip: If your physical shifter's gate boundaries vary slightly from standard values, you can easily fine-tune the raw coordinate ranges by modifying the threshold numbers inside Section 5 of the void loop().

## Project Links & Schematics

Full Hardware Wiring Schematics: Interactive circuit layouts and diagrams can be found on [Cirkit Designer Project Page](https://app.cirkitdesigner.com/project/0f07b352-7ed3-4124-ac75-4cc197a48ab5).

Detailed Build & Debugging Log: For an in-depth breakdown of the build concept, physical materials used, and the step-by-step electrical troubleshooting process, check out the full article in my [blog post](https://jsmonkey.netlify.app/blog/diy-shifter-interface).
