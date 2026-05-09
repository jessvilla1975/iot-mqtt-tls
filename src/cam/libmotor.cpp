#include "libmotor.h"
#include <Arduino.h>
#include <string.h>

/*
 * Convierte microsegundos a duty cycle de 16 bits para 50 Hz.
 *   Período = 20 000 µs → 65 536 counts
 *   duty = us × 65 536 / 20 000
 */
static inline uint32_t usToDuty(uint16_t us) {
    return (uint32_t)((uint64_t)us * 65536ULL / 20000ULL);
}

/* Escribe directamente el pulso en µs a un canal LEDC */
static void servoWrite(uint8_t channel, uint16_t us) {
    us = (uint16_t)constrain(us, SERVO_MIN_US, SERVO_MAX_US);
    ledcWrite(channel, usToDuty(us));
}

void setupMotors() {
    ledcSetup(SERVO_LEFT_CHAN,  SERVO_FREQ_HZ, SERVO_RESOLUTION);
    ledcSetup(SERVO_RIGHT_CHAN, SERVO_FREQ_HZ, SERVO_RESOLUTION);
    ledcAttachPin(SERVO_LEFT_GPIO,  SERVO_LEFT_CHAN);
    ledcAttachPin(SERVO_RIGHT_GPIO, SERVO_RIGHT_CHAN);
    stopMotors();
    Serial.println("[Motor] OK — GPIO " + String(SERVO_LEFT_GPIO) +
                   " (izq) | GPIO " + String(SERVO_RIGHT_GPIO) + " (der)");
}

/*
 * Maneja el comando de movimiento recibido por MQTT.
 *
 * payload esperado: {"dir":"forward","speed":70}
 * dir: "forward" | "backward" | "left" | "right" | "stop"
 * speed: 0-100  (0 = mínimo movimiento, 100 = máximo)
 *
 * Tracción diferencial — servo derecho montado espejado:
 *   FORWARD  → izq >1500 (CW=adelante)  der <1500 (CCW=adelante)
 *   BACKWARD → izq <1500                der >1500
 *   LEFT     → izq <1500  der <1500     (giro sobre el eje)
 *   RIGHT    → izq >1500  der >1500     (giro sobre el eje)
 */
void handleMovement(const char* dir, int speed) {
    speed = constrain(speed, 0, 100);
    int delta = speed * 5;          // 0-500 µs desde el centro

    uint16_t leftUs  = (uint16_t)(SERVO_STOP_US + SERVO_DEAD_BAND_US);
    uint16_t rightUs = (uint16_t)(SERVO_STOP_US - SERVO_DEAD_BAND_US);

    if (strcmp(dir, "forward") == 0) {
        leftUs  = (uint16_t)(SERVO_STOP_US + delta);
        rightUs = (uint16_t)(SERVO_STOP_US - delta);
    } else if (strcmp(dir, "backward") == 0) {
        leftUs  = (uint16_t)(SERVO_STOP_US - delta);
        rightUs = (uint16_t)(SERVO_STOP_US + delta);
    } else if (strcmp(dir, "left") == 0) {
        leftUs  = (uint16_t)(SERVO_STOP_US - delta);
        rightUs = (uint16_t)(SERVO_STOP_US - delta);
    } else if (strcmp(dir, "right") == 0) {
        leftUs  = (uint16_t)(SERVO_STOP_US + delta);
        rightUs = (uint16_t)(SERVO_STOP_US + delta);
    } else {
        stopMotors();
        return;
    }

    servoWrite(SERVO_LEFT_CHAN,  leftUs);
    servoWrite(SERVO_RIGHT_CHAN, rightUs);

    Serial.printf("[Motor] dir=%s spd=%d  izq=%dµs der=%dµs\n",
                  dir, speed, leftUs, rightUs);
}

void stopMotors() {
    ledcWrite(SERVO_LEFT_CHAN,  usToDuty(SERVO_STOP_US + SERVO_DEAD_BAND_US));
    ledcWrite(SERVO_RIGHT_CHAN, usToDuty(SERVO_STOP_US - SERVO_DEAD_BAND_US));
}
