#include <Servo.h>

#define COLOR_S0   7
#define COLOR_S1   8
#define COLOR_S2   5
#define COLOR_S3   6
#define COLOR_OUT  4

#define ENCODER_A 3
#define ENCODER_B 2

#define MOTOR_CCW 12
#define MOTOR_CW  11
#define MOTOR_PWM 10

#define SERVO_PIN 9

#define BOTON_PARO 13

const bool INVERTIR_MOTOR = false;
const bool INVERTIR_ENCODER = false;

const long POS_ROJO = -900;
const long POS_VERDE = 0;
const long POS_AZUL = 900;

const long POS_CENTRO = 0;

const long LIMITE_SEGURIDAD_IZQUIERDA = -1100;
const long LIMITE_SEGURIDAD_DERECHA = 1100;

const int TOLERANCIA_POSICION = 15;

const unsigned long TIEMPO_ACOMODAR_PIEZA = 1500;

const int PWM_MAX = 200;
const int PWM_MIN = 75;
const int RAMPA = 6;

const long ZONA_LENTA = 200;

volatile long posicion = 0;

long setpoint = POS_CENTRO;

int pwmActual = 0;
int direccionActual = 0;

bool movimientoActivo = false;

unsigned long tiempoInicioMovimiento = 0;

const unsigned long TIEMPO_MAX_MOVIMIENTO = 10000;

long errorAnterior = 0;
bool errorAnteriorValido = false;

// =====================================================
// CONTROL PID DE POSICION
// =====================================================

// Valores nuevos exclusivamente para el PID.
// Todos los valores originales del programa se conservan.
const float KP = 1.8;
const float KI = 0.02;
const float KD = 0.15;

const unsigned long TIEMPO_MUESTREO_PID = 20;

float integralPID = 0.0;
long errorPIDAnterior = 0;
unsigned long tiempoPIDAnterior = 0;
bool pidInicializado = false;

const int SERVO_REPOSO = 95;

const int SERVO_ACTIVO_COLORES = 170;

const int SERVO_ACTIVO_NEGRO = 20;

const unsigned long TIEMPO_SERVO_ACTIVO = 500;
const unsigned long TIEMPO_SERVO_REPOSO = 300;

Servo servoMotor;

bool servoPendiente = false;

bool regresoCentroPendiente = false;

bool regresandoAlCentro = false;

const int NUM_MUESTRAS = 20;
const int TIEMPO_ESTABILIZACION = 15;
const int TIEMPO_ENTRE_LECTURAS = 400;

const unsigned long TIMEOUT_SENSOR = 50000;

const int TOLERANCIA_COLOR = 9;

const int LECTURAS_CONFIRMACION = 2;

const int BASE_R_MIN = 75;
const int BASE_R_MAX = 85;

const int BASE_G_MIN = 75;
const int BASE_G_MAX = 95;

const int BASE_B_MIN = 70;
const int BASE_B_MAX = 80;

const int ROJO_R_MIN = 110;
const int ROJO_R_MAX = 115;

const int ROJO_G_MIN = 180;
const int ROJO_G_MAX = 190;

const int ROJO_B_MIN = 150;
const int ROJO_B_MAX = 160;

const int VERDE_R_MIN = 140;
const int VERDE_R_MAX = 150;

const int VERDE_G_MIN = 110;
const int VERDE_G_MAX = 120;

const int VERDE_B_MIN = 110;
const int VERDE_B_MAX = 115;

const int AZUL_R_MIN = 170;
const int AZUL_R_MAX = 200;

const int AZUL_G_MIN = 140;
const int AZUL_G_MAX = 170;

const int AZUL_B_MIN = 90;
const int AZUL_B_MAX = 115;

const int NEGRO_R_MIN = 220;
const int NEGRO_R_MAX = 230;

const int NEGRO_G_MIN = 200;
const int NEGRO_G_MAX = 215;

const int NEGRO_B_MIN = 160;
const int NEGRO_B_MAX = 180;

enum TipoColor {
  COLOR_DESCONOCIDO,
  COLOR_SIN_PIEZA,
  COLOR_ROJO,
  COLOR_VERDE,
  COLOR_AZUL,
  COLOR_NEGRO
};

