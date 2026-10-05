#!/usr/bin/env python3
"""
Bring-up helper for M1: logs telemetry to CSV and (optionally) sweeps the duty
to find the dead-band.  Requires:  pip install pyserial

    python bench.py COM5 --log m1.csv                      # just log
    python bench.py COM5 --sweep --max-duty 250 --step 10  # wheel LIFTED, current-limited supply!

NOT yet tested on hardware: read it before running.
"""
import argparse
import csv
import time

import serial

FIELDS = ["tick_ms", "state", "faults", "counts", "delta", "angle_x100",
          "rpm_x100", "duty", "exec_us", "jit_us", "warn", "cpr_x100"]


def send(ser, text):
    ser.write((text + "\r\n").encode())


def read_lines(ser):
    while True:
        raw = ser.readline()
        if not raw:
            return
        yield raw.decode(errors="ignore").strip()


def parse(line):
    if not line.startswith("T,"):
        return None
    parts = line.split(",")[1:]
    if len(parts) != len(FIELDS):
        return None
    try:
        return dict(zip(FIELDS, (int(p) for p in parts)))
    except ValueError:
        return None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("port")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--log", default="m1_log.csv")
    ap.add_argument("--sweep", action="store_true")
    ap.add_argument("--max-duty", type=int, default=250)
    ap.add_argument("--step", type=int, default=10)
    ap.add_argument("--dwell", type=float, default=1.0, help="seconds per step")
    ap.add_argument("--rpm-threshold", type=float, default=1.0)
    a = ap.parse_args()

    ser = serial.Serial(a.port, a.baud, timeout=0.05)
    deadband = None
    with open(a.log, "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=FIELDS + ["cmd_duty"])
        w.writeheader()
        send(ser, "tel 1")
        cmd_duty = 0
        try:
            if a.sweep:
                send(ser, "tmo 3000")
                send(ser, "arm")
                time.sleep(0.3)
            steps = range(a.step, a.max_duty + 1, a.step) if a.sweep else [None]
            for duty in steps:
                t_end = time.time() + (a.dwell if a.sweep else 1e9)
                last_cmd = 0.0
                while time.time() < t_end:
                    if a.sweep and time.time() - last_cmd > 0.2:   # keep-alive
                        cmd_duty = duty
                        send(ser, f"d {duty}")
                        last_cmd = time.time()
                    for line in read_lines(ser):
                        rec = parse(line)
                        if rec:
                            rec["cmd_duty"] = cmd_duty
                            w.writerow(rec)
                            if a.sweep and deadband is None and abs(rec["rpm_x100"]) / 100.0 > a.rpm_threshold:
                                deadband = duty
        except KeyboardInterrupt:
            pass
        finally:
            send(ser, "d 0")
            send(ser, "disarm")
            send(ser, "tel 0")
            ser.close()
    if a.sweep:
        print(f"dead-band estimate: {deadband} permille" if deadband else "motor never moved: check wiring/limits")


if __name__ == "__main__":
    main()
