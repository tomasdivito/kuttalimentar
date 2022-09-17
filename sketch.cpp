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
#define ESTADO_EMBED_OPEN_SERVING
#define ESTADO_EMBED_CLOSED_MEASURING 102
#define ESTADO_EMBED_SERVING 103
#define ESTADO_EMBED_INSUFICIENTE 104

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
#define EVENTO_PESO_PORCION_FALTA 2000
#define EVENTO_PESO_PORCION_COMPLETA 3000
#define EVENTO_PESO_PORCION_INSUFICIENTE 4000
#define EVENTO_PRESENCIA_DETECTADA 5000
#define EVENTO_OPEN_SERVING_TIMEOUT 6000

/// PINES
#define PIN_PULSADOR 2
#define PIN_LED 3
#define PIN_SERVO_1 7
#define PIN_SERVO_2 6
#define PIN_FLEX A0
#define PIN_DISTANCIA A1

#define SERVO_OPEN_POSIION 90
#define SERVO_CLOSED_POSITION 180

/// UMBRALES
#define UMBRAL_TIMEOUT 1000 // Para correr una vez por segundo
#define UMBRAL_LED_FAST_BLINK_TIMEOUT 1000 // Estos numeros son grandes pero probablemente tengan que ser mucho mas chicos en el arduino normal
#define UMBRAL_LED_SLOW_BLINK_TIMEOUT 2500
#define UMBRAL_PESO_PORCION 1001
#define UMBRAL_PROCESO_PORCION 3000
#define UMBRAL_PROCESO_SERVING 4000

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
typedef struct stPulsador {
  int estado;
} stPulsador;
typedef int stEvento;
typedef int stEstado;

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
Servo servo_porcion;  // Objeto servo para manejar puerta de porcion.
Servo servo_puerta;

void do_init() {
  Serial.begin(9600);

  timeout = false;
  ultima_lectura_millis = millis();

  pinMode(PIN_PULSADOR, INPUT);  
  attachInterrupt(digitalPinToInterrupt(PIN_PULSADOR), interrupt_pulsador, CHANGE);

  servo_porcion.attach(PIN_SERVO_1);
  servo_puerta.attach(PIN_SERVO_2);
  
  sensor_distancia.pin = PIN_DISTANCIA;
  sensor_distancia.estado = ESTADO_DISTANCIA_AUSENTE;

  sensor_flex.pin = PIN_FLEX;

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

void detectar_eventos() {
  long lectura_millis = millis();
  int diferencia;
  bool timeout_proceso = false;

  switch (estado) {
    case ESTADO_EMBED_IDLE:
      {
        if (sensor_flex.valor_actual < UMBRAL_PESO_PORCION {
          ultima_lectura_millis_proceso_porcion = lectura_millis;
          evento = EVENTO_PESO_PORCION_FALTA;
          break;
        }

        if (sensor_distancia.valor_actual < UMBRAL_PRESENCIA_MAXIMA) {
          diferencia = (lectura_millis - ultima_lectura_millis_proceso);
          timeout_proceso = (diferencia < UMBRAL_PROCESO_SERVING) ? (true) : (false);
          if (timeout_proceso) {
            evento = EVENTO_PRESENCIA_DETECTADA;
            break;
          }
        }

        if (pulsador.estado == ESTADO_BOTON_PRESIONADO) {
          evento = EVENTO_PRESENCIA_DETECTADA;
          break;
        }

        evento = EVENTO_CONTINUE;
        break;
      }
    case ESTADO_EMBED_OPEN_SERVING:
      {
        diferencia = (lectura_millis - ultima_lectura_millis_proceso_porcion);
        timeout_proceso = (diferencia < UMBRAL_PROCESO_PORCION) ? (true) : (false);
        if (timeout_proceso) {
          evento = EVENTO_OPEN_SERVING_TIMEOUT;
          break;
        }

        evento = EVENTO_CONTINUE;
        break;
      }
    case ESTADO_EMBED_CLOSED_MEASURING:
      {
        // TODO: ESTO NUNCA VA A PASAR!!! pero tenemos que ver si NO HAY CAMBIOS
        if (sensor_flex.valor_previo == sensor_flex.valor_actual) {
          evento = EVENTO_PESO_PORCION_INSUFICIENTE;
          break;          
        }
        if (sensor_flex.valor_actual < UMBRAL_PESO_PORCION {
          ultima_lectura_millis_proceso_porcion = lectura_millis;
          evento = EVENTO_PESO_PORCION_FALTA;
          break;
        }
        break;
      }
    case ESTADO_EMBED_SERVING:
      {
        break;
      }
    default:
      DebugPrint("No sensors to read");
  }

  // Si el pulsador fue presionado y no se proceso 
  // lo ponemos como que fue suelto
  pulsador.estado = ESTADO_BOTON_SUELTO;
  
  //FLEX_VALUE = analogRead(FLEX_PIN);
  //Serial.println(FLEX_VALUE);
  //SERVO_POSITION = map(FLEX_VALUE, 770, 950, 0, 180);
  //SERVO_POSITION = constrain(SERVO_POSITION, 0, 180);
  //servo.write(SERVO_POSITION);

  //DEPENDIENDO DEL ESTADO DEL EMBEBIDO PODRIAMOS LEER UNO U OTRO SENSOR
  //if(leer_sensor_distancia();
  //if(evento_sensor_peso() || evento_sensor_distancia());
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
    detectar_eventos();
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
          case EVENTO_PESO_PORCION_FALTA:
            {
              DebugPrintEstado("ESTADO_EMBED_IDLE", "EVENTO_PESO_PORCION_FALTA");
              led.estado = ESTADO_LED_FAST_BLINK_PRENDIDO;
              estado = ESTADO_EMBED_OPEN_SERVING;
              break;
            }
          case EVENTO_PRESENCIA_DETECTADA:
            {
              DebugPrintEstado("ESTADO_EMBED_IDLE", "EVENTO_PRESENCIA_DETECTADA");
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
              DebugPrintEstado("ESTADO_EMBED_OPEN_SERVING", "EVENTO_CONTINUE");
              servo_porcion.write(SERVO_OPEN_POSITION);
              break;   
            }
          case EVENTO_OPEN_SERVING_TIMEOUT:
            {
              DebugPrintEstado("ESTADO_EMBED_OPEN_SERVING", "EVENTO_CONTINUE");
              servo_porcion.write(SERVO_CLOSED_POSITION);
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
              break;   
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_SERVING", "Evento desconocido");
              break;              
            } 
        }
      }
    case ESTADO_EMBED_INSUFICIENTE:
      {
        switch (evento) {
          case EVENTO_CONTINUE:
            {
              DebugPrintEstado("ESTADO_EMBED_INSUFICIENTE", "EVENTO_CONTINUE");
              led.estado = ESTADO_LED_SLOW_BLINK_PRENDIDO;
              break;   
            }
          default:
            {
              DebugPrintEstado("ESTADO_EMBED_INSUFICIENTE", "Evento desconocido");
              break;              
            } 
        }
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
  pulsador.estado = ESTADO_BOTON_PRESIONADO;
}