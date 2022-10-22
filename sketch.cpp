/// Habilitacion de debug para la impresion por el puerto serial
#define SERIAL_DEBUG_ENABLED 1

#if SERIAL_DEBUG_ENABLED
#define DebugPrint(str) \
  { \
    Serial.println(str); \
  }
#else
#define DebugPrint(str)
#endif

#define DebugPrintEstado(estado, evento) \
  { \
    String est = estado; \
    String evt = evento; \
    String str; \
    str = "-----------------------------------------------------"; \
    DebugPrint(str); \
    str = "EST-> [" + est + "]: " + "EVT-> [" + evt + "]."; \
    DebugPrint(str); \
    str = "-----------------------------------------------------"; \
    DebugPrint(str); \
  }

/// ESTADOS DEL EMBEBIDO
#define ESTADO_EMBED_INIT 100
#define ESTADO_EMBED_IDLE 101
#define ESTADO_EMBED_OPEN_SERVING 102
#define ESTADO_EMBED_CLOSED_MEASURING 103
#define ESTADO_EMBED_SERVING 104
#define ESTADO_EMBED_INSUFICIENTE 105

/// ESTADOS DEL LED
#define ESTADO_LED_APAGADO 200
#define ESTADO_LED_PRENDIDO 201

#define ESTADO_LED_SLOW_BLINK_PRENDIDO 202
#define ESTADO_LED_SLOW_BLINK_APAGADO 203

#define ESTADO_LED_FAST_BLINK_PRENDIDO 204
#define ESTADO_LED_FAST_BLINK_APAGADO 205

/// ESTADOS DEL BOTON
#define ESTADO_BOTON_PRESIONADO 1
#define ESTADO_BOTON_SUELTO 0

/// ESTADOS DEL SERVO
#define ESTADO_SERVO_ABIERTO 1
#define ESTADO_SERVO_CERRADO 0

/// ESTADOS DEL FLEX
#define ESTADO_FLEX_PORCION_SERVIDA 1
#define ESTADO_FLEX_PORCION_FALTANTE 0

/// EVENTOS
#define EVENTO_CONTINUE 1000
#define EVENTO_PESO_PORCION_FALTA 2000
#define EVENTO_PESO_PORCION_COMPLETA 3000
#define EVENTO_PESO_PORCION_INSUFICIENTE 4000
#define EVENTO_PRESENCIA_DETECTADA 5000
#define EVENTO_OPEN_SERVING_TIMEOUT 6000
#define EVENTO_PORCION_SERVIDA 7000

/// PINES
#define PIN_PULSADOR 2
#define PIN_LED 3
#define PIN_SERVO_1 7
#define PIN_SERVO_2 6
#define PIN_FLEX A0
#define PIN_DISTANCIA A1

#define SERVO_OPEN_POSITION 0
#define SERVO_CLOSED_POSITION 180

/// UMBRALES
#define UMBRAL_TIMEOUT 1000 // Para correr una vez por segundo
#define UMBRAL_LED_FAST_BLINK_TIMEOUT 200 // Estos numeros son grandes pero probablemente tengan que ser mucho mas chicos en el arduino normal
#define UMBRAL_LED_SLOW_BLINK_TIMEOUT 500
#define UMBRAL_PESO_PORCION 900
#define UMBRAL_PRESENCIA_MAXIMA 100
#define UMBRAL_PROCESO_PORCION 1500
#define UMBRAL_PROCESO_SERVING 3000

// INCLUDES
#include <Servo.h>
#include <SoftwareSerial.h>

/// TIPOS
typedef struct stSensor {
  int pin;
  long valor_actual;
  long valor_previo;
  int estado;
} stSensor;
typedef struct stLed {
  int pin;
  int estado;

  long ultima_lectura_millis; // Esto va a ser usado para hacer un blink rapido o lento.
  bool timeout;
} stLed;
typedef struct stPulsador {
  int estado;
} stPulsador;
typedef int stEvento;
typedef int stEstado;
typedef struct stServo {
  Servo servo;
  int estado_servo;
} stServo;

