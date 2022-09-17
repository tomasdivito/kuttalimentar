/// Habilitacion de debug para la impresion por el puerto serial
#define SERIAL_DEBUG_ENABLED 0

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
#define ESTADO_EMBED_OPEN 102
#define ESTADO_EMBED_MEASURING 103

/// ESTADOS DEL LED
#define ESTADO_LED_APAGADO 100
#define ESTADO_LED_PRENDIDO 101

#define ESTADO_LED_SLOW_BLINK_PRENDIDO 102
#define ESTADO_LED_SLOW_BLINK_APAGADO 103

#define ESTADO_LED_FAST_BLINK_PRENDIDO 104
#define ESTADO_LED_FAST_BLINK_APAGADO 105

/// ESTADOS DEL BOTON
#define ESTADO_BOTON_PRESIONADO 1
#define ESTADO_BOTON_SUELTO 0

/// EVENTOS
#define EVENTO_CONTINUE 1000
#define EVENTO_PULSADOR_ACTIVADO 2000

/// PINES
#define PIN_PULSADOR 2
#define PIN_LED 3
#define PIN_SERVO 7
#define PIN_FLEX A0
#define PIN_DISTANCIA A1

/// UMBRALES
#define UMBRAL_TIMEOUT 1000 // Para correr una vez por segundo
#define UMBRAL_LED_FAST_BLINK_TIMEOUT 1000 // Estos numeros son grandes pero probablemente tengan que ser mucho mas chicos en el arduino normal
#define UMBRAL_LED_SLOW_BLINK_TIMEOUT 2500
#define UMBRAL_PESO_MAXIMO 10


// INCLUDES
#include <Servo.h>

/// TIPOS
typedef struct stSensor {
  int pin;
  long valor_actual;
  long valor_previo;

  long ultima_lectura_millis;
} stSensor;
typedef struct stLed {
  int pin;
  int estado;

  long ultima_lectura_millis; // Esto va a ser usado para hacer un blink rapido o lento.
  bool timeout;
} stLed;

typedef int stEvento;
typedef int stEstado;

/// VARIABLES GLOBALES
stEvento evento;  // Objeto con el ultimo evento ocurrido
volatile stEstado estado;  // Objeto estado con el estado actual del embebido
stSensor sensor_distancia;
stSensor sensor_flex;
stLed led;
bool timeout;  // Timeout para el ciclo de la maquina de estados
long ultima_lectura_millis;
Servo servo;  // Objeto servo para manejar Puerta.

void do_init() {
  Serial.begin(9600);

  timeout = false;
  ultima_lectura_millis = millis();

  pinMode(PIN_PULSADOR, INPUT);  
  attachInterrupt(digitalPinToInterrupt(PIN_PULSADOR), interrupt_pulsador, CHANGE);

  servo.attach(PIN_SERVO);
  
  sensor_distancia.pin = PIN_DISTANCIA;
  sensor_flex.pin = PIN_FLEX;

  led.pin = PIN_LED;
  led.estado = ESTADO_LED_APAGADO;
  pinMode(PIN_LED, OUTPUT);

  estado = ESTADO_EMBED_INIT;
}

void setup() {
  do_init();
}

void loop() {
  maquina_estado();
}

void leer_sensores() {

  //FLEX_VALUE = analogRead(FLEX_PIN);
  //Serial.println(FLEX_VALUE);
  //SERVO_POSITION = map(FLEX_VALUE, 770, 950, 0, 180);
  //SERVO_POSITION = constrain(SERVO_POSITION, 0, 180);
  //servo.write(SERVO_POSITION);

  /// DEPENDIENDO DEL ESTADO DEL EMBEBIDO PODRIAMOS LEER UNO U OTRO SENSOR
  //   if(leer_sensor_distancia();
  //if(evento_sensor_peso() || evento_sensor_distancia());
}

void generar_evento() {
  long lectura_millis = millis();
  long diferencia = lectura_millis - ultima_lectura_millis;
  timeout = (diferencia > UMBRAL_TIMEOUT) ? (true) : (false);

  if (timeout) {
    timeout = false;
    ultima_lectura_millis = lectura_millis;

    // Procesamiento de los eventos
        
  } else {    
    evento = EVENTO_CONTINUE;
  }
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
              estado = ESTADO_EMBED_IDLE;
              led.estado = ESTADO_LED_PRENDIDO;              
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
              break;
            }
          case EVENTO_PULSADOR_ACTIVADO:
            {
              DebugPrintEstado("ESTADO_EMBED_IDLE", "EVENTO_PULSADOR_ACTIVADO");
              estado = ESTADO_EMBED_OPEN;
              led.estado = ESTADO_LED_FAST_BLINK_APAGADO;
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
    case ESTADO_EMBED_OPEN:
      {
        switch (evento) {
          case EVENTO_CONTINUE:
            {
              DebugPrintEstado("ESTADO_EMBED_OPEN", "EVENTO_CONTINUE");
              break;   
            }
          case EVENTO_PULSADOR_ACTIVADO:
            {
              DebugPrintEstado("ESTADO_EMBED_OPEN", "EVENTO_PULSADOR_ACTIVADO");
              evento = EVENTO_CONTINUE;
              break;
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_OPEN", "Evento desconocido");
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

  manejar_led();
}

void leer_sensor_distancia() {
  long tiempo_pulso;
  long distancia;
  int pinSensor = 0;

  pinMode(pinSensor, OUTPUT);
  digitalWrite(pinSensor, LOW);

  delayMicroseconds(2);
  digitalWrite(pinSensor, HIGH);

  delayMicroseconds(5);
  digitalWrite(pinSensor, LOW);

  pinMode(pinSensor, INPUT);
  tiempo_pulso = pulseIn(pinSensor, HIGH);

  // Convierto la medicion en centimetros.
  distancia = tiempo_pulso / 29 / 2;

  sensor_distancia.valor_previo = sensor_distancia.valor_actual;
  sensor_distancia.valor_actual = distancia;
}

void leer_sensor_peso() {
  long lectura = analogRead(sensor_flex.pin);
  sensor_flex.valor_previo = sensor_flex.valor_actual;
  sensor_flex.valor_actual = lectura;
}

void manejar_led() {
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
        led.timeout = (diferencia > UMBRAL_LED_FAST_BLINK_TIMEOUT) ? (true) : (false);
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
  evento = EVENTO_PULSADOR_ACTIVADO;
}