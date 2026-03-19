const heroStatus = document.getElementById("heroStatus");
const heroMessage = document.getElementById("heroMessage");
const sidebarState = document.getElementById("sidebarState");
const lastUpdate = document.getElementById("lastUpdate");
const tcpTarget = document.getElementById("tcpTarget");
const serialPort = document.getElementById("serialPort");

const systemStateChip = document.getElementById("systemStateChip");
const modeType = document.getElementById("modeType");
const inputPin = document.getElementById("inputPin");

const ledGreen = document.getElementById("ledGreen");
const ledRed = document.getElementById("ledRed");
const greenStateText = document.getElementById("greenStateText");
const redStateText = document.getElementById("redStateText");

const a0Value = document.getElementById("a0Value");
const thresholdValue = document.getElementById("thresholdValue");
const differenceValue = document.getElementById("differenceValue");
const a0Bar = document.getElementById("a0Bar");
const thresholdBar = document.getElementById("thresholdBar");
const a0Percent = document.getElementById("a0Percent");
const thresholdPercent = document.getElementById("thresholdPercent");

const metricInput = document.getElementById("metricInput");
const metricThreshold = document.getElementById("metricThreshold");
const metricLedsMode = document.getElementById("metricLedsMode");
const metricDriver = document.getElementById("metricDriver");

const statusMessage = document.getElementById("statusMessage");
const recommendationBox = document.getElementById("recommendationBox");
const historyList = document.getElementById("historyList");

const waveCanvas = document.getElementById("waveCanvas");
const waveStatus = document.getElementById("waveStatus");
const waveCtx = waveCanvas.getContext("2d");

const POLL_MS = 120;
const MAX_SAMPLES = 220;

let samplesA0 = [];
let historyItems = [];
let previousSignature = "";

function setColorClass(el, color) {
  el.classList.remove("green", "red", "orange", "neutral");
  el.classList.add(color);
}

function setLedState(greenOn, redOn) {
  if (greenOn) {
    ledGreen.classList.remove("off");
    ledGreen.classList.add("on");
    greenStateText.textContent = "Encendido";
  } else {
    ledGreen.classList.remove("on");
    ledGreen.classList.add("off");
    greenStateText.textContent = "Apagado";
  }

  if (redOn) {
    ledRed.classList.remove("off");
    ledRed.classList.add("on");
    redStateText.textContent = "Encendido";
  } else {
    ledRed.classList.remove("on");
    ledRed.classList.add("off");
    redStateText.textContent = "Apagado";
  }
}

function pushWaveSample(a0, active) {
  if (!active) return;
  samplesA0.push(a0);
  if (samplesA0.length > MAX_SAMPLES) samplesA0.shift();
}

function resizeCanvas() {
  const dpr = window.devicePixelRatio || 1;
  const rect = waveCanvas.getBoundingClientRect();
  waveCanvas.width = rect.width * dpr;
  waveCanvas.height = rect.height * dpr;
  waveCtx.setTransform(dpr, 0, 0, dpr, 0, 0);
  drawWaveform(500);
}

function drawGrid(w, h) {
  waveCtx.strokeStyle = "rgba(255,255,255,0.08)";
  waveCtx.lineWidth = 1;

  for (let x = 0; x <= w; x += 50) {
    waveCtx.beginPath();
    waveCtx.moveTo(x, 0);
    waveCtx.lineTo(x, h);
    waveCtx.stroke();
  }

  for (let y = 0; y <= h; y += 40) {
    waveCtx.beginPath();
    waveCtx.moveTo(0, y);
    waveCtx.lineTo(w, y);
    waveCtx.stroke();
  }
}

function mapAnalogToY(value, h) {
  const top = 24;
  const bottom = h - 24;
  const normalized = Math.max(0, Math.min(1023, value)) / 1023;
  return bottom - normalized * (bottom - top);
}

function drawThresholdLine(threshold, w, h) {
  const y = mapAnalogToY(threshold, h);
  waveCtx.strokeStyle = "#ff9a3d";
  waveCtx.lineWidth = 2;
  waveCtx.setLineDash([8, 6]);
  waveCtx.beginPath();
  waveCtx.moveTo(0, y);
  waveCtx.lineTo(w, y);
  waveCtx.stroke();
  waveCtx.setLineDash([]);
}

function drawTrace(samples, color, w, h) {
  if (samples.length < 2) return;

  waveCtx.strokeStyle = color;
  waveCtx.lineWidth = 3;
  waveCtx.beginPath();

  const stepX = w / Math.max(MAX_SAMPLES - 1, 1);

  for (let i = 0; i < samples.length; i++) {
    const x = i * stepX;
    const y = mapAnalogToY(samples[i], h);

    if (i === 0) waveCtx.moveTo(x, y);
    else waveCtx.lineTo(x, y);
  }

  waveCtx.stroke();
}