/// VARIABLES GLOBALES
stEvento evento;  // Objeto con el ultimo evento ocurrido
volatile stEstado estado;  // Objeto estado con el estado actual del embebido
stSensor sensor_distancia;
stSensor sensor_flex;
stPulsador pulsador;
stLed led;
bool timeout;  // Timeout para el ciclo de la maquina de estado
long ultima_lectura_millis;
long ultima_lectura_millis_proceso_servir;
long ultima_lectura_millis_proceso_porcion; // Ultima lectura para medir el timing de los servos.
long ultima_lectura_millis_proceso_puerta;
stServo servo_porcion;  // Objeto servo para manejar puerta de porcion.
stServo servo_puerta;
SoftwareSerial BTSerial(10,11); // Objeto software serial para el modulo bluetooth en los pines 10 y 11.

void do_init() {
  Serial.begin(9600);
  BTSerial.begin(9600);

  timeout = false;
  ultima_lectura_millis = millis();

  pinMode(PIN_PULSADOR, INPUT);  
  attachInterrupt(digitalPinToInterrupt(PIN_PULSADOR), interrupt_pulsador, CHANGE);

  servo_porcion.servo.attach(PIN_SERVO_1);
  servo_puerta.servo.attach(PIN_SERVO_2);
  servo_porcion.estado_servo = ESTADO_SERVO_CERRADO;
  servo_puerta.estado_servo = ESTADO_SERVO_CERRADO;
  
  sensor_distancia.pin = PIN_DISTANCIA;

  sensor_flex.pin = PIN_FLEX;
  sensor_flex.estado = ESTADO_FLEX_PORCION_FALTANTE; // todo repensarlo

  led.pin = PIN_LED;
  led.estado = ESTADO_LED_APAGADO;
  pinMode(PIN_LED, OUTPUT);

  pulsador.estado = ESTADO_BOTON_SUELTO;

  estado = ESTADO_EMBED_INIT;
}

void setup() {
  do_init();
}

void loop() {
  maquina_estado();
}

bool detectar_eventos_flex(int lectura_millis) {
  if (servo_puerta.estado_servo == ESTADO_SERVO_ABIERTO || servo_porcion.estado_servo == ESTADO_SERVO_ABIERTO) {
    return false;
  }

  if (sensor_flex.valor_actual < UMBRAL_PESO_PORCION) {
    sensor_flex.estado = ESTADO_FLEX_PORCION_FALTANTE;
    ultima_lectura_millis_proceso_porcion = lectura_millis;
    evento = EVENTO_PESO_PORCION_FALTA;
    return true;
  }

  if (sensor_flex.valor_actual >= UMBRAL_PESO_PORCION && sensor_flex.estado != ESTADO_FLEX_PORCION_SERVIDA) {
    sensor_flex.estado = ESTADO_FLEX_PORCION_SERVIDA;
    evento = EVENTO_PESO_PORCION_COMPLETA;
    return true;
  }

  return false;
}

bool detectar_eventos_distancia(int lectura_millis) {
  bool timeout_proceso = false;
  int diferencia;
  if (servo_porcion.estado_servo == ESTADO_SERVO_ABIERTO || servo_puerta.estado_servo == ESTADO_SERVO_ABIERTO) {
    return false;
  }
  if (sensor_distancia.valor_actual < UMBRAL_PRESENCIA_MAXIMA) {
    // Chequeamos el timeout para el proceso de servir que se resetea una vez
    // que se sirve la primera comida.
    // diferencia = (lectura_millis - ultima_lectura_millis_proceso_servir);
    diferencia = 1;
    timeout_proceso = (diferencia >= UMBRAL_PROCESO_SERVING) ? (true) : (false);
    if (timeout_proceso) {
      ultima_lectura_millis_proceso_puerta = lectura_millis;
      evento = EVENTO_PRESENCIA_DETECTADA;
      return true;
    }
  }

  return false;
}

bool detectar_eventos_servo_porcion(int lectura_millis) {
  bool timeout_proceso = false;
  int diferencia;
  if (servo_porcion.estado_servo == ESTADO_SERVO_ABIERTO) {
    diferencia = (lectura_millis - ultima_lectura_millis_proceso_porcion);
    timeout_proceso = (diferencia > UMBRAL_PROCESO_PORCION) ? (true) : (false);
    if (timeout_proceso) {
      evento = EVENTO_OPEN_SERVING_TIMEOUT;
      return true;
    }
  }
  return false;
}