unsigned long lecturaRojo = 0;
unsigned long lecturaVerde = 0;
unsigned long lecturaAzul = 0;

TipoColor ultimoColorLeido = COLOR_DESCONOCIDO;
TipoColor colorPendiente = COLOR_DESCONOCIDO;

int contadorColorEstable = 0;

bool piezaProcesada = false;

bool esperandoAntesMovimiento = false;

unsigned long tiempoDeteccionPieza = 0;

long destinoPendiente = POS_CENTRO;

unsigned long tiempoAnteriorColor = 0;
unsigned long tiempoAnteriorPosicion = 0;

void setup() {

  Serial.begin(9600);

  pinMode(COLOR_S0, OUTPUT);
  pinMode(COLOR_S1, OUTPUT);
  pinMode(COLOR_S2, OUTPUT);
  pinMode(COLOR_S3, OUTPUT);
  pinMode(COLOR_OUT, INPUT);

  digitalWrite(COLOR_S0, HIGH);
  digitalWrite(COLOR_S1, LOW);

  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(ENCODER_A),
    leerEncoder,
    CHANGE
  );

  pinMode(MOTOR_CCW, OUTPUT);
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_PWM, OUTPUT);

  detenerMotor();

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(SERVO_REPOSO);

  pinMode(BOTON_PARO, INPUT_PULLUP);

  delay(500);

  noInterrupts();
  posicion = POS_CENTRO;
  interrupts();

  setpoint = POS_CENTRO;

  Serial.println();
  Serial.println("========================================");
  Serial.println("CLASIFICADOR AUTOMATICO INICIADO");
  Serial.println("========================================");
  Serial.println("ROJO  -> -900 -> SERVO -> 0");
  Serial.println("VERDE -> 0    -> SERVO -> 0");
  Serial.println("AZUL  -> 900  -> SERVO -> 0");
  Serial.println("NEGRO -> -900 -> SERVO CONTRARIO -> 0");
  Serial.println("========================================");
  Serial.println("FUNCIONAMIENTO CONTINUO ACTIVADO");
  Serial.println("El carro debe iniciar fisicamente en 0.");
}

void loop() {

  if (revisarBotonParo()) {
    return;
  }

  gestionarEsperaMovimiento();

  controlarMotor();

  if (
    movimientoActivo &&
    millis() - tiempoAnteriorPosicion >= 100
  ) {

    tiempoAnteriorPosicion = millis();

    Serial.print("Posicion: ");
    Serial.print(leerPosicion());

    Serial.print(" | Destino: ");
    Serial.print(setpoint);

    Serial.print(" | PWM: ");
    Serial.println(pwmActual);
  }

  if (
    !movimientoActivo &&
    !esperandoAntesMovimiento &&
    millis() - tiempoAnteriorColor >=
    TIEMPO_ENTRE_LECTURAS
  ) {

    tiempoAnteriorColor = millis();

    leerYProcesarColor();
  }
}

void leerYProcesarColor() {

  lecturaRojo = leerFiltro(LOW, LOW);
  lecturaVerde = leerFiltro(HIGH, HIGH);
  lecturaAzul = leerFiltro(LOW, HIGH);

  TipoColor colorDetectado = detectarColor(
    lecturaRojo,
    lecturaVerde,
    lecturaAzul
  );

  Serial.print("R: ");
  Serial.print(lecturaRojo);

  Serial.print(" | G: ");
  Serial.print(lecturaVerde);

  Serial.print(" | B: ");
  Serial.print(lecturaAzul);

  Serial.print(" | COLOR: ");
  Serial.println(nombreColor(colorDetectado));

  if (colorDetectado == ultimoColorLeido) {

    if (contadorColorEstable < LECTURAS_CONFIRMACION) {
      contadorColorEstable++;
    }

  } else {

    ultimoColorLeido = colorDetectado;
    contadorColorEstable = 1;
  }

  if (contadorColorEstable < LECTURAS_CONFIRMACION) {
    return;
  }

  if (colorDetectado == COLOR_SIN_PIEZA) {

    piezaProcesada = false;

    return;
  }

  if (piezaProcesada) {
    return;
  }

  if (colorDetectado == COLOR_ROJO) {

    piezaProcesada = true;

    programarMovimiento(
      POS_ROJO,
      COLOR_ROJO
    );

    return;
  }

  if (colorDetectado == COLOR_VERDE) {

    piezaProcesada = true;

    programarMovimiento(
      POS_VERDE,
      COLOR_VERDE
    );

    return;
  }

  if (colorDetectado == COLOR_AZUL) {

    piezaProcesada = true;

    programarMovimiento(
      POS_AZUL,
      COLOR_AZUL
    );

    return;
  }

  if (colorDetectado == COLOR_NEGRO) {

    piezaProcesada = true;

    programarMovimiento(
      POS_ROJO,
      COLOR_NEGRO
    );

    return;
  }
}

