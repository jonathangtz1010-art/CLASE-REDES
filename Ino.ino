#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// Puente H
const int ENA = 3;
const int IN1 = 4;
const int IN2 = 2;

// Encoder
const int encoderA = 6;   // amarillo
const int encoderB = 5;   // verde

volatile long pulsos = 0;

float temperatura = 0.0;
float humedad = 0.0;
String rango = "";
String velocidad = "";
String estado = "";
int pwmValue = 0;

String buffer = "";

void contarEncoder() {
  pulsos++;
}

void actualizarControl(float temp) {
  if (temp < 24.0) {
    rango = "Rango 1";
    velocidad = "Baja";
    estado = "Temperatura baja";
    pwmValue = 90;
  }
  else if (temp >= 24.0 && temp < 30.0) {
    rango = "Rango 2";
    velocidad = "Media";
    estado = "Temperatura media";
    pwmValue = 170;
  }
  else {
    rango = "Rango 3";
    velocidad = "Alta";
    estado = "Temperatura alta";
    pwmValue = 255;
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

  attachInterrupt(digitalPinToInterrupt(encoderA), contarEncoder, RISING);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}

void loop() {
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

  delay(500);
}