bool detectar_eventos_servo_puerta(int lectura_millis) {
  bool timeout_proceso = false;
  int diferencia;
  if (servo_puerta.estado_servo == ESTADO_SERVO_ABIERTO) {
    diferencia = (lectura_millis - ultima_lectura_millis_proceso_puerta);
    timeout_proceso = (diferencia > UMBRAL_PROCESO_SERVING) ? (true) : (false);
    if (timeout_proceso) {
      // Cuando cerramos el servo de la puerta queremos esperar
      // a que se cumpla el tiempo para poder volver a servir nuevamente.
      ultima_lectura_millis_proceso_servir = lectura_millis;
      evento = EVENTO_PORCION_SERVIDA;
      return true;
    }
  }
  return false;
}


bool detectar_eventos(int lectura_millis) {
  int diferencia;
  bool timeout_proceso = false;

  if (detectar_eventos_flex(lectura_millis)) {
    return true;
  }

  if (detectar_eventos_distancia(lectura_millis)) {
    return true;
  }

  if (detectar_eventos_servo_porcion(lectura_millis)) {
    return true;
  }

  if (detectar_eventos_servo_puerta(lectura_millis)) {
    return true;
  }

  if (pulsador.estado == ESTADO_BOTON_PRESIONADO || leerBluetooth()==1) {
    ultima_lectura_millis_proceso_puerta = lectura_millis;
    pulsador.estado = ESTADO_BOTON_SUELTO;
    evento = EVENTO_PRESENCIA_DETECTADA;
    return true;
  }

  pulsador.estado = ESTADO_BOTON_SUELTO;
  return false;
}

int leerBluetooth(){
  int returnValue = 0;
  char inputBT = 'z';
  if (Serial.available()) {
    inputBT = Serial.read();
  }
  if(inputBT == 'a') {
    returnValue = 1;
  }
  return returnValue;
}

void leer_sensores() {
  leer_sensor_distancia();
  leer_sensor_peso();
}

void generar_evento() {
  long lectura_millis = millis();
  long diferencia = lectura_millis - ultima_lectura_millis;
  timeout = (diferencia > UMBRAL_TIMEOUT) ? (true) : (false);

  if (timeout) {
    timeout = false;
    ultima_lectura_millis = lectura_millis;

    leer_sensores();
    if (detectar_eventos(lectura_millis)) {
      return;
    }
  }
  
  evento = EVENTO_CONTINUE;
}