void programarMovimiento(
  long destino,
  TipoColor color
) {

  destinoPendiente = destino;
  colorPendiente = color;

  tiempoDeteccionPieza = millis();

  esperandoAntesMovimiento = true;

  Serial.print("Color confirmado: ");
  Serial.println(nombreColor(color));

  Serial.print("Esperando ");
  Serial.print(TIEMPO_ACOMODAR_PIEZA);
  Serial.println(" ms para acomodar la pieza.");

  Serial.print("Destino: ");
  Serial.println(destino);
}

void gestionarEsperaMovimiento() {

  if (!esperandoAntesMovimiento) {
    return;
  }

  if (
    millis() - tiempoDeteccionPieza <
    TIEMPO_ACOMODAR_PIEZA
  ) {

    return;
  }

  esperandoAntesMovimiento = false;

  Serial.print("Moviendo pieza ");
  Serial.print(nombreColor(colorPendiente));

  Serial.print(" hacia ");
  Serial.println(destinoPendiente);

  regresandoAlCentro = false;

  iniciarMovimiento(
    destinoPendiente,
    true,
    true
  );
}

void iniciarMovimiento(
  long nuevaPosicion,
  bool activarServoAlLlegar,
  bool volverAlCentro
) {

  long posicionActual = leerPosicion();

  setpoint = nuevaPosicion;

  servoPendiente = activarServoAlLlegar;
  regresoCentroPendiente = volverAlCentro;

  long error = setpoint - posicionActual;
  long distancia = labs(error);

  errorAnterior = error;
  errorAnteriorValido = true;

  // Reiniciar el PID cada vez que comienza un movimiento.
  integralPID = 0.0;
  errorPIDAnterior = error;
  tiempoPIDAnterior = millis();
  pidInicializado = true;

  if (distancia <= TOLERANCIA_POSICION) {

    detenerMotor();

    movimientoActivo = false;
    errorAnteriorValido = false;

    Serial.println("El motor ya se encuentra en el destino.");

    finalizarDestino();

    return;
  }

  pwmActual = 0;
  direccionActual = 0;

  tiempoInicioMovimiento = millis();

  movimientoActivo = true;
}

void finalizarDestino() {

  long destinoAlcanzado = setpoint;

  if (!regresandoAlCentro) {

    bool debeActivarServo = servoPendiente;
    bool debeRegresarCentro = regresoCentroPendiente;

    servoPendiente = false;
    regresoCentroPendiente = false;

    if (debeActivarServo) {

      Serial.println("Activando servo para tirar la pieza.");

      moverServo();
    }

    if (
      debeRegresarCentro &&
      destinoAlcanzado != POS_CENTRO
    ) {

      Serial.println("Regresando automaticamente a cero.");

      regresandoAlCentro = true;

      iniciarMovimiento(
        POS_CENTRO,
        false,
        false
      );

      return;
    }
  }

  regresandoAlCentro = false;
  regresoCentroPendiente = false;
  servoPendiente = false;

  detenerMotor();

  servoMotor.write(SERVO_REPOSO);

  piezaProcesada = false;

  ultimoColorLeido = COLOR_DESCONOCIDO;
  colorPendiente = COLOR_DESCONOCIDO;

  contadorColorEstable = 0;

  tiempoAnteriorColor = millis();

  Serial.println();
  Serial.println("========================================");
  Serial.println("CICLO TERMINADO");
  Serial.println("Motor en posicion cero");
  Serial.println("Servo en posicion inicial");
  Serial.println("Sistema listo para otra pieza");
  Serial.println("========================================");
}

