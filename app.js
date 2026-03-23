const temperatura = document.getElementById("temperatura");
const humedad = document.getElementById("humedad");
const rango = document.getElementById("rango");
const velocidad = document.getElementById("velocidad");
const pwm = document.getElementById("pwm");
const estado = document.getElementById("estado");
const motorEstado = document.getElementById("motorEstado");
const statusFill = document.getElementById("statusFill");
const ultimaActualizacion = document.getElementById("ultimaActualizacion");

function limpiarClases() {
    rango.classList.remove("bajo", "medio", "alto");
    velocidad.classList.remove("bajo", "medio", "alto");
    estado.classList.remove("bajo", "medio", "alto");
}

function aplicarEstiloSegunRango(nombreRango) {
    limpiarClases();

    if (nombreRango === "Rango 1") {
        rango.classList.add("bajo");
        velocidad.classList.add("bajo");
        estado.classList.add("bajo");
        statusFill.style.width = "33%";
        statusFill.style.background = "#3b82f6";
        motorEstado.textContent = "Motor en velocidad baja";
    } 
    else if (nombreRango === "Rango 2") {
        rango.classList.add("medio");
        velocidad.classList.add("medio");
        estado.classList.add("medio");
        statusFill.style.width = "66%";
        statusFill.style.background = "#f59e0b";
        motorEstado.textContent = "Motor en velocidad media";
    } 
    else if (nombreRango === "Rango 3") {
        rango.classList.add("alto");
        velocidad.classList.add("alto");
        estado.classList.add("alto");
        statusFill.style.width = "100%";
        statusFill.style.background = "#ef4444";
        motorEstado.textContent = "Motor en velocidad alta";
    } 
    else {
        statusFill.style.width = "0%";
        statusFill.style.background = "#9ca3af";
        motorEstado.textContent = "--";
    }
}

function horaActual() {
    const ahora = new Date();
    return ahora.toLocaleTimeString();
}

async function actualizarDatos() {
    try {
        const response = await fetch("/get_data");
        const data = await response.json();

        if (data.error) {
            estado.textContent = "Error de comunicación";
            motorEstado.textContent = "Sin datos";
            statusFill.style.width = "0%";
            statusFill.style.background = "#9ca3af";
            return;
        }

        temperatura.textContent = data.temperatura + " °C";
        humedad.textContent = data.humedad + " %";
        rango.textContent = data.rango;
        velocidad.textContent = data.velocidad;
        pwm.textContent = data.pwm;
        estado.textContent = data.estado;

        aplicarEstiloSegunRango(data.rango);
        ultimaActualizacion.textContent = "Última actualización: " + horaActual();

    } catch (error) {
        estado.textContent = "Error al actualizar";
        motorEstado.textContent = "Sin conexión";
        statusFill.style.width = "0%";
        statusFill.style.background = "#9ca3af";
        console.log("Error:", error);
    }
}

setInterval(actualizarDatos, 1000);
actualizarDatos();
