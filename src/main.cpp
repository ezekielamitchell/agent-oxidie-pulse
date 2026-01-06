/**
 * @author Ezekiel A. Mitchell
 * @date 2026-01-06
 * @version 0.1
 * @file main.cpp
 * @brief agent-oxide-pulse: JTAG Debug Proof of Concept
 *
 * Target: ESP32 DevKit V1 | Sensor: GPIO 4 (Touch) | Debug: ESP-PROG
 */

#include <Arduino.h>

// Hardware & Config
const int PIN_TOUCH_SENSOR = 4;
const int POLL_RATE_HZ = 50;
const int POLL_DELAY_MS = 1000 / POLL_RATE_HZ;

// Signal Processing
const int TOUCH_BASELINE = 75;
const int THRESHOLD_TRIGGER = 40;
const int BUFFER_SIZE = 10;

// Global State (Volatile for JTAG visibility)
volatile int g_raw_reading = 0;
volatile int g_signal_strength = 0;
volatile int g_filtered_signal = 0;
volatile bool g_false_positive_detected = false; // <-- TRAP TARGET

// Data Buffers
int g_buffer[BUFFER_SIZE];
int g_buffer_index = 0;
volatile bool g_debug_interrupt_flag = false;

// ISR for manual debug break
void IRAM_ATTR handleDebugButtonData() { g_debug_interrupt_flag = true; }

int processMovingAverage(int new_sample) {
  g_buffer[g_buffer_index] = new_sample;
  g_buffer_index = (g_buffer_index + 1) % BUFFER_SIZE;

  long sum = 0;
  for (int i = 0; i < BUFFER_SIZE; i++)
    sum += g_buffer[i];
  return (int)(sum / BUFFER_SIZE);
}

void setup() {
  Serial.begin(115200);
  memset(g_buffer, 0, sizeof(g_buffer));

  Serial.println("[SYSTEM] agent-oxide-pulse Ready.");

  pinMode(0, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(0), handleDebugButtonData, FALLING);
}

void loop() {
  // 1. Acquire & Normalize
  g_raw_reading = touchRead(PIN_TOUCH_SENSOR);
  int diff = max(0, TOUCH_BASELINE - g_raw_reading);
  g_signal_strength = map(diff, 0, TOUCH_BASELINE, 0, 100);

  // 2. Filter
  g_filtered_signal = processMovingAverage((int)g_signal_strength);

  // 3. Logic Trap
  // Goal: Manually set g_false_positive_detected = 1 via debugger to trigger
  // this block.
  if (g_signal_strength > THRESHOLD_TRIGGER &&
      g_filtered_signal < THRESHOLD_TRIGGER) {
    g_false_positive_detected = true;
    Serial.println("[TRAP] False Positive Detected!");
  } else {
    g_false_positive_detected = false;
    if (g_filtered_signal > THRESHOLD_TRIGGER) {
      Serial.printf("Valid Target: Strength %d\n", g_filtered_signal);
    }
  }

  // Telemetry
  Serial.printf("S:%d F:%d\n", g_signal_strength, g_filtered_signal);

  // Manual Break
  if (g_debug_interrupt_flag) {
    g_debug_interrupt_flag = false;
    Serial.println("[DEBUG] Break triggered.");
    asm("break.n 1");
  }

  delay(POLL_DELAY_MS);
}