void controlarMotor() {

  if (!movimientoActivo) {
    return;
  }

  long posicionActual = leerPosicion();

  long error = setpoint - posicionActual;
  long distancia = labs(error);

  bool cruzoDestino = false;

  if (errorAnteriorValido) {

    if (
      errorAnterior > 0 &&
      error < 0
    ) {

      cruzoDestino = true;
    }

    if (
      errorAnterior < 0 &&
      error > 0
    ) {

      cruzoDestino = true;
    }
  }

  if (
    distancia <= TOLERANCIA_POSICION ||
    cruzoDestino
  ) {

    detenerMotor();

    movimientoActivo = false;
    errorAnteriorValido = false;
    pidInicializado = false;

    Serial.print("Destino alcanzado. Posicion final: ");
    Serial.println(posicionActual);

    if (cruzoDestino) {

      Serial.println(
        "El motor cruzo ligeramente el destino."
      );
    }

    finalizarDestino();

    return;
  }

  errorAnterior = error;
  errorAnteriorValido = true;

  if (
    posicionActual <= LIMITE_SEGURIDAD_IZQUIERDA ||
    posicionActual >= LIMITE_SEGURIDAD_DERECHA
  ) {

    detenerMotor();

    movimientoActivo = false;

    servoPendiente = false;
    regresoCentroPendiente = false;
    regresandoAlCentro = false;

    errorAnteriorValido = false;
    pidInicializado = false;

    piezaProcesada = false;
    contadorColorEstable = 0;
    ultimoColorLeido = COLOR_DESCONOCIDO;

    Serial.println("PARO: limite de seguridad.");

    return;
  }

  if (
    millis() - tiempoInicioMovimiento >=
    TIEMPO_MAX_MOVIMIENTO
  ) {

    detenerMotor();

    movimientoActivo = false;

    servoPendiente = false;
    regresoCentroPendiente = false;
    regresandoAlCentro = false;

    errorAnteriorValido = false;
    pidInicializado = false;

    piezaProcesada = false;
    contadorColorEstable = 0;
    ultimoColorLeido = COLOR_DESCONOCIDO;

    Serial.println("PARO: tiempo maximo.");
    Serial.println("Revisa encoder y direcciones.");

    return;
  }

  // ===================================================
  // CALCULO PID
  // ===================================================

  unsigned long tiempoActualPID = millis();

  if (!pidInicializado) {

    integralPID = 0.0;
    errorPIDAnterior = error;
    tiempoPIDAnterior = tiempoActualPID;
    pidInicializado = true;
  }

  unsigned long tiempoTranscurridoPID =
    tiempoActualPID - tiempoPIDAnterior;

  // Actualizar el PID solamente cada 20 ms.
  if (tiempoTranscurridoPID < TIEMPO_MUESTREO_PID) {
    return;
  }

  float dt = tiempoTranscurridoPID / 1000.0;

  // Termino integral.
  integralPID += error * dt;

  // Termino derivativo.
  float derivadaPID =
    (error - errorPIDAnterior) / dt;

  // Salida PID con signo.
  float salidaPID =
    (KP * error) +
    (KI * integralPID) +
    (KD * derivadaPID);

  // Antiwindup:
  // si la salida rebasa el PWM maximo, se cancela
  // la ultima suma realizada al termino integral.
  if (
    salidaPID > PWM_MAX ||
    salidaPID < -PWM_MAX
  ) {

    integralPID -= error * dt;

    salidaPID =
      (KP * error) +
      (KI * integralPID) +
      (KD * derivadaPID);
  }

  errorPIDAnterior = error;
  tiempoPIDAnterior = tiempoActualPID;

  // La direccion sigue dependiendo del error de posicion.
  int direccion;

  if (error > 0) {
    direccion = 1;
  } else {
    direccion = -1;
  }

  if (direccion != direccionActual) {

    analogWrite(MOTOR_PWM, 0);

    digitalWrite(MOTOR_CW, LOW);
    digitalWrite(MOTOR_CCW, LOW);

    pwmActual = 0;
    direccionActual = direccion;
  }

  // Si la derivada intenta mandar el motor en sentido
  // contrario al error, se reduce la salida al minimo.
  if (
    (error > 0 && salidaPID < 0) ||
    (error < 0 && salidaPID > 0)
  ) {

    salidaPID = 0;
  }

  int pwmObjetivo = (int)fabs(salidaPID);

  pwmObjetivo = constrain(
    pwmObjetivo,
    PWM_MIN,
    PWM_MAX
  );

  // Se conserva la rampa original.
  if (pwmActual < pwmObjetivo) {

    pwmActual += RAMPA;

    if (pwmActual > pwmObjetivo) {
      pwmActual = pwmObjetivo;
    }
  }

  else if (pwmActual > pwmObjetivo) {

    pwmActual -= RAMPA;

    if (pwmActual < pwmObjetivo) {
      pwmActual = pwmObjetivo;
    }
  }

  pwmActual = constrain(
    pwmActual,
    0,
    PWM_MAX
  );

  bool sentidoPositivo = direccion > 0;

  if (INVERTIR_MOTOR) {
    sentidoPositivo = !sentidoPositivo;
  }

  if (sentidoPositivo) {

    digitalWrite(MOTOR_CW, HIGH);
    digitalWrite(MOTOR_CCW, LOW);

  } else {

    digitalWrite(MOTOR_CW, LOW);
    digitalWrite(MOTOR_CCW, HIGH);
  }

  analogWrite(MOTOR_PWM, pwmActual);
}


