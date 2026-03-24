const temperatura = document.getElementById("temperatura");
const humedad = document.getElementById("humedad");
const rango = document.getElementById("rango");
const velocidad = document.getElementById("velocidad");
const pwm = document.getElementById("pwm");
const estado = document.getElementById("estado");
const motorEstado = document.getElementById("motorEstado");
const statusFill = document.getElementById("statusFill");
const ultimaActualizacion = document.getElementById("ultimaActualizacion");
const ultimaActualizacionTexto = document.getElementById("ultimaActualizacionTexto");
const estadoMini = document.getElementById("estadoMini");
const statusChip = document.getElementById("statusChip");
const rangePill = document.getElementById("rangePill");
const pulsos = document.getElementById("pulsos");

function horaActual() {
    const ahora = new Date();
    return ahora.toLocaleTimeString();
}

function aplicarEstiloSegunRango(nombreRango) {
    if (nombreRango === "Rango 1") {
        statusFill.style.width = "33%";
        statusFill.style.background = "linear-gradient(90deg, #60a5fa, #2563eb)";
        motorEstado.textContent = "Motor en velocidad baja";
        rangePill.textContent = "Rango 1";
        statusChip.textContent = "Operación baja";
    } else if (nombreRango === "Rango 2") {
        statusFill.style.width = "66%";
        statusFill.style.background = "linear-gradient(90deg, #fbbf24, #f59e0b)";
        motorEstado.textContent = "Motor en velocidad media";
        rangePill.textContent = "Rango 2";
        statusChip.textContent = "Operación media";
    } else if (nombreRango === "Rango 3") {
        statusFill.style.width = "100%";
        statusFill.style.background = "linear-gradient(90deg, #fb7185, #ef4444)";
        motorEstado.textContent = "Motor en velocidad alta";
        rangePill.textContent = "Rango 3";
        statusChip.textContent = "Operación alta";
    } else {
        statusFill.style.width = "0%";
        statusFill.style.background = "linear-gradient(90deg, #94a3b8, #64748b)";
        motorEstado.textContent = "Sin lectura";
        rangePill.textContent = "Sin lectura";
        statusChip.textContent = "Esperando datos";
    }
}

async function actualizarDatos() {
    try {
        const response = await fetch("/get_data");
        const data = await response.json();

        if (!data.ok) {
            estado.textContent = "Error de comunicación";
            estadoMini.textContent = "Sin respuesta";
            motorEstado.textContent = "Sin datos";
            rangePill.textContent = "Error";
            statusChip.textContent = "Error";
            statusFill.style.width = "0%";
            statusFill.style.background = "linear-gradient(90deg, #94a3b8, #64748b)";
            return;
        }

        temperatura.textContent = data.temperatura + " °C";
        humedad.textContent = data.humedad + " %";
        rango.textContent = data.rango;
        velocidad.textContent = data.velocidad;
        pwm.textContent = data.pwm;
        estado.textContent = data.estado;
        estadoMini.textContent = data.estado;
        pulsos.textContent = data.pulsos;

        aplicarEstiloSegunRango(data.rango);

        const hora = horaActual();
        ultimaActualizacion.textContent = hora;
        ultimaActualizacionTexto.textContent = hora;

    } catch (error) {
        estado.textContent = "Error al actualizar";
        estadoMini.textContent = "Sin conexión";
        motorEstado.textContent = "Sin conexión";
        rangePill.textContent = "Error";
        statusChip.textContent = "Error";
        statusFill.style.width = "0%";
        statusFill.style.background = "linear-gradient(90deg, #94a3b8, #64748b)";
        console.log("Error:", error);
    }
}

setInterval(actualizarDatos, 1000);
actualizarDatos();

setInterval(actualizarDatos, 1000);
actualizarDatos();
