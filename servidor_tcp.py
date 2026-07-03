#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import socket
import signal
import time
import threading
import serial
import json
import re

HOST = "0.0.0.0"
PORT = 5001

BAUD = 9600
SERIAL_PORT = "/dev/ttyACM0"

_running = True
_lock = threading.Lock()

estado_sistema = {
    "ok": True,
    "estado": "Esperando datos del Arduino",
    "color": "DESCONOCIDO",
    "posicion": 0,
    "setpoint": 0,
    "pwm": 0,
    "motor": "DETENIDO",
    "servo": "REPOSO",
    "meta": 30,
    "contadores": {
        "rojo": 0,
        "verde": 0,
        "azul": 0,
        "negro": 0,
        "total": 0
    }
}


def handle_sig(*_):
    global _running
    _running = False


def open_serial():
    ser = serial.Serial(SERIAL_PORT, BAUD, timeout=1)
    time.sleep(2.0)
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    print(f"[SERIAL] Conectado a {SERIAL_PORT} @ {BAUD}")
    return ser


def normalizar_color(color):
    color = color.strip().upper()

    if color == "ROJO":
        return "ROJO"

    if color == "VERDE":
        return "VERDE"

    if color == "AZUL":
        return "AZUL"

    if color == "NEGRO":
        return "NEGRO"

    if color == "SIN PIEZA":
        return "SIN_PIEZA"

    return "DESCONOCIDO"


def sumar_color(color):
    color = normalizar_color(color)

    if color == "ROJO":
        estado_sistema["contadores"]["rojo"] += 1

    elif color == "VERDE":
        estado_sistema["contadores"]["verde"] += 1

    elif color == "AZUL":
        estado_sistema["contadores"]["azul"] += 1

    elif color == "NEGRO":
        estado_sistema["contadores"]["negro"] += 1

    total = (
        estado_sistema["contadores"]["rojo"] +
        estado_sistema["contadores"]["verde"] +
        estado_sistema["contadores"]["azul"] +
        estado_sistema["contadores"]["negro"]
    )

    estado_sistema["contadores"]["total"] = total


def limpiar_contadores():
    estado_sistema["contadores"]["rojo"] = 0
    estado_sistema["contadores"]["verde"] = 0
    estado_sistema["contadores"]["azul"] = 0
    estado_sistema["contadores"]["negro"] = 0
    estado_sistema["contadores"]["total"] = 0


def reiniciar_estado():
    estado_sistema["estado"] = "Sistema reiniciado desde cero"
    estado_sistema["color"] = "DESCONOCIDO"
    estado_sistema["posicion"] = 0
    estado_sistema["setpoint"] = 0
    estado_sistema["pwm"] = 0
    estado_sistema["motor"] = "DETENIDO"
    estado_sistema["servo"] = "REPOSO"
    limpiar_contadores()