void moverServo() {

  int anguloActivo;

  if (colorPendiente == COLOR_NEGRO) {

    anguloActivo = SERVO_ACTIVO_NEGRO;
    Serial.println("Servo NEGRO: giro contrario.");

  } else {

    anguloActivo = SERVO_ACTIVO_COLORES;
    Serial.println("Servo activado.");
  }

  servoMotor.write(anguloActivo);

  delay(TIEMPO_SERVO_ACTIVO);

  servoMotor.write(SERVO_REPOSO);

  delay(TIEMPO_SERVO_REPOSO);

  Serial.println("Servo regreso a 95 grados.");
}

void detenerMotor() {

  analogWrite(MOTOR_PWM, 0);

  digitalWrite(MOTOR_CW, LOW);
  digitalWrite(MOTOR_CCW, LOW);

  pwmActual = 0;
  direccionActual = 0;

  pidInicializado = false;
  integralPID = 0.0;
}

bool revisarBotonParo() {

  static bool ultimaLectura = HIGH;
  static bool estadoEstable = HIGH;

  static unsigned long tiempoUltimoCambio = 0;

  static bool regresarAlCentroBoton = false;

  bool lecturaActual = digitalRead(BOTON_PARO);

  if (lecturaActual == LOW) {

    detenerMotor();

    movimientoActivo = false;
    esperandoAntesMovimiento = false;

    servoPendiente = false;
    regresoCentroPendiente = false;
    regresandoAlCentro = false;

    errorAnteriorValido = false;
  }

  if (lecturaActual != ultimaLectura) {

    ultimaLectura = lecturaActual;
    tiempoUltimoCambio = millis();
  }

  if (
    millis() - tiempoUltimoCambio >= 40 &&
    lecturaActual != estadoEstable
  ) {

    estadoEstable = lecturaActual;

    if (estadoEstable == LOW) {

      detenerMotor();

      movimientoActivo = false;
      esperandoAntesMovimiento = false;

      servoPendiente = false;
      regresoCentroPendiente = false;
      regresandoAlCentro = false;

      errorAnteriorValido = false;

      setpoint = leerPosicion();

      regresarAlCentroBoton = true;

      Serial.println();
      Serial.println("BOTON DE PARO PRESIONADO.");
      Serial.println("Motor detenido.");
      Serial.println("El cero no fue modificado.");
    }

    else if (regresarAlCentroBoton) {

      regresarAlCentroBoton = false;

      Serial.println("Boton liberado.");
      Serial.println("Regresando al centro.");

      regresandoAlCentro = true;

      iniciarMovimiento(
        POS_CENTRO,
        false,
        false
      );
    }
  }

  return (
    estadoEstable == LOW ||
    lecturaActual == LOW
  );
}