void maquina_estado() {
  generar_evento();

  switch (estado) {
    case ESTADO_EMBED_INIT:
      {
        switch (evento) {
          case EVENTO_CONTINUE:
            {
              DebugPrintEstado("ESTADO_EMBED_INIT", "ESTADO CONTINUE");
              servo_puerta.servo.write(SERVO_CLOSED_POSITION);
              servo_porcion.servo.write(SERVO_CLOSED_POSITION);
              estado = ESTADO_EMBED_IDLE;
              led.estado = ESTADO_LED_PRENDIDO;
              manejar_led();
              break;
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_INIT", "Evento desconocido");
              break;
            }
        }
        break;
      }
    case ESTADO_EMBED_IDLE:
      {
        switch (evento) {
          case EVENTO_CONTINUE:
            {
              DebugPrintEstado("ESTADO_EMBED_IDLE", "EVENTO_CONTINUE");
              manejar_led();
              break;
            }
          case EVENTO_PESO_PORCION_FALTA:
            {
              DebugPrintEstado("ESTADO_EMBED_IDLE", "EVENTO_PESO_PORCION_FALTA");
              led.estado = ESTADO_LED_FAST_BLINK_PRENDIDO;
              manejar_led();
              estado = ESTADO_EMBED_OPEN_SERVING;
              break;
            }
          case EVENTO_PRESENCIA_DETECTADA:
            {
              DebugPrintEstado("ESTADO_EMBED_IDLE", "EVENTO_PRESENCIA_DETECTADA");
              led.estado = ESTADO_LED_FAST_BLINK_PRENDIDO;
              manejar_led();
              estado = ESTADO_EMBED_SERVING;
              break;
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_IDLE", "Evento desconocido");
              break;
            }
        }
        break;
      }
    case ESTADO_EMBED_OPEN_SERVING:
      {
        switch (evento) {
          case EVENTO_CONTINUE:
            {
              // manejar_led();
              DebugPrintEstado("ESTADO_EMBED_OPEN_SERVING", "EVENTO_CONTINUE");
              manejar_led();
              servo_porcion.servo.write(SERVO_OPEN_POSITION);
              servo_porcion.estado_servo = ESTADO_SERVO_ABIERTO;
              break;   
            }
          case EVENTO_OPEN_SERVING_TIMEOUT:
            {
              DebugPrintEstado("ESTADO_EMBED_OPEN_SERVING", "EVENTO_OPEN_SERVING_TIMEOUT");
              manejar_led();
              servo_porcion.servo.write(SERVO_CLOSED_POSITION);
              servo_porcion.estado_servo = ESTADO_SERVO_CERRADO;
              estado = ESTADO_EMBED_CLOSED_MEASURING;
              break;
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_OPEN_SERVING", "Evento desconocido");
              break;              
            }
        }
        break;
      }
    case ESTADO_EMBED_CLOSED_MEASURING:
      {
        switch (evento) {
          case EVENTO_CONTINUE:
            {
              DebugPrintEstado("ESTADO_EMBED_CLOSED_MEASURING", "EVENTO_CONTINUE");
              manejar_led();
              break;   
            }
          case EVENTO_PESO_PORCION_COMPLETA:
            {
              DebugPrintEstado("ESTADO_EMBED_CLOSED_MEASURING", "EVENTO_PESO_PORCION_COMPLETA");
              manejar_led();
              led.estado = ESTADO_LED_PRENDIDO;
              estado = ESTADO_EMBED_IDLE;
              BTSerial.write("****** PORCION COMPLETA****** \n\n");
              break;
            }
          case EVENTO_PESO_PORCION_INSUFICIENTE:
            {
              DebugPrintEstado("ESTADO_EMBED_CLOSED_MEASURING", "EVENTO_PESO_PORCION_INSUFICIENTE");
              manejar_led();
              led.estado = ESTADO_LED_SLOW_BLINK_PRENDIDO;
              estado = ESTADO_EMBED_INSUFICIENTE;
              break;
            }
          case EVENTO_PESO_PORCION_FALTA:
            {
              DebugPrintEstado("ESTADO_EMBED_CLOSED_MEASURING", "EVENTO_PESO_PORCION_FALTA");
              manejar_led();
              estado = ESTADO_EMBED_OPEN_SERVING;
              break;
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_CLOSED_MEASURING", "Evento desconocido");
              break;              
            }
        }
        break;
      }
    case ESTADO_EMBED_SERVING:
      {
        switch (evento) {
          case EVENTO_CONTINUE:
            {
              DebugPrintEstado("ESTADO_EMBED_SERVING", "EVENTO_CONTINUE");
              manejar_led();
              servo_puerta.servo.write(SERVO_OPEN_POSITION);
              servo_puerta.estado_servo = ESTADO_SERVO_ABIERTO;
              // manejar_led();
              break;   
            }
          case EVENTO_PORCION_SERVIDA:
            {
              DebugPrintEstado("ESTADO_EMBED_SERVING", "EVENTO_PORCION_SERVIDA");
              manejar_led();
              servo_puerta.servo.write(SERVO_CLOSED_POSITION);
              servo_puerta.estado_servo = ESTADO_SERVO_CERRADO;
              estado = ESTADO_EMBED_IDLE;              
              break;   
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_SERVING", "Evento desconocido");
              break;              
            }
        }
        break;
      }
    case ESTADO_EMBED_INSUFICIENTE:
      {
        switch (evento) {
          case EVENTO_CONTINUE:
            {
              DebugPrintEstado("ESTADO_EMBED_INSUFICIENTE", "EVENTO_CONTINUE");
              manejar_led();
              break;   
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_INSUFICIENTE", "Evento desconocido");
              break;              
            } 
        }
        break;
      }
    default:
      {
        DebugPrintEstado("ESTADO DESCONOCIDO", "");
      }
  }
}

