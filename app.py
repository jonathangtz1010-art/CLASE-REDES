#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from flask import Flask, render_template, request, redirect, url_for, session, jsonify
from werkzeug.security import check_password_hash
import socket
import json

# ====== CREDENCIALES ======
APP_USER = "NOMBRA TU USUARIO"
APP_PW_HASH = "PEGA_AQUI_EL_HASH_GENERAD"
SECRET_KEY = "REDES"

# ====== TCP hacia servidor_tcp.py ======
TCP_HOST = "127.0.0.1"
TCP_PORT = 5001

# ====== CONFIGURACION DEL SISTEMA ======
META_PIEZAS = 30

app = Flask(
    __name__,
    template_folder="templates",
    static_folder="static",
    static_url_path="/static"
)

app.secret_key = SECRET_KEY


def is_logged_in():
    return session.get("logged_in") is True


def send_cmd(cmd: str) -> str:
    with socket.create_connection((TCP_HOST, TCP_PORT), timeout=3) as s:
        s.sendall((cmd.strip() + "\n").encode("utf-8"))

        data = b""

        while b"\n" not in data:
            chunk = s.recv(4096)

            if not chunk:
                break

            data += chunk

        return data.decode("utf-8", errors="ignore").strip()


def estado_base():
    return {
        "ok": True,
        "estado": "Esperando datos del sistema",
        "color": "DESCONOCIDO",
        "posicion": 0,
        "setpoint": 0,
        "pwm": 0,
        "motor": "DETENIDO",
        "servo": "REPOSO",
        "meta": META_PIEZAS,
        "contadores": {
            "rojo": 0,
            "verde": 0,
            "azul": 0,
            "negro": 0,
            "total": 0
        }
    }


def convertir_numero(valor, defecto=0):
    try:
        return int(valor)
    except Exception:
        return defecto


def normalizar_estado(data):
    base = estado_base()

    if not isinstance(data, dict):
        base["estado"] = "Respuesta invalida del sistema"
        return base

    base["estado"] = data.get("estado", base["estado"])
    base["color"] = data.get("color", base["color"])

    base["posicion"] = convertir_numero(
        data.get("posicion", base["posicion"])
    )

    base["setpoint"] = convertir_numero(
        data.get("setpoint", base["setpoint"])
    )

    base["pwm"] = convertir_numero(
        data.get("pwm", base["pwm"])
    )

    base["motor"] = data.get("motor", base["motor"])
    base["servo"] = data.get("servo", base["servo"])

    contadores = data.get("contadores", {})

    rojo = convertir_numero(contadores.get("rojo", 0))
    verde = convertir_numero(contadores.get("verde", 0))
    azul = convertir_numero(contadores.get("azul", 0))
    negro = convertir_numero(contadores.get("negro", 0))

    total = convertir_numero(
        contadores.get("total", rojo + verde + azul + negro)
    )

    base["contadores"] = {
        "rojo": rojo,
        "verde": verde,
        "azul": azul,
        "negro": negro,
        "total": total
    }

    base["meta"] = META_PIEZAS

    return base


def parsear_estado(resp: str):
    base = estado_base()

    if not resp:
        base["estado"] = "Sin respuesta del sistema"
        return base

    try:
        data = json.loads(resp)
        return normalizar_estado(data)
    except Exception:
        pass

    texto = resp.strip()

    base["estado"] = texto

    if "ROJO" in texto.upper():
        base["color"] = "ROJO"
    elif "VERDE" in texto.upper():
        base["color"] = "VERDE"
    elif "AZUL" in texto.upper():
        base["color"] = "AZUL"
    elif "NEGRO" in texto.upper():
        base["color"] = "NEGRO"
    elif "SIN PIEZA" in texto.upper():
        base["color"] = "SIN_PIEZA"

    return base


def comando_simple(cmd):
    try:
        resp = send_cmd(cmd)

        return jsonify({
            "ok": True,
            "cmd": cmd,
            "resp": resp
        })

    except Exception as e:
        return jsonify({
            "ok": False,
            "cmd": cmd,
            "error": str(e)
        }), 500


@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        user = request.form.get("username", "").strip()
        pw = request.form.get("password", "")

        if user == APP_USER and check_password_hash(APP_PW_HASH, pw):
            session["logged_in"] = True
            return redirect(url_for("index"))

        return render_template(
            "login.html",
            error="Usuario o contraseña incorrectos"
        )

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
        return jsonify({
            "ok": False,
            "error": "No autorizado"
        }), 401

    try:
        resp = send_cmd("STATUS")
        data = parsear_estado(resp)
        data["ok"] = True
        data["resp"] = resp
        return jsonify(data)

    except Exception as e:
        data = estado_base()
        data["ok"] = True
        data["estado"] = "Sin conexion con servidor TCP"
        data["color"] = "DESCONOCIDO"
        data["error"] = str(e)

        return jsonify(data)


@app.post("/api/start")
def api_start():
    if not is_logged_in():
        return jsonify({
            "ok": False,
            "error": "No autorizado"
        }), 401

    return comando_simple("START")


@app.post("/api/reset")
def api_reset():
    if not is_logged_in():
        return jsonify({
            "ok": False,
            "error": "No autorizado"
        }), 401

    return comando_simple("RESET")


@app.post("/api/emergency")
def api_emergency():
    if not is_logged_in():
        return jsonify({
            "ok": False,
            "error": "No autorizado"
        }), 401

    return comando_simple("EMERGENCY")


@app.post("/api/clear-counters")
def api_clear_counters():
    if not is_logged_in():
        return jsonify({
            "ok": False,
            "error": "No autorizado"
        }), 401

    return comando_simple("CLEAR_COUNTERS")


@app.get("/api/ping")
def ping():
    if not is_logged_in():
        return jsonify({
            "ok": False,
            "error": "No autorizado"
        }), 401

    return jsonify({
        "ok": True,
        "sistema": "clasificador_colores",
        "meta": META_PIEZAS
    })


if __name__ == "__main__":
    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )
