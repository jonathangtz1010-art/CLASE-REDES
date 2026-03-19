#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import signal
import time
import threading
import serial

HOST = "0.0.0.0"
PORT = 5001

BAUD = 115200
SERIAL_PORT = "/dev/ttyACM0"

_running = True
_lock = threading.Lock()

def handle_sig(*_):
    global _running
    _running = False

def open_serial():
    ser = serial.Serial(SERIAL_PORT, BAUD, timeout=2)
    time.sleep(2.0)
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    print(f"[SERIAL] Conectado a {SERIAL_PORT} @ {BAUD}")
    return ser

def read_line(ser):
    return ser.readline().decode("utf-8", errors="ignore").strip()

def send_to_arduino(ser, cmd):
    ser.write((cmd.strip() + "\n").encode("utf-8"))
    ser.flush()
    resp = read_line(ser)
    return resp if resp else "ERR sin_respuesta_del_arduino"

def normalize_command(msg):
    msg = msg.strip().upper()

    if msg in ("", "GET", "READ", "STATUS"):
        return "STATUS"

    if msg == "PING":
        return "PING"

    return "STATUS"

def main():
    global _running
    signal.signal(signal.SIGINT, handle_sig)
    signal.signal(signal.SIGTERM, handle_sig)

    ser = open_serial()

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((HOST, PORT))
        s.listen(5)
        s.settimeout(0.5)

        print(f"[TCP] Escuchando en {HOST}:{PORT}")

        while _running:
            try:
                conn, _ = s.accept()
            except socket.timeout:
                continue

            with conn:
                conn.settimeout(1.0)
                data = b""

                try:
                    while True:
                        chunk = conn.recv(1024)
                        if not chunk:
                            break
                        data += chunk
                        if b"\n" in data:
                            break
                except socket.timeout:
                    pass

                msg = data.decode("utf-8", errors="ignore").strip()
                cmd = normalize_command(msg)

                try:
                    with _lock:
                        resp = send_to_arduino(ser, cmd)
                    conn.sendall((resp + "\n").encode("utf-8"))
                except Exception as e:
                    conn.sendall((f"ERR {e}\n").encode("utf-8"))

    try:
        ser.close()
    except Exception:
        pass

    print("Cerrado limpio.")

if __name__ == "__main__":
    main()
