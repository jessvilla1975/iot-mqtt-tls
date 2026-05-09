#ifndef LIBMOTOR_H
#define LIBMOTOR_H

/*
 * Control de 2x SG90 modificados para rotación continua
 * mediante PWM (LEDC) del ESP32-S3.
 *
 * Tracción diferencial:
 *   - Servo LEFT  → GPIO 14   (canal LEDC 4)
 *   - Servo RIGHT → GPIO 21   (canal LEDC 5)
 *
 * Convención de pulso para SG90 continuo:
 *   1500 µs  = detenido
 *   > 1500   = gira en un sentido  (izq: adelante | der: atrás)
 *   < 1500   = gira en otro sentido
 *
 * AJUSTE FINO: si el carrito no se mueve recto, cambia
 * SERVO_DEAD_BAND_US entre 0 y 50 para centrar los servos.
 */

// ── GPIOs ──────────────────────────────────────────────────────────
#define SERVO_LEFT_GPIO     14
#define SERVO_RIGHT_GPIO    21

// ── LEDC (evita conflicto con canal 0 que usa la cámara) ──────────
#define SERVO_LEFT_CHAN      4
#define SERVO_RIGHT_CHAN     5
#define SERVO_FREQ_HZ       50      // 50 Hz → período 20 ms
#define SERVO_RESOLUTION    16      // 16 bits → 65536 niveles

// ── Microsegundos de referencia ────────────────────────────────────
#define SERVO_STOP_US       1500    // Centro = parado
#define SERVO_MAX_US        2000    // Límite superior
#define SERVO_MIN_US        1000    // Límite inferior
#define SERVO_DEAD_BAND_US  0       // Ajuste fino de punto muerto

void setupMotors();
void handleMovement(const char* dir, int speed);
void stopMotors();

#endif /* LIBMOTOR_H */