void leerEncoder() {

  int estadoA = digitalRead(ENCODER_A);
  int estadoB = digitalRead(ENCODER_B);

  int cambio;

  if (estadoA == estadoB) {
    cambio = 1;
  } else {
    cambio = -1;
  }

  if (INVERTIR_ENCODER) {
    cambio = -cambio;
  }

  posicion += cambio;
}

long leerPosicion() {

  long copiaPosicion;

  noInterrupts();

  copiaPosicion = posicion;

  interrupts();

  return copiaPosicion;
}

unsigned long leerFiltro(
  byte estadoS2,
  byte estadoS3
) {

  digitalWrite(COLOR_S2, estadoS2);
  digitalWrite(COLOR_S3, estadoS3);

  delay(TIEMPO_ESTABILIZACION);

  pulseIn(
    COLOR_OUT,
    LOW,
    TIMEOUT_SENSOR
  );

  unsigned long suma = 0;
  int lecturasValidas = 0;

  for (int i = 0; i < NUM_MUESTRAS; i++) {

    unsigned long lectura = pulseIn(
      COLOR_OUT,
      LOW,
      TIMEOUT_SENSOR
    );

    if (lectura > 0) {

      suma += lectura;
      lecturasValidas++;
    }

    delay(2);
  }

  if (lecturasValidas == 0) {
    return 0;
  }

  return suma / lecturasValidas;
}

bool estaEnRango(
  unsigned long valor,
  int minimo,
  int maximo
) {

  long limiteMinimo =
    (long)minimo - TOLERANCIA_COLOR;

  long limiteMaximo =
    (long)maximo + TOLERANCIA_COLOR;

  return (
    (long)valor >= limiteMinimo &&
    (long)valor <= limiteMaximo
  );
}

bool coincideConColor(
  unsigned long r,
  unsigned long g,
  unsigned long b,
  int rMin,
  int rMax,
  int gMin,
  int gMax,
  int bMin,
  int bMax
) {

  return (
    estaEnRango(r, rMin, rMax) &&
    estaEnRango(g, gMin, gMax) &&
    estaEnRango(b, bMin, bMax)
  );
}

TipoColor detectarColor(
  unsigned long r,
  unsigned long g,
  unsigned long b
) {

  if (coincideConColor(
        r, g, b,
        BASE_R_MIN, BASE_R_MAX,
        BASE_G_MIN, BASE_G_MAX,
        BASE_B_MIN, BASE_B_MAX
      )) {

    return COLOR_SIN_PIEZA;
  }

  if (coincideConColor(
        r, g, b,
        ROJO_R_MIN, ROJO_R_MAX,
        ROJO_G_MIN, ROJO_G_MAX,
        ROJO_B_MIN, ROJO_B_MAX
      )) {

    return COLOR_ROJO;
  }

  if (coincideConColor(
        r, g, b,
        VERDE_R_MIN, VERDE_R_MAX,
        VERDE_G_MIN, VERDE_G_MAX,
        VERDE_B_MIN, VERDE_B_MAX
      )) {

    return COLOR_VERDE;
  }

  if (coincideConColor(
        r, g, b,
        AZUL_R_MIN, AZUL_R_MAX,
        AZUL_G_MIN, AZUL_G_MAX,
        AZUL_B_MIN, AZUL_B_MAX
      )) {

    return COLOR_AZUL;
  }

  if (coincideConColor(
        r, g, b,
        NEGRO_R_MIN, NEGRO_R_MAX,
        NEGRO_G_MIN, NEGRO_G_MAX,
        NEGRO_B_MIN, NEGRO_B_MAX
      )) {

    return COLOR_NEGRO;
  }

  return COLOR_DESCONOCIDO;
}

const char* nombreColor(TipoColor color) {

  switch (color) {

    case COLOR_SIN_PIEZA:
      return "SIN PIEZA";

    case COLOR_ROJO:
      return "ROJO";

    case COLOR_VERDE:
      return "VERDE";

    case COLOR_AZUL:
      return "AZUL";

    case COLOR_NEGRO:
      return "NEGRO";

    default:
      return "DESCONOCIDO";
  }
}
