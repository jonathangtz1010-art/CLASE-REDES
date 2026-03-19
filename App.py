#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from flask import Flask, render_template, request, redirect, url_for, session, jsonify
from werkzeug.security import check_password_hash
from datetime import datetime
import socket

APP_USER = "Alejandro"
APP_PASS = "PEGA_AQUI_EL_HASH_QUE_TE_GENERO_EL_COMANDO"
SECRET_KEY = "REDES_A0_LDR_2026"

TCP_HOST = "127.0.0.1"
TCP_PORT = 5001

SERIAL_LABEL = "/dev/ttyACM0"
SERIAL_BAUD = 115200

app = Flask(__name__, template_folder="templates", static_folder="static", static_url_path="/static")
app.secret_key = SECRET_KEY

def is_logged_in():
    return session.get("logged_in") is True

def send_cmd(cmd: str) -> str:
    with socket.create_connection((TCP_HOST, TCP_PORT), timeout=3) as s:
        s.sendall((cmd.strip() + "\n").encode("utf-8"))
        data = b""
        while b"\n" not in data:
            chunk = s.recv(1024)
            if not chunk:
                break
            data += chunk
        return data.decode("utf-8", errors="ignore").strip()

def parse_status(resp: str):
    data = {}
    for item in resp.split():
        if "=" in item:
            k, v = item.split("=", 1)
            data[k.strip()] = v.strip()

    a0 = int(data.get("a0", "0"))
    threshold = int(data.get("threshold", "500"))
    red_on = data.get("red", "0") == "1"
    green_on = data.get("green", "0") == "1"
    state_code = data.get("state", "REPOSO")
    mode = data.get("mode", "ANALOG")
    input_pin = data.get("input", "A0")
    leds_mode = data.get("leds", "HARDWARE")
    driver = data.get("driver", "TRANSISTOR")

    active = (state_code == "ACTIVO")
    difference = a0 - threshold
    percent_a0 = round((a0 / 1023.0) * 100.0, 1)
    percent_threshold = round((threshold / 1023.0) * 100.0, 1)

    if active:
        state_label = "Sistema activo"
        state_short = "ACTIVO"
        color = "green"
        message = "La luz entrante medida por el LDR superó el umbral y el circuito activó físicamente el LED rojo y el LED verde mediante el transistor."
        recommendation = "La gráfica continúa capturando porque el sistema está en estado activo."
    else:
        state_label = "Sistema en reposo"
        state_short = "REPOSO"
        color = "red"
        message = "La luz entrante medida por el LDR no supera el umbral. El circuito mantiene apagados el LED rojo y el LED verde."
        recommendation = "La gráfica queda congelada hasta que el circuito vuelva a entrar en estado activo."

    return {
        "ok": True,
        "source_response": resp,
        "a0": a0,
        "threshold": threshold,
        "difference": difference,
        "red_on": red_on,
        "green_on": green_on,
        "active": active,
        "state_code": state_code,
        "mode": mode,
        "input_pin": input_pin,
        "leds_mode": leds_mode,
        "driver": driver,
        "percent_a0": percent_a0,
        "percent_threshold": percent_threshold,
        "state_label": state_label,
        "state_short": state_short,
        "message": message,
        "recommendation": recommendation,
        "color": color,
        "serial_port": SERIAL_LABEL,
        "serial_baud": SERIAL_BAUD,
        "tcp_target": f"{TCP_HOST}:{TCP_PORT}",
        "updated_at": datetime.now().strftime("%d/%m/%Y %H:%M:%S")
    }

@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        user = request.form.get("username", "").strip()
        pw = request.form.get("password", "").strip()

        if user == APP_USER and check_password_hash(APP_PASS, pw):
            session["logged_in"] = True
            return redirect(url_for("index"))

        return render_template("login.html", error="Usuario o contraseña incorrectos")

    return render_template("login.html", error=None)

@app.route("/logout")
def logout():
    session.clear()
    return redirect(url_for("login"))

@app.route("/")
def index():
    if not is_logged_in():
        return redirect(url_for("login"))
    return render_template("index.html")

@app.get("/api/status")
def api_status():
    if not is_logged_in():
        return jsonify({"ok": False, "error": "No autorizado"}), 401

    try:
        resp = send_cmd("STATUS")
        if not resp.startswith("OK"):
            return jsonify({
                "ok": False,
                "error": resp,
                "updated_at": datetime.now().strftime("%d/%m/%Y %H:%M:%S")
            })
        return jsonify(parse_status(resp))
    except Exception as e:
        return jsonify({
            "ok": False,
            "error": str(e),
            "updated_at": datetime.now().strftime("%d/%m/%Y %H:%M:%S")
        })

@app.get("/api/ping")
def ping():
    if not is_logged_in():
        return jsonify({"ok": False, "error": "No autorizado"}), 401
    return jsonify({"ok": True})

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