def procesar_linea_arduino(linea):
    linea = linea.strip()

    if not linea:
        return

    print(f"[ARDUINO] {linea}")

    with _lock:

        if "CLASIFICADOR AUTOMATICO INICIADO" in linea:
            estado_sistema["estado"] = "Clasificador iniciado"
            estado_sistema["motor"] = "DETENIDO"
            estado_sistema["servo"] = "REPOSO"
            return

        if "FUNCIONAMIENTO CONTINUO ACTIVADO" in linea:
            estado_sistema["estado"] = "Funcionamiento continuo activado"
            return

        if linea.startswith("R:") and "COLOR:" in linea:
            partes = linea.split("COLOR:")

            if len(partes) > 1:
                color = normalizar_color(partes[1])
                estado_sistema["color"] = color

                if color == "SIN_PIEZA":
                    estado_sistema["estado"] = "Esperando pieza"

                elif color == "DESCONOCIDO":
                    estado_sistema["estado"] = "Color desconocido"

                else:
                    estado_sistema["estado"] = f"Detectando color {color}"

            return

        if "Color confirmado:" in linea:
            color = linea.split("Color confirmado:")[-1].strip()
            color = normalizar_color(color)

            estado_sistema["color"] = color
            estado_sistema["estado"] = f"Color confirmado: {color}"

            sumar_color(color)
            return

        if "Esperando" in linea and "acomodar la pieza" in linea:
            estado_sistema["estado"] = "Esperando para acomodar pieza"
            estado_sistema["motor"] = "DETENIDO"
            return

        if "Moviendo pieza" in linea:
            estado_sistema["estado"] = linea
            estado_sistema["motor"] = "MOVIENDO"
            return

        if linea.startswith("Destino:"):
            valor = linea.replace("Destino:", "").strip()

            try:
                estado_sistema["setpoint"] = int(valor)
            except Exception:
                pass

            return

        if linea.startswith("Posicion:"):
            patron = r"Posicion:\s*(-?\d+)\s*\|\s*Destino:\s*(-?\d+)\s*\|\s*PWM:\s*(-?\d+)"

            m = re.search(patron, linea)

            if m:
                estado_sistema["posicion"] = int(m.group(1))
                estado_sistema["setpoint"] = int(m.group(2))
                estado_sistema["pwm"] = int(m.group(3))
                estado_sistema["motor"] = "MOVIENDO"

            return

        if "Destino alcanzado" in linea:
            estado_sistema["estado"] = "Destino alcanzado"
            estado_sistema["motor"] = "DETENIDO"

            m = re.search(r"Posicion final:\s*(-?\d+)", linea)

            if m:
                estado_sistema["posicion"] = int(m.group(1))

            return

        if "Activando servo" in linea:
            estado_sistema["estado"] = "Activando servo para tirar pieza"
            estado_sistema["servo"] = "ACTIVO"
            return

        if "Servo NEGRO" in linea:
            estado_sistema["estado"] = "Servo negro: giro contrario"
            estado_sistema["servo"] = "ACTIVO NEGRO"
            return

        if "Servo activado" in linea:
            estado_sistema["estado"] = "Servo activado"
            estado_sistema["servo"] = "ACTIVO"
            return

        if "Servo regreso" in linea:
            estado_sistema["servo"] = "REPOSO"
            return

        if "Regresando automaticamente a cero" in linea:
            estado_sistema["estado"] = "Regresando automaticamente a cero"
            estado_sistema["setpoint"] = 0
            estado_sistema["motor"] = "MOVIENDO"
            return

        if "CICLO TERMINADO" in linea:
            estado_sistema["estado"] = "Ciclo terminado"
            estado_sistema["motor"] = "DETENIDO"
            estado_sistema["servo"] = "REPOSO"
            estado_sistema["setpoint"] = 0
            estado_sistema["pwm"] = 0
            return

        if "Sistema listo para otra pieza" in linea:
            estado_sistema["estado"] = "Sistema listo para otra pieza"
            estado_sistema["motor"] = "DETENIDO"
            estado_sistema["servo"] = "REPOSO"
            return

        if "PARO:" in linea:
            estado_sistema["estado"] = linea
            estado_sistema["motor"] = "DETENIDO"
            estado_sistema["pwm"] = 0
            return

        if "BOTON DE PARO PRESIONADO" in linea:
            estado_sistema["estado"] = "Boton de paro presionado"
            estado_sistema["motor"] = "DETENIDO"
            estado_sistema["pwm"] = 0
            return

        if "Boton liberado" in linea:
            estado_sistema["estado"] = "Boton liberado, regresando al centro"
            estado_sistema["motor"] = "MOVIENDO"
            estado_sistema["setpoint"] = 0
            return


def hilo_lectura_serial(ser):
    global _running

    while _running:
        try:
            linea = ser.readline().decode("utf-8", errors="ignore").strip()

            if linea:
                procesar_linea_arduino(linea)

        except Exception as e:
            print(f"[SERIAL ERROR] {e}")
            time.sleep(0.5)


def enviar_a_arduino(ser, cmd):
    try:
        ser.write((cmd.strip() + "\n").encode("utf-8"))
        ser.flush()
        return "OK comando enviado al Arduino"
    except Exception as e:
        return f"ERR {e}"


def obtener_status_json():
    with _lock:
        copia = json.loads(json.dumps(estado_sistema))

    return json.dumps(copia, ensure_ascii=False)


def procesar_comando_tcp(ser, msg):
    msg = msg.strip().upper()

    if msg == "STATUS":
        return obtener_status_json()

    if msg == "START":
        with _lock:
            estado_sistema["estado"] = "Sistema iniciado desde interfaz"
            estado_sistema["motor"] = "DETENIDO"
            estado_sistema["servo"] = "REPOSO"

        enviar_a_arduino(ser, "START")
        return "OK START"

    if msg == "RESET":
        with _lock:
            reiniciar_estado()

        enviar_a_arduino(ser, "RESET")
        return "OK RESET"

    if msg == "EMERGENCY":
        with _lock:
            estado_sistema["estado"] = "PARO DE EMERGENCIA ACTIVADO"
            estado_sistema["motor"] = "DETENIDO"
            estado_sistema["servo"] = "REPOSO"
            estado_sistema["pwm"] = 0

        enviar_a_arduino(ser, "EMERGENCY")
        return "OK EMERGENCY"

    if msg == "CLEAR_COUNTERS":
        with _lock:
            limpiar_contadores()
            estado_sistema["estado"] = "Contadores reiniciados"

        enviar_a_arduino(ser, "CLEAR_COUNTERS")
        return "OK CLEAR_COUNTERS"

    enviar_a_arduino(ser, msg)
    return f"OK comando reenviado: {msg}"


def main():
    global _running

    signal.signal(signal.SIGINT, handle_sig)
    signal.signal(signal.SIGTERM, handle_sig)

    ser = open_serial()

    t = threading.Thread(
        target=hilo_lectura_serial,
        args=(ser,),
        daemon=True
    )

    t.start()

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

                if not msg:
                    conn.sendall(b"ERR comando vacio\n")
                    continue

                try:
                    with _lock:
                        resp = procesar_comando_tcp(ser, msg)

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
