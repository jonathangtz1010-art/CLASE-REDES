const SENSORES = [
  { key: "humedad", min: 0, max: 100, defaultValue: 45 },
  { key: "temperatura", min: 0, max: 50, defaultValue: 24 },
  { key: "luz", min: 0, max: 1000, defaultValue: 650 },
  { key: "nivel", min: 0, max: 100, defaultValue: 70 },
  { key: "proximidad", min: 0, max: 200, defaultValue: 90 }
];

const estados = {};

document.addEventListener("DOMContentLoaded", () => {
  if (document.getElementById("clock")) {
    iniciarDashboard();
  }
});

function iniciarDashboard() {
  const randomBtn = document.getElementById("randomBtn");
  const resetBtn = document.getElementById("resetBtn");

  SENSORES.forEach(sensor => {
    const range = document.getElementById(`range-${sensor.key}`);
    const input = document.getElementById(`input-${sensor.key}`);

    range.addEventListener("input", () => {
      let valor = limitarValor(Number(range.value), sensor.min, sensor.max);
      input.value = valor;
      actualizarSensor(sensor.key, valor);
      actualizarResumen();
    });

    input.addEventListener("input", () => {
      if (input.value === "") return;

      let valor = limitarValor(Number(input.value), sensor.min, sensor.max);
      input.value = valor;
      range.value = valor;
      actualizarSensor(sensor.key, valor);
      actualizarResumen();
    });

    actualizarSensor(sensor.key, sensor.defaultValue);
  });

  randomBtn.addEventListener("click", () => {
    SENSORES.forEach(sensor => {
      const valor = numeroAleatorio(sensor.min, sensor.max);
      document.getElementById(`range-${sensor.key}`).value = valor;
      document.getElementById(`input-${sensor.key}`).value = valor;
      actualizarSensor(sensor.key, valor);
    });

    actualizarResumen();
  });

  resetBtn.addEventListener("click", () => {
    SENSORES.forEach(sensor => {
      document.getElementById(`range-${sensor.key}`).value = sensor.defaultValue;
      document.getElementById(`input-${sensor.key}`).value = sensor.defaultValue;
      actualizarSensor(sensor.key, sensor.defaultValue);
    });

    actualizarResumen();
  });

  actualizarResumen();
  actualizarReloj();
  setInterval(actualizarReloj, 1000);
}

function actualizarSensor(key, value) {
  document.getElementById(`value-${key}`).textContent = value;

  const resultado = clasificarSensor(key, value);

  const led = document.getElementById(`led-${key}`);
  const badge = document.getElementById(`badge-${key}`);
  const state = document.getElementById(`state-${key}`);

  led.className = `led ${resultado.led}`;
  badge.className = `pill ${resultado.pill}`;
  badge.textContent = resultado.badge;
  state.textContent = resultado.texto;

  estados[key] = resultado.nivel;
}

function clasificarSensor(key, value) {
  if (key === "humedad") {
    if (value <= 8) {
      return { nivel: "critical", led: "led-red", pill: "pill-critical", badge: "Crítico", texto: "Humedad muy baja" };
    } else if (value <= 20) {
      return { nivel: "warning", led: "led-yellow", pill: "pill-warning", badge: "Advertencia", texto: "Humedad baja" };
    } else {
      return { nivel: "normal", led: "led-green", pill: "pill-normal", badge: "Estable", texto: "Humedad adecuada" };
    }
  }

  if (key === "temperatura") {
    if (value < 18) {
      return { nivel: "warning", led: "led-blue", pill: "pill-info", badge: "Baja", texto: "Temperatura baja" };
    } else if (value <= 28) {
      return { nivel: "normal", led: "led-green", pill: "pill-normal", badge: "Normal", texto: "Temperatura estable" };
    } else if (value <= 35) {
      return { nivel: "warning", led: "led-yellow", pill: "pill-warning", badge: "Alta", texto: "Temperatura elevada" };
    } else {
      return { nivel: "critical", led: "led-red", pill: "pill-critical", badge: "Crítica", texto: "Temperatura demasiado alta" };
    }
  }

  if (key === "luz") {
    if (value < 200) {
      return { nivel: "critical", led: "led-red", pill: "pill-critical", badge: "Deficiente", texto: "Iluminación muy baja" };
    } else if (value <= 600) {
      return { nivel: "warning", led: "led-yellow", pill: "pill-warning", badge: "Media", texto: "Iluminación intermedia" };
    } else {
      return { nivel: "normal", led: "led-green", pill: "pill-normal", badge: "Óptima", texto: "Iluminación adecuada" };
    }
  }

  if (key === "nivel") {
    if (value < 25) {
      return { nivel: "critical", led: "led-red", pill: "pill-critical", badge: "Bajo", texto: "Nivel insuficiente" };
    } else if (value <= 60) {
      return { nivel: "warning", led: "led-yellow", pill: "pill-warning", badge: "Medio", texto: "Nivel intermedio" };
    } else {
      return { nivel: "normal", led: "led-green", pill: "pill-normal", badge: "Óptimo", texto: "Nivel suficiente" };
    }
  }

  if (key === "proximidad") {
    if (value < 20) {
      return { nivel: "critical", led: "led-red", pill: "pill-critical", badge: "Riesgo", texto: "Objeto demasiado cercano" };
    } else if (value <= 80) {
      return { nivel: "warning", led: "led-yellow", pill: "pill-warning", badge: "Precaución", texto: "Objeto a distancia media" };
    } else {
      return { nivel: "normal", led: "led-green", pill: "pill-normal", badge: "Segura", texto: "Distancia segura" };
    }
  }
}

function actualizarResumen() {
  let normales = 0;
  let advertencias = 0;
  let criticos = 0;

  for (let key in estados) {
    if (estados[key] === "normal") normales++;
    if (estados[key] === "warning") advertencias++;
    if (estados[key] === "critical") criticos++;
  }

  document.getElementById("stat-total").textContent = SENSORES.length;
  document.getElementById("stat-normal").textContent = normales;
  document.getElementById("stat-warning").textContent = advertencias;
  document.getElementById("stat-critical").textContent = criticos;

  const titulo = document.getElementById("overallTitle");
  const texto = document.getElementById("overallText");

  if (criticos > 0) {
    titulo.textContent = "Alerta del sistema";
    texto.textContent = "Se detectaron sensores en condición crítica. Es necesario revisar inmediatamente los valores.";
  } else if (advertencias > 0) {
    titulo.textContent = "Sistema en advertencia";
    texto.textContent = "El sistema sigue operando, pero existen valores intermedios que requieren atención.";
  } else {
    titulo.textContent = "Operación normal";
    texto.textContent = "Todos los sensores se encuentran en rangos adecuados y el sistema opera correctamente.";
  }
}

function actualizarReloj() {
  const reloj = document.getElementById("clock");
  const ahora = new Date();
  reloj.textContent = ahora.toLocaleTimeString();
}

function limitarValor(valor, min, max) {
  if (isNaN(valor)) return min;
  if (valor < min) return min;
  if (valor > max) return max;
  return valor;
}

function numeroAleatorio(min, max) {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}
