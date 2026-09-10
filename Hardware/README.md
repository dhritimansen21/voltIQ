# Hardware — VoltIQ

Covers the ESP32-based sensing unit: ZMPT101B voltage sensor, SCT-013 current
clamp, DS3231 RTC, microSD buffering, and (later) the relay/load-cutoff
module and I2C LCD.

## Status (10/9/26)

Sensor hardware is on order and not yet in hand. To avoid blocking downstream
work (data pipeline, ML, dashboard) on physical component delivery, this
folder currently contains:

- `firmware/src/main.cpp` — full ESP32 firmware: RTC timestamping, buffered
  microSD CSV logging, and the sensor-read interface. `readVoltage()` and
  `readCurrent()` are simulated placeholders — everything else (timestamp
  formatting, CSV schema, SD write/flush batching) is the real logic that
  will be used once the sensors are wired.
- `firmware/platformio.ini` — PlatformIO build config (ESP32 dev board,
  Arduino framework, RTClib dependency).
- `mock_data/generate_mock_data.py` — generates a synthetic dataset matching
  the firmware's exact CSV schema (`timestamp,voltage,current,is_anomaly`),
  with injected anomaly windows simulating theft-like current drops, so the
  ML and data-pipeline work can proceed without waiting on real sensor data.
- `mock_data/sample_output/mock_readings.csv` — sample generator output
  (2 days, 1-minute interval).

## Next steps

1. Hardware arrival → wire ZMPT101B + SCT-013, replace simulated reads in
   `main.cpp` with calibrated `analogRead()` values, validate against a
   known load.
2. Wire DS3231 + microSD module per the pin config documented in
   `firmware/src/main.cpp`.
3. Wire relay/load-cutoff module once theft classification logic is ready
   to trigger it.
