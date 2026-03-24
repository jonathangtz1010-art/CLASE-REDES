#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Puente H
const int ENA = 3;
const int IN1 = 4;
const int IN2 = 2;

// Encoder
const int encoderA = 6;
const int encoderB = 5;

long pulsos = 0;
int estadoAnteriorA = 0;

// Variables del sistema
float temperatura = 0.0;
float humedad = 0.0;
String rango = "";
String velocidad = "";
String estado = "";
int pwmValue = 0;

String buffer = "";
unsigned long tiempoSerial = 0;

void actualizarControl(float temp) {
  if (temp >= 26.0 && temp < 27.0) {
    rango = "Rango 1";
    velocidad = "Baja";
    estado = "Temperatura baja";
    pwmValue = 70;
  } 
  else if (temp >= 27.0 && temp < 28.0) {
    rango = "Rango 2";
    velocidad = "Media";
    estado = "Temperatura media";
    pwmValue = 170;
  } 
  else if (temp >= 28.0) {
    rango = "Rango 3";
    velocidad = "Alta";
    estado = "Temperatura alta";
    pwmValue = 255;
  } 
  else {
    rango = "Fuera de rango";
    velocidad = "Apagado";
    estado = "Temperatura menor a 26";
    pwmValue = 0;
  }

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, pwmValue);
}

String crearRespuesta() {
  String respuesta = "{";
  respuesta += "\"temperatura\":" + String(temperatura, 1) + ",";
  respuesta += "\"humedad\":" + String(humedad, 1) + ",";
  respuesta += "\"rango\":\"" + rango + "\",";
  respuesta += "\"velocidad\":\"" + velocidad + "\",";
  respuesta += "\"estado\":\"" + estado + "\",";
  respuesta += "\"pwm\":" + String(pwmValue) + ",";
  respuesta += "\"pulsos\":" + String(pulsos);
  respuesta += "}";
  return respuesta;
}

void setup() {
  Serial.begin(9600);
  dht.begin();

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(encoderA, INPUT_PULLUP);
  pinMode(encoderB, INPUT_PULLUP);

  estadoAnteriorA = digitalRead(encoderA);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

void loop() {
  int estadoActualA = digitalRead(encoderA);

  if (estadoActualA != estadoAnteriorA) {
    if (estadoActualA == HIGH) {
      pulsos++;
    }
  }
  estadoAnteriorA = estadoActualA;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    humedad = h;
    temperatura = t;
    actualizarControl(temperatura);
  }

  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      buffer.trim();

      if (buffer == "GET_DATA") {
        Serial.println(crearRespuesta());
      }

      buffer = "";
    } else {
      buffer += c;
    }
  }

  if (millis() - tiempoSerial >= 1000) {
    tiempoSerial = millis();

    Serial.print("Temp: ");
    Serial.print(temperatura);
    Serial.print(" C | Humedad: ");
    Serial.print(humedad);
    Serial.print(" % | ");
    Serial.print(rango);
    Serial.print(" | Velocidad: ");
    Serial.print(velocidad);
    Serial.print(" | PWM: ");
    Serial.print(pwmValue);
    Serial.print(" | Pulsos: ");
    Serial.println(pulsos);
  }

  delay(100);
}
