"""
VoltIQ — Mock Sensor Data Generator
-----------------------------------------------------------------
Generates a synthetic CSV dataset matching the exact schema the
ESP32 firmware will produce (timestamp, voltage, current), so the
ML/forecasting and data-pipeline work can proceed while we wait on
hardware delivery.

Includes simulated theft/anomaly events (sudden current drops or
irregular spikes) so anomaly-detection and theft-classification
work has something realistic to train against early.

Usage:
    python generate_mock_data.py --days 7 --interval 60 --out mock_readings.csv
"""

import argparse
import csv
import random
from datetime import datetime, timedelta


def generate_readings(start_time: datetime, num_samples: int, interval_seconds: int):
    """Yield (timestamp, voltage, current, is_anomaly) tuples."""
    voltage_base = 230.0
    current_base = 3.0

    # Pre-pick a few anomaly windows to simulate theft/fault events
    anomaly_windows = set(
        random.sample(range(num_samples), k=max(1, num_samples // 200))
    )

    t = start_time
    for i in range(num_samples):
        voltage = voltage_base + random.uniform(-4, 4)

        is_anomaly = i in anomaly_windows
        if is_anomaly:
            # Simulate a theft-like pattern: current drops despite normal voltage
            # (load appears disconnected from the meter's perspective)
            current = max(0.0, current_base * random.uniform(0.05, 0.2))
        else:
            current = current_base + random.uniform(-0.3, 0.3)

        yield t.strftime("%Y-%m-%d %H:%M:%S"), round(voltage, 2), round(current, 2), int(is_anomaly)
        t += timedelta(seconds=interval_seconds)


def main():
    parser = argparse.ArgumentParser(description="Generate mock VoltIQ sensor data.")
    parser.add_argument("--days", type=int, default=7, help="Number of days of data to simulate")
    parser.add_argument("--interval", type=int, default=60, help="Seconds between samples")
    parser.add_argument("--out", type=str, default="data/mock_readings.csv", help="Output CSV path")
    args = parser.parse_args()

    num_samples = (args.days * 24 * 60 * 60) // args.interval
    start_time = datetime.now() - timedelta(days=args.days)

    with open(args.out, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["timestamp", "voltage", "current", "is_anomaly"])
        for row in generate_readings(start_time, num_samples, args.interval):
            writer.writerow(row)

    print(f"Wrote {num_samples} samples to {args.out}")


if __name__ == "__main__":
    main()
