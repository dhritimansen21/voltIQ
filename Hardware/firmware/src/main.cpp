/*
 * VoltIQ — ESP32 Firmware Skeleton
 * -----------------------------------------------------------------
 * Sensor hardware (ZMPT101B voltage sensor, SCT-013 current clamp)
 * has not arrived yet. This skeleton implements the full data path
 * — RTC timestamping, buffered sensor read, and SD logging — using
 * simulated ADC values in place of real sensor input.
 *
 * Once hardware arrives, only readVoltage() and readCurrent() need
 * to be swapped from simulation to real analogRead() + calibration
 * math. Everything else (timestamping, CSV formatting, SD writes,
 * flush interval) is already the real logic.
 *
 * Libraries required (add to platformio.ini):
 *   - RTClib (Adafruit)  -> DS3231 RTC
 *   - SD (bundled with ESP32 Arduino core) -> microSD logging
 */

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

// ---- Pin configuration -------------------------------------------------
#define SD_CS_PIN        5     // microSD module chip-select
#define VOLTAGE_ADC_PIN  34    // reserved for ZMPT101B once wired
#define CURRENT_ADC_PIN  35    // reserved for SCT-013 once wired

// ---- Globals ------------------------------------------------------------
RTC_DS3231 rtc;
File logFile;

const char* LOG_FILENAME   = "/log.csv";
const unsigned long SAMPLE_INTERVAL_MS = 1000;   // one reading per second
const uint8_t FLUSH_EVERY_N_SAMPLES    = 10;      // batch writes to protect SD card

unsigned long lastSampleTime = 0;
uint8_t samplesSinceFlush = 0;

// ---- Simulated sensor reads (placeholder until hardware arrives) --------
// TODO: replace with real analogRead(VOLTAGE_ADC_PIN) + calibration formula
float readVoltage() {
  // Simulate mains voltage hovering around 230V with small noise
  return 225.0 + random(-50, 50) / 10.0;
}

// TODO: replace with real analogRead(CURRENT_ADC_PIN) + calibration formula
float readCurrent() {
  // Simulate a load current with occasional simulated spikes (anomaly stand-in)
  bool spike = random(0, 100) < 3; // ~3% chance of a spike sample
  float base = 2.0 + random(-20, 20) / 100.0;
  return spike ? base + random(5, 15) : base;
}

// ---- Setup ---------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("VoltIQ firmware starting...");

  // RTC init
  if (!rtc.begin()) {
    Serial.println("ERROR: RTC not found. Check wiring.");
  } else if (rtc.lostPower()) {
    Serial.println("RTC lost power, resetting to compile time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // SD init
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("ERROR: SD card init failed. Check wiring/card.");
  } else {
    logFile = SD.open(LOG_FILENAME, FILE_APPEND);
    if (!logFile) {
      Serial.println("ERROR: could not open log file.");
    } else if (logFile.size() == 0) {
      // Write CSV header only if the file is new
      logFile.println("timestamp,voltage,current");
      logFile.flush();
    }
  }

  randomSeed(analogRead(0)); // seed for simulated readings
}

// ---- Main loop -------------------------------------------------------
void loop() {
  unsigned long now = millis();
  if (now - lastSampleTime < SAMPLE_INTERVAL_MS) {
    return;
  }
  lastSampleTime = now;

  DateTime timestamp = rtc.now();
  float voltage = readVoltage();
  float current = readCurrent();

  char timeBuf[20];
  snprintf(timeBuf, sizeof(timeBuf), "%04d-%02d-%02d %02d:%02d:%02d",
           timestamp.year(), timestamp.month(), timestamp.day(),
           timestamp.hour(), timestamp.minute(), timestamp.second());

  Serial.printf("[%s] V=%.2f I=%.2f\n", timeBuf, voltage, current);

  if (logFile) {
    logFile.printf("%s,%.2f,%.2f\n", timeBuf, voltage, current);
    samplesSinceFlush++;

    if (samplesSinceFlush >= FLUSH_EVERY_N_SAMPLES) {
      logFile.flush();
      samplesSinceFlush = 0;
    }
  }
}
