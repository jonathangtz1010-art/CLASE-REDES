from flask import Flask, request, redirect, url_for, session

app = Flask(__name__)
app.secret_key = "prueba_login_123"

APP_USER = "admin"
APP_PASS = "1234"

@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        user = request.form.get("username", "").strip()
        pw = request.form.get("password", "").strip()

        print("RECIBIDO:", user, pw)

        if user == APP_USER and pw == APP_PASS:
            session["logged_in"] = True
            print("LOGIN OK")
            return redirect(url_for("index"))

        print("LOGIN FAIL")
        return """
        <h2>Login incorrecto</h2>
        <a href="/login">Volver</a>
        """

    return """
    <form method="POST">
      <input name="username" placeholder="usuario">
      <input name="password" type="password" placeholder="contraseña">
      <button type="submit">Entrar</button>
    </form>
    """

@app.route("/")
def index():
    print("SESSION:", dict(session))
    if session.get("logged_in") is True:
        return "<h1>Entraste bien a la interfaz</h1>"
    return redirect(url_for("login"))

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False)