function drawWaveform(threshold) {
  const w = waveCanvas.clientWidth;
  const h = waveCanvas.clientHeight;

  waveCtx.clearRect(0, 0, w, h);

  const bg = waveCtx.createLinearGradient(0, 0, 0, h);
  bg.addColorStop(0, "#07111f");
  bg.addColorStop(1, "#0d1830");
  waveCtx.fillStyle = bg;
  waveCtx.fillRect(0, 0, w, h);

  drawGrid(w, h);

  waveCtx.fillStyle = "rgba(255,255,255,0.8)";
  waveCtx.font = "12px Segoe UI";
  waveCtx.fillText("Trazo A0", 16, 22);
  waveCtx.fillText("Línea de umbral", 16, 40);

  drawThresholdLine(threshold, w, h);
  drawTrace(samplesA0, "#49a1ff", w, h);
}

function renderHistory() {
  if (historyItems.length === 0) {
    historyList.innerHTML = '<div class="history-empty">Todavía no hay eventos registrados.</div>';
    return;
  }

  historyList.innerHTML = historyItems.map(item => `
    <div class="history-item">
      <div class="history-dot ${item.color}"></div>
      <div class="history-main">
        <strong>${item.title}</strong>
        <span>${item.message}</span>
      </div>
      <div class="history-time">${item.time}</div>
    </div>
  `).join("");
}

function addHistory(title, message, color, time) {
  historyItems.unshift({ title, message, color, time });
  historyItems = historyItems.slice(0, 8);
  renderHistory();
}

function updateBars(data) {
  a0Value.textContent = data.a0;
  thresholdValue.textContent = data.threshold;
  differenceValue.textContent = data.difference;

  a0Bar.style.width = `${data.percent_a0}%`;
  thresholdBar.style.width = `${data.percent_threshold}%`;

  a0Percent.textContent = `${data.percent_a0}%`;
  thresholdPercent.textContent = `${data.percent_threshold}%`;
}

function updateUI(data) {
  heroStatus.textContent = data.state_short;
  heroMessage.textContent = data.message;

  sidebarState.textContent = data.state_label;
  setColorClass(sidebarState, data.color);

  systemStateChip.textContent = data.state_label;
  setColorClass(systemStateChip, data.color);

  modeType.textContent = data.mode;
  inputPin.textContent = data.input_pin;

  lastUpdate.textContent = data.updated_at;
  tcpTarget.textContent = data.tcp_target;
  serialPort.textContent = `${data.serial_port} @ ${data.serial_baud}`;

  statusMessage.textContent = data.message;
  recommendationBox.textContent = data.recommendation;

  metricInput.textContent = data.input_pin;
  metricThreshold.textContent = data.threshold;
  metricLedsMode.textContent = data.leds_mode;
  metricDriver.textContent = data.driver;

  setLedState(data.green_on, data.red_on);
  updateBars(data);

  if (data.active) {
    waveStatus.textContent = "Captura activa";
    setColorClass(waveStatus, "green");
  } else {
    waveStatus.textContent = "Captura congelada";
    setColorClass(waveStatus, "red");
  }

  pushWaveSample(data.a0, data.active);
  drawWaveform(data.threshold);

  const signature = `${data.state_label}-${data.a0}-${data.active}`;
  if (signature !== previousSignature) {
    addHistory(data.state_label, data.message, data.color, data.updated_at);
    previousSignature = signature;
  }
}

function showError(message = "No se pudo establecer comunicación con el sistema.") {
  heroStatus.textContent = "ERROR";
  heroMessage.textContent = message;

  sidebarState.textContent = "Sin conexión";
  setColorClass(sidebarState, "red");

  systemStateChip.textContent = "ERROR DE ENLACE";
  setColorClass(systemStateChip, "red");

  waveStatus.textContent = "Sin datos";
  setColorClass(waveStatus, "red");

  statusMessage.textContent = "Revisa Flask, servidor TCP, puerto serial y el Arduino.";
  recommendationBox.textContent = "Verifica que servidor_tcp.py esté activo y que el Arduino esté respondiendo.";
}

async function fetchStatus() {
  try {
    const res = await fetch("/api/status", { cache: "no-store" });

    if (res.status === 401) {
      window.location.href = "/login";
      return;
    }

    const data = await res.json();

    if (!data.ok) {
      showError(data.error || "Respuesta inválida del sistema");
      return;
    }

    updateUI(data);
  } catch (error) {
    showError("No fue posible consultar el estado del sistema.");
  }
}

window.addEventListener("resize", resizeCanvas);
resizeCanvas();
fetchStatus();
setInterval(fetchStatus, POLL_MS);
