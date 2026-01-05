# agent-oxide-pulse

Proof of Concept for Hardware-Level JTAG Debugging on the ESP32.

## 🔌 Hardware Setup

**Required:** ESP32 DevKit V1 + ESP-Prog JTAG Adapter.

| Signal | ESP-Prog | ESP32 |
|:-------|:---------|:------|
| **TDI** | TDI | GPIO 12 |
| **TCK** | TCK | GPIO 13 |
| **TMS** | TMS | GPIO 14 |
| **TDO** | TDO | GPIO 15 |
| **GND** | GND | GND |

**⚠️ Critical:** Connect a common ground!

## 🚀 Quick Start

1.  **Flash Firmware** (PlatformIO/VS Code):
    - `Cmd + Shift + P` -> `PlatformIO: Upload`

2.  **Start Debugging**:
    - Select **"PIO Debug"** (or ESP32 JTAG) in Run & Debug.
    - Press **F5**.

3.  **The Hack**:
    - Pause execution.
    - Debug Console: `set var g_false_positive_detected = 1`
    - Continue.
    - Result: `[TRAP] False Positive Detected!`

### B. "Ghost" Device Permissions
If OpenOCD fails with `libusb_open() failed`, macOS might be blocking the connection.
1. Go to **System Settings > Privacy & Security**.
2. Look for "Allow accessories to connect".
3. Set to "Always" or "Ask for New Accessories".
4. If prompted when plugging in ESP-PROG, click **Allow**.

## 3. Operations Manual (Debugging)

### Phase 1: Build & Flash
1. Open this folder in VS Code with PlatformIO installed.
2. Build the "Debug" Firmware:
   ```bash
   pio run -e esp32doit-devkit-v1
   ```
3. Upload Firmware (via JTAG):
   ```bash
   pio run -t upload
   ```
   *Note: Ensure ESP-PROG is connected. If upload fails, check wiring.*

### Phase 2: The "Trap" (JTAG Session)
We will now catch the code execution in the act of a False Positive.
1. In VS Code, go to the **Run & Debug** Activity Bar (Side Panel).
2. Select **"PIO Debug"** configuration at the top.
3. Press **Green Play Button (Start Debugging)**.
   - The status bar will turn orange. OpenOCD will launch.
   - The code will HALT automatically at `setup()` (as defined in `platformio.ini`).
4. **Set the Trap:**
   - Open `src/main.cpp`.
   - Scroll to the line inside `if (g_false_positive_detected)` around line **92** (`Serial.println("[TRAP]...")`).
   - Click to the left of the line number to **Set Breakpoint** (Red Dot).
5. **Resume Execution:**
   - Press the **Continue** (Play) button in the floating debug toolbar.
   - The code is now running at 50Hz.
6. **Trigger the Trap:**
   - Quickly **TAP** the GPIO 4 pin (Touch 0) with your finger. Do NOT hold it.
   - A quick tap creates a spike (Raw Signal) but usually fails to pull the Moving Average up fast enough.
   - **RESULT:** The debugger should **PAUSE** execution at your breakpoint.
7. **Inspect:**
   - Look at the **VARIABLES** pane on the left.
   - Expand `Global`.
   - Observe `g_signal_strength` (likely > 40) vs `g_filtered_signal` (likely < 40).
   - You have strictly proven a noise artifact occurred.

### Phase 3: The "Manual Trap" (Button Interrupt)
**New Feature:** You can intentionally break the code at any time using the hardware.
1. Ensure the debugger is running (Play button).
2. Press the **BOOT** button on the ESP32 DevKit.
3. The code will immediately **HALT** inside the `loop()` function.
4. The Serial Monitor will print: `[DEBUG] Manual Interrupt Triggered via BOOT Button.`
5. This confirms your interrupt service routines (ISRs) and JTAG control are working perfectly in tandem.

## 4. Simulation
Open `simulation.html` in any browser to visualize the algorithm logic without hardware strings attached.
