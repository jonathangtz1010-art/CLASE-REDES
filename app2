// ============================================================
// app.js – Lógica completa del Clasificador de Colores
// ============================================================

const statusEl = document.getElementById("status");
const colorCircle = document.getElementById("colorCircle");
const colorName = document.getElementById("colorName");
const colorSub = document.getElementById("colorSub");
const progressFill = document.getElementById("progressFill");
const progressText = document.getElementById("progressText");
const redCount = document.getElementById("redCount");
const greenCount = document.getElementById("greenCount");
const blueCount = document.getElementById("blueCount");
const blackCount = document.getElementById("blackCount");
const totalCount = document.getElementById("totalCount");
const posicionEl = document.getElementById("posicion");
const setpointEl = document.getElementById("setpoint");
const pwmEl = document.getElementById("pwm");
const motorStateEl = document.getElementById("motorState");
const servoStateEl = document.getElementById("servoState");
const logBox = document.getElementById("logBox");
const btnStart = document.getElementById("btnStart");
const btnReset = document.getElementById("btnReset");
const btnEmergency = document.getElementById("btnEmergency");
const btnClearCounters = document.getElementById("btnClearCounters");
const liveClock = document.getElementById("liveClock");
const productionMode = document.getElementById("productionMode");
const connectionState = document.getElementById("connectionState");
const qualityState = document.getElementById("qualityState");

const META_PIEZAS = 30;

let ultimoEstado = "";
let ultimoColor = "";
let ultimoTotal = -1;

// ===== RELOJ EN VIVO =====
function actualizarReloj() {
  if (!liveClock) return;
  liveClock.textContent = new Date().toLocaleTimeString();
}

setInterval(actualizarReloj, 1000);
actualizarReloj();

// ===== STATUS =====
function setStatus(texto, tipo = "") {
  if (!statusEl) return;
  statusEl.textContent = texto;
  statusEl.className = "status";
  if (tipo) {
    statusEl.classList.add(tipo);
  }
}

// ===== LOG =====
function escribirLog(texto) {
  if (!logBox) return;
  const p = document.createElement("p");
  const hora = new Date().toLocaleTimeString();
  p.textContent = `[${hora}] ${texto}`;
  logBox.prepend(p);
  while (logBox.children.length > 45) {
    logBox.removeChild(logBox.lastChild);
  }
}

// ===== HELPERS COLOR =====
function normalizarColor(color) {
  if (!color) return "DESCONOCIDO";
  return String(color)
    .trim()
    .toUpperCase()
    .replace(" ", "_");
}

function nombreColor(color) {
  color = normalizarColor(color);
  if (color === "ROJO") return "ROJO";
  if (color === "VERDE") return "VERDE";
  if (color === "AZUL") return "AZUL";
  if (color === "NEGRO") return "NEGRO";
  if (color === "SIN_PIEZA") return "SIN PIEZA";
  return "DESCONOCIDO";
}

function claseColor(color) {
  color = normalizarColor(color);
  if (color === "ROJO") return "red";
  if (color === "VERDE") return "green";
  if (color === "AZUL") return "blue";
  if (color === "NEGRO") return "black";
  return "";
}

// ===== ACTUALIZAR COLOR UI =====
function actualizarColor(color) {
  if (!colorCircle || !colorName || !colorSub) return;
  const colorNormal = normalizarColor(color);
  colorCircle.className = "colorCircle";
  const clase = claseColor(colorNormal);
  if (clase) {
    colorCircle.classList.add(clase);
  }
  colorName.textContent = nombreColor(colorNormal);
  if (colorNormal === "SIN_PIEZA") {
    colorSub.textContent = "Esperando pieza en el sensor";
    if (qualityState) qualityState.textContent = "Sin pieza";
  } else if (colorNormal === "DESCONOCIDO") {
    colorSub.textContent = "Color no reconocido";
    if (qualityState) qualityState.textContent = "Lectura sin clasificar";
  } else {
    colorSub.textContent = "Color detectado por el sensor";
    if (qualityState) qualityState.textContent = "Color válido";
  }
}

// ===== ACTUALIZAR CONTADORES =====
function actualizarContadores(contadores) {
  if (!contadores) return;
  const rojo = Number(contadores.rojo || 0);
  const verde = Number(contadores.verde || 0);
  const azul = Number(contadores.azul || 0);
  const negro = Number(contadores.negro || 0);
  const total = Number(
    contadores.total !== undefined
      ? contadores.total
      : rojo + verde + azul + negro
  );
  if (redCount) redCount.textContent = rojo;
  if (greenCount) greenCount.textContent = verde;
  if (blueCount) blueCount.textContent = azul;
  if (blackCount) blackCount.textContent = negro;
  if (totalCount) totalCount.textContent = total;
  actualizarProgreso(total);
  if (total !== ultimoTotal) {
    ultimoTotal = total;
    escribirLog(`Total de piezas: ${total}/${META_PIEZAS}`);
  }
}

