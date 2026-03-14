/* =========================================
   CREDENCIALES DEL LOGIN
========================================= */
const usuarioCorrecto = "jonathan";
const contrasenaCorrecta = "12345";

/* =========================================
   ATAJO PARA OBTENER ELEMENTOS POR ID
========================================= */
const $ = id => document.getElementById(id);

/* =========================================
   PONER MAYÚSCULA A LA PRIMERA LETRA
========================================= */
const cap = txt => txt.charAt(0).toUpperCase() + txt.slice(1);

/* =========================================
   LIMITAR VALORES DENTRO DE UN RANGO
========================================= */
const limitar = (v, min, max) => Math.max(min, Math.min(max, parseInt(v) || min));

/* =========================================
   NÚMERO ALEATORIO
========================================= */
const random = (min, max) => Math.floor(Math.random() * (max - min + 1)) + min;

/* =========================================
   ELEMENTOS PRINCIPALES
========================================= */
const loginSection = $("loginSection");
const dashboardSection = $("dashboardSection");
const usuarioInput = $("usuario");
const contrasenaInput = $("contrasena");
const btnLogin = $("btnLogin");
const mensajeLogin = $("mensajeLogin");
const btnLogout = $("btnLogout");

const horaActual = $("horaActual");
const estadoGeneral = $("estadoGeneral");
const descripcionGeneral = $("descripcionGeneral");

const totalSensores = $("totalSensores");
const totalNormales = $("totalNormales");
const totalAdvertencia = $("totalAdvertencia");
const totalCriticos = $("totalCriticos");

const btnAleatorio = $("btnAleatorio");
const btnReset = $("btnReset");

const cardsGrid = $("cardsGrid");
const logicList = $("logicList");

/* =========================================
   CONFIGURACIÓN DE LOS SENSORES
========================================= */
const sensores = [
  {
    id: "humedad",
    nombre: "Sensor de humedad",
    badge: "Obligatorio",
    badgeClass: "",
    unidad: "%",
    min: 0,
    max: 100,
    valor: 45,
    control: "Modificar humedad",
    regla: "≤ 8% rojo, 9% a 20% amarillo, > 20% verde.",
    evaluar(v) {
      if (v <= 8) return ["rojo", "Crítico (humedad muy baja)"];
      if (v <= 20) return ["amarillo", "Advertencia (humedad baja)"];
      return ["verde", "Estable"];
    }
  },
  {
    id: "temperatura",
    nombre: "Sensor de temperatura",
    badge: "Adicional",
    badgeClass: "secondary",
    unidad: "°C",
    min: 0,
    max: 50,
    valor: 24,
    control: "Modificar temperatura",
    regla: "< 10°C rojo, 10°C a 17°C amarillo, 18°C a 28°C verde, 29°C a 35°C amarillo, > 35°C rojo.",
    evaluar(v) {
      if (v < 10) return ["rojo", "Crítico (temperatura muy baja)"];
      if (v <= 17) return ["amarillo", "Advertencia (temperatura baja)"];
      if (v <= 28) return ["verde", "Normal"];
      if (v <= 35) return ["amarillo", "Advertencia (temperatura alta)"];
      return ["rojo", "Crítico (temperatura muy alta)"];
    }
  },
  {
    id: "luz",
    nombre: "Sensor de luz ambiental",
    badge: "Adicional",
    badgeClass: "secondary",
    unidad: "%",
    min: 0,
    max: 100,
    valor: 60,
    control: "Modificar luz ambiental",
    regla: "≤ 20% rojo, 21% a 50% amarillo, > 50% verde.",
    evaluar(v) {
      if (v <= 20) return ["rojo", "Crítico (iluminación muy baja)"];
      if (v <= 50) return ["amarillo", "Advertencia (iluminación media)"];
      return ["verde", "Adecuado"];
    }
  },
  {
    id: "nivel",
    nombre: "Sensor de nivel de agua del tanque",
    badge: "Adicional",
    badgeClass: "secondary",
    unidad: "%",
    min: 0,
    max: 100,
    valor: 70,
    control: "Modificar nivel de agua del tanque",
    regla: "≤ 20% rojo, 21% a 50% amarillo, > 50% verde.",
    evaluar(v) {
      if (v <= 20) return ["rojo", "Crítico (tanque casi vacío)"];
      if (v <= 50) return ["amarillo", "Advertencia (nivel medio)"];
      return ["verde", "Correcto"];
    }
  },
  {
    id: "proximidad",
    nombre: "Sensor de proximidad de objeto",
    badge: "Adicional",
    badgeClass: "secondary",
    unidad: "cm",
    min: 0,
    max: 100,
    valor: 40,
    control: "Modificar proximidad",
    regla: "≤ 10 cm rojo, 11 cm a 25 cm amarillo, > 25 cm verde.",
    evaluar(v) {
      if (v <= 10) return ["rojo", "Crítico (objeto muy cerca)"];
      if (v <= 25) return ["amarillo", "Advertencia (objeto cercano)"];
      return ["verde", "Distancia segura"];
    }
  }
];

/* =========================================
   CAMBIAR COLOR DEL LED
========================================= */
function ponerLed(led, color) {
  led.className = `led ${
    color === "rojo" ? "led-red" :
    color === "amarillo" ? "led-yellow" :
    color === "verde" ? "led-green" :
    "led-off"
  }`;
}

/* =========================================
   ACTUALIZAR HORA
========================================= */
function actualizarHora() {
  horaActual.textContent = new Date().toLocaleTimeString();
}

