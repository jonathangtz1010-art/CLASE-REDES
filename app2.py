from flask import Flask, render_template, request, redirect, url_for, session

app = Flask(__name__, template_folder="templates", static_folder="static")
app.secret_key = "smartsense_secret_2026"

USUARIO_VALIDO = "admin"
CONTRASENA_VALIDA = "12345"


@app.route("/", methods=["GET", "POST"])
def login():
    if session.get("activa"):
        return redirect(url_for("dashboard"))

    error = ""

    if request.method == "POST":
        usuario = request.form.get("username", "").strip()
        contrasena = request.form.get("password", "").strip()

        if usuario == USUARIO_VALIDO and contrasena == CONTRASENA_VALIDA:
            session["activa"] = True
            session["usuario"] = usuario
            return redirect(url_for("dashboard"))
        else:
            error = "Usuario o contraseña incorrectos."

    return render_template("login.html", error=error)


@app.route("/dashboard", methods=["GET"])
def dashboard():
    if not session.get("activa"):
        return redirect(url_for("login"))

    usuario = session.get("usuario", "admin")
    return render_template("index.html", usuario=usuario)


@app.route("/logout", methods=["GET"])
def logout():
    session.clear()
    return redirect(url_for("login"))


if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5000)