// ===== ACTUALIZAR PROGRESO =====
function actualizarProgreso(total) {
  const porcentaje = Math.min((total / META_PIEZAS) * 100, 100);
  if (progressFill) {
    progressFill.style.width = `${porcentaje}%`;
  }
  if (progressText) {
    progressText.textContent = `${total}/${META_PIEZAS} piezas`;
  }
  if (total >= META_PIEZAS) {
    setStatus("Meta de 30 piezas completada", "ok");
    if (productionMode) productionMode.textContent = "Meta completa";
  } else {
    if (productionMode) productionMode.textContent = "Automático";
  }
}

// ===== DATOS TÉCNICOS =====
function actualizarDatosTecnicos(data) {
  if (posicionEl) posicionEl.textContent = data.posicion ?? "0";
  if (setpointEl) setpointEl.textContent = data.setpoint ?? "0";
  if (pwmEl) pwmEl.textContent = data.pwm ?? "0";
  if (motorStateEl) {
    motorStateEl.textContent = data.motor || "DETENIDO";
  }
  if (servoStateEl) {
    servoStateEl.textContent = data.servo || "REPOSO";
  }
}

// ===== ACTUALIZAR ESTADO COMPLETO =====
function actualizarEstado(data) {
  if (!data) return;
  const estado = data.estado || "Sistema conectado";
  const color = data.color || "DESCONOCIDO";

  let tipoEstado = "ok";
  if (
    estado.toUpperCase().includes("PARO") ||
    estado.toUpperCase().includes("ERROR") ||
    estado.toUpperCase().includes("EMERGENCIA") ||
    estado.toUpperCase().includes("NO CONECTADO")
  ) {
    tipoEstado = "danger";
  }
  if (
    estado.toUpperCase().includes("ESPERANDO") ||
    estado.toUpperCase().includes("MOVIENDO") ||
    estado.toUpperCase().includes("DETECTANDO")
  ) {
    tipoEstado = "warning";
  }

  setStatus(estado, tipoEstado);

  if (connectionState) {
    if (estado.toUpperCase().includes("NO CONECTADO") || estado.toUpperCase().includes("SIN CONEXION")) {
      connectionState.textContent = "Sin conexión";
    } else {
      connectionState.textContent = "Conectado";
    }
  }

  actualizarColor(color);
  actualizarContadores(data.contadores);
  actualizarDatosTecnicos(data);

  if (estado !== ultimoEstado) {
    ultimoEstado = estado;
    escribirLog(estado);
  }
  if (color !== ultimoColor) {
    ultimoColor = color;
    escribirLog(`Color detectado: ${nombreColor(color)}`);
  }
}

// ===== CARGAR ESTADO DESDE API =====
async function cargarEstado() {
  try {
    const res = await fetch("/api/status");
    if (res.status === 401) {
      window.location.href = "/login";
      return;
    }
    const data = await res.json();
    if (data.ok) {
      actualizarEstado(data);
    } else {
      setStatus(data.error || "Error al leer el sistema", "danger");
      if (connectionState) connectionState.textContent = "Error";
    }
  } catch (e) {
    setStatus("Sin conexión con servidor TCP", "danger");
    if (connectionState) connectionState.textContent = "Sin conexión";
  }
}

// ===== ENVIAR COMANDO =====
async function enviarComando(ruta, mensaje) {
  setStatus("Enviando comando...", "warning");
  try {
    const res = await fetch(ruta, {
      method: "POST",
      headers: { "Content-Type": "application/json" }
    });
    if (res.status === 401) {
      window.location.href = "/login";
      return;
    }
    const data = await res.json();
    if (data.ok) {
      setStatus(mensaje, "ok");
      escribirLog(mensaje);
      cargarEstado();
    } else {
      setStatus(data.error || "No se pudo enviar el comando", "danger");
      escribirLog(data.error || "Error en comando");
    }
  } catch (e) {
    setStatus("Error de comunicación", "danger");
    escribirLog("Error de comunicación con el servidor");
  }
}

// ===== EVENTOS BOTONES =====
if (btnStart) {
  btnStart.addEventListener("click", () => {
    enviarComando("/api/start", "Sistema iniciado");
  });
}

if (btnReset) {
  btnReset.addEventListener("click", () => {
    enviarComando("/api/reset", "Sistema reiniciado desde cero");
  });
}

if (btnEmergency) {
  btnEmergency.addEventListener("click", () => {
    enviarComando("/api/emergency", "PARO DE EMERGENCIA ACTIVADO");
  });
}

if (btnClearCounters) {
  btnClearCounters.addEventListener("click", () => {
    enviarComando("/api/clear-counters", "Contadores reiniciados");
  });
}

// ===== INICIALIZACIÓN =====
setStatus("Cargando sistema...", "warning");
actualizarColor("DESCONOCIDO");
actualizarProgreso(0);
cargarEstado();

// ===== POLLING AUTOMÁTICO =====
setInterval(cargarEstado, 700);
