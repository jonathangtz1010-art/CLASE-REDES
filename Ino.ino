const int SENSOR_PIN = A0;
const int UMBRAL = 500;
const int HYSTERESIS = 20;          // un poco más amplia para evitar brincos
const unsigned long HOLD_MS = 350;  // tiempo mínimo para confirmar cambio

char bufferLine[40];
byte bufferIndex = 0;

bool activeState = false;
bool pendingState = false;
bool hasPendingChange = false;
unsigned long pendingSince = 0;

int readStableA0() {
  long sum = 0;
  const byte n = 16;   // más muestras = más estabilidad
  for (byte i = 0; i < n; i++) {
    sum += analogRead(SENSOR_PIN);
    delay(2);
  }
  return sum / n;
}

bool computeDesiredState(int value) {
  if (activeState) {
    if (value <= UMBRAL - HYSTERESIS) {
      return false;
    }
    return true;
  } else {
    if (value >= UMBRAL + HYSTERESIS) {
      return true;
    }
    return false;
  }
}

bool updateStateRobust(int value) {
  bool desiredState = computeDesiredState(value);
  unsigned long now = millis();

  if (desiredState == activeState) {
    hasPendingChange = false;
    return activeState;
  }

  if (!hasPendingChange) {
    pendingState = desiredState;
    pendingSince = now;
    hasPendingChange = true;
    return activeState;
  }

  if (pendingState != desiredState) {
    pendingState = desiredState;
    pendingSince = now;
    return activeState;
  }

  if (now - pendingSince >= HOLD_MS) {
    activeState = pendingState;
    hasPendingChange = false;
  }

  return activeState;
}

void printStatus() {
  int value = readStableA0();
  bool active = updateStateRobust(value);

  Serial.print("OK ");
  Serial.print("a0=");
  Serial.print(value);

  Serial.print(" threshold=");
  Serial.print(UMBRAL);

  Serial.print(" red=");
  Serial.print(active ? 1 : 0);

  Serial.print(" green=");
  Serial.print(active ? 1 : 0);

  Serial.print(" state=");
  Serial.print(active ? "ACTIVO" : "REPOSO");

  Serial.print(" mode=ANALOG");
  Serial.print(" input=A0");
  Serial.print(" leds=HARDWARE");
  Serial.print(" driver=TRANSISTOR");
  Serial.println();
}

void handleCommand(char *cmd) {
  for (int i = 0; cmd[i]; i++) {
    cmd[i] = toupper(cmd[i]);
  }

  if (strcmp(cmd, "PING") == 0) {
    Serial.println("OK pong");
    return;
  }

  if (strcmp(cmd, "GET") == 0 || strcmp(cmd, "READ") == 0 || strcmp(cmd, "STATUS") == 0) {
    printStatus();
    return;
  }

  Serial.println("ERR comando_no_valido");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("OK Arduino_A0_listo");
}

void loop() {
  int value = readStableA0();
  updateStateRobust(value);

  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        bufferLine[bufferIndex] = '\0';
        handleCommand(bufferLine);
        bufferIndex = 0;
      }
    } else {
      if (bufferIndex < sizeof(bufferLine) - 1) {
        bufferLine[bufferIndex++] = c;
      }
    }
  }
}