void leer_sensor_distancia() {
  long tiempo_pulso;
  long distancia;
  long velocidad_sonido = 29; // 1 cm cada 29 nanosegundos
  long factor_distancia = 2; // Dividimos la distancia por 2 porque el ancho de pulso nos da el tiempo que tarda en ir y volver.

  pinMode(sensor_distancia.pin, OUTPUT);
  digitalWrite(sensor_distancia.pin, LOW);

  delayMicroseconds(2);
  digitalWrite(sensor_distancia.pin, HIGH);

  delayMicroseconds(5);
  digitalWrite(sensor_distancia.pin, LOW);

  pinMode(sensor_distancia.pin, INPUT);
  // dividimos el tiempo de pulso por dos porque 
  tiempo_pulso = pulseIn(sensor_distancia.pin, HIGH);

  // Convierto la medicion en centimetros.
  distancia = tiempo_pulso / velocidad_sonido / factor_distancia;

  sensor_distancia.valor_previo = sensor_distancia.valor_actual;
  sensor_distancia.valor_actual = distancia;
}

void leer_sensor_peso() {
  long lectura = analogRead(sensor_flex.pin);
  sensor_flex.valor_previo = sensor_flex.valor_actual;
  sensor_flex.valor_actual = lectura;
}

void  manejar_led() {
  switch (led.estado) {
    case ESTADO_LED_APAGADO:
      {
        digitalWrite(led.pin, LOW);
        break;        
      }
    case ESTADO_LED_PRENDIDO:
      {
        digitalWrite(led.pin, HIGH);
        break;
      }
    case ESTADO_LED_FAST_BLINK_APAGADO:
      {
        long lectura_millis = millis();
        long diferencia = lectura_millis - led.ultima_lectura_millis;
        led.timeout = (diferencia > UMBRAL_LED_FAST_BLINK_TIMEOUT) ? (true) : (false);
        if (led.timeout) {
          led.estado = ESTADO_LED_FAST_BLINK_PRENDIDO;
          led.ultima_lectura_millis = lectura_millis;
        } else {
          digitalWrite(led.pin, LOW);
        }
        break;
      }
    case ESTADO_LED_FAST_BLINK_PRENDIDO:
      {
        long lectura_millis = millis();
        long diferencia = lectura_millis - led.ultima_lectura_millis;
        led.timeout = (diferencia > UMBRAL_LED_FAST_BLINK_TIMEOUT) ? (true) : (false);
        if (led.timeout) {
          led.estado = ESTADO_LED_FAST_BLINK_APAGADO;
          led.ultima_lectura_millis = lectura_millis;
        } else {
          digitalWrite(led.pin, HIGH);
        }
        break;
      }
    case ESTADO_LED_SLOW_BLINK_APAGADO:
      {
        long lectura_millis = millis();
        long diferencia = lectura_millis - led.ultima_lectura_millis;
        led.timeout = (diferencia > UMBRAL_LED_SLOW_BLINK_TIMEOUT) ? (true) : (false);
        if (led.timeout) {
          led.estado = ESTADO_LED_SLOW_BLINK_PRENDIDO;
          led.ultima_lectura_millis = lectura_millis;
        } else {
          digitalWrite(led.pin, LOW);
        }
        break;
      }
    case ESTADO_LED_SLOW_BLINK_PRENDIDO:
      {
        long lectura_millis = millis();
        long diferencia = lectura_millis - led.ultima_lectura_millis;
        led.timeout = (diferencia > UMBRAL_LED_SLOW_BLINK_TIMEOUT) ? (true) : (false);
        if (led.timeout) {
          led.estado = ESTADO_LED_SLOW_BLINK_APAGADO;
          led.ultima_lectura_millis = lectura_millis;
        } else {
          digitalWrite(led.pin, HIGH);
        }
        break;
      }
    default:
      digitalWrite(led.pin, LOW);
  }
}

void interrupt_pulsador() {
  Serial.println("INTERRUPCION");
  pulsador.estado = ESTADO_BOTON_PRESIONADO;
}