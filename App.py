from flask import Flask, render_template, request, redirect, url_for, session, jsonify
import socket
import json

app = Flask(__name__)
app.secret_key = "12345"

TCP_HOST = "127.0.0.1"
TCP_PORT = 5001

USUARIO = "jonathan"
PASSWORD = "12345"

def pedir_datos():
    try:
        cliente = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        cliente.connect((TCP_HOST, TCP_PORT))
        cliente.sendall(b"GET_DATA")
        respuesta = cliente.recv(4096).decode("utf-8")
        cliente.close()
        return json.loads(respuesta)
    except Exception as e:
        return {"error": str(e)}

@app.route("/")
def home():
    if "usuario" not in session:
        return redirect(url_for("login"))
    return render_template("index.html")

@app.route("/login", methods=["GET", "POST"])
def login():
    mensaje = ""

    if request.method == "POST":
        usuario = request.form.get("usuario")
        password = request.form.get("password")

        if usuario == USUARIO and password == PASSWORD:
            session["usuario"] = usuario
            return redirect(url_for("home"))
        else:
            mensaje = "Usuario o contraseña incorrectos"

    return render_template("login.html", mensaje=mensaje)

@app.route("/logout")
def logout():
    session.clear()
    return redirect(url_for("login"))

@app.route("/get_data")
def get_data():
    if "usuario" not in session:
        return jsonify({"error": "No autorizado"})
    return jsonify(pedir_datos())

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
