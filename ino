void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(A0));
}

void loop() {
  int humedad = random(0, 101);
  int temperatura = random(0, 51);
  int luz = random(0, 1001);
  int nivel = random(0, 101);
  int proximidad = random(0, 201);

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.print(" | Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" | Luz: ");
  Serial.print(luz);
  Serial.print(" | Nivel: ");
  Serial.print(nivel);
  Serial.print(" | Proximidad: ");
  Serial.println(proximidad);

  delay(1000);
}