/* =========================================
   CREAR TARJETAS DE SENSORES
========================================= */
function crearInterfaz() {
  cardsGrid.innerHTML = sensores.map(s => `
    <article class="sensor-card">
      <div class="card-top">
        <h3>${s.nombre}</h3>
        <span class="badge ${s.badgeClass}">${s.badge}</span>
      </div>

      <div class="sensor-value">
        <span id="${s.id}Valor">${s.valor}</span><small>${s.unidad}</small>
      </div>

      <div class="control-box">
        <label for="${s.id}Range">${s.control}</label>
        <input type="range" id="${s.id}Range" min="${s.min}" max="${s.max}" value="${s.valor}">
      </div>

      <div class="control-box">
        <label for="${s.id}Input">Valor manual</label>
        <input type="number" id="${s.id}Input" min="${s.min}" max="${s.max}" value="${s.valor}">
      </div>

      <div class="led-info">
        <div class="led led-off" id="led${cap(s.id)}"></div>
        <div>
          <p class="led-title">LED de ${s.nombre.toLowerCase().replace("sensor de ", "")}</p>
          <p class="led-state" id="estado${cap(s.id)}">Estado: --</p>
        </div>
      </div>
    </article>
  `).join("");

  logicList.innerHTML = sensores
    .map(s => `<li><strong>${s.nombre}:</strong> ${s.regla}</li>`)
    .join("");
}

/* =========================================
   ACTUALIZAR CADA SENSOR
========================================= */
function actualizarSensor(s) {
  const v = +$(s.id + "Range").value;

  $(s.id + "Input").value = v;
  $(s.id + "Valor").textContent = v;

  const [color, texto] = s.evaluar(v);

  ponerLed($("led" + cap(s.id)), color);
  $("estado" + cap(s.id)).textContent = "Estado: " + texto;

  return color;
}

/* =========================================
   ACTUALIZAR RESUMEN GENERAL
========================================= */
function actualizarResumen(estados) {
  const normales = estados.filter(e => e === "verde").length;
  const advertencia = estados.filter(e => e === "amarillo").length;
  const criticos = estados.filter(e => e === "rojo").length;

  totalSensores.textContent = estados.length;
  totalNormales.textContent = normales;
  totalAdvertencia.textContent = advertencia;
  totalCriticos.textContent = criticos;

  if (criticos) {
    estadoGeneral.textContent = "Alerta del sistema";
    descripcionGeneral.textContent =
      "Se detectó al menos una condición crítica en los sensores. Es necesario revisar los valores mostrados y tomar acciones correctivas.";
  } else if (advertencia) {
    estadoGeneral.textContent = "Sistema en advertencia";
    descripcionGeneral.textContent =
      "Uno o más sensores se encuentran en una condición intermedia que requiere atención.";
  } else {
    estadoGeneral.textContent = "Operación normal";
    descripcionGeneral.textContent =
      "Todos los sensores se encuentran en rangos adecuados y el sistema opera correctamente.";
  }
}

/* =========================================
   ACTUALIZAR TODO
========================================= */
function actualizarTodo() {
  const estados = sensores.map(s => actualizarSensor(s));
  actualizarResumen(estados);
  actualizarHora();
}

/* =========================================
   ENLAZAR RANGE E INPUT
========================================= */
function enlazarControles() {
  sensores.forEach(s => {
    const range = $(s.id + "Range");
    const input = $(s.id + "Input");

    range.addEventListener("input", () => {
      input.value = range.value;
      actualizarTodo();
    });

    input.addEventListener("input", () => {
      const v = limitar(input.value, s.min, s.max);
      input.value = v;
      range.value = v;
      actualizarTodo();
    });
  });
}

/* =========================================
   LOGIN
========================================= */
function iniciarSesion() {
  const usuario = usuarioInput.value.trim();
  const contrasena = contrasenaInput.value.trim();

  if (usuario === usuarioCorrecto && contrasena === contrasenaCorrecta) {
    mensajeLogin.textContent = "Acceso correcto. Bienvenido al sistema.";
    mensajeLogin.style.color = "#22c55e";

    loginSection.classList.remove("active");
    dashboardSection.classList.add("active");

    actualizarTodo();
  } else {
    mensajeLogin.textContent = "Usuario o contraseña incorrectos.";
    mensajeLogin.style.color = "#ef4444";
  }
}

/* =========================================
   CERRAR SESIÓN
========================================= */
function cerrarSesion() {
  dashboardSection.classList.remove("active");
  loginSection.classList.add("active");

  usuarioInput.value = "";
  contrasenaInput.value = "";
  mensajeLogin.textContent = "";
}

/* =========================================
   VALORES ALEATORIOS
========================================= */
function aleatorio() {
  sensores.forEach(s => {
    const v = random(s.min, s.max);
    $(s.id + "Range").value = v;
    $(s.id + "Input").value = v;
  });

  actualizarTodo();
}

/* =========================================
   RESTABLECER VALORES
========================================= */
function resetear() {
  sensores.forEach(s => {
    $(s.id + "Range").value = s.valor;
    $(s.id + "Input").value = s.valor;
  });

  actualizarTodo();
}

/* =========================================
   EVENTOS
========================================= */
btnLogin.addEventListener("click", iniciarSesion);
btnLogout.addEventListener("click", cerrarSesion);
btnAleatorio.addEventListener("click", aleatorio);
btnReset.addEventListener("click", resetear);

/* =========================================
   ENTRAR CON TECLA ENTER
========================================= */
[usuarioInput, contrasenaInput].forEach(input => {
  input.addEventListener("keypress", e => {
    if (e.key === "Enter") iniciarSesion();
  });
});

/* =========================================
   INICIO DEL SISTEMA
========================================= */
crearInterfaz();
enlazarControles();
actualizarTodo();

/* =========================================
   RELOJ AUTOMÁTICO
========================================= */
setInterval(actualizarHora, 1000);
