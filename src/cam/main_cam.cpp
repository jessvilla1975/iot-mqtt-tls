#include <WiFi.h>
#include <libwifi.h>
#include <libstorage.h>
#include <libprovision.h>
#include "libiot_cam.h"
#include "libcamera.h"
#include "libmotor.h"

// ─────────────────────────────────────────────────────────────────
//  CARRITO IoT — ESP32-S3-CAM-N16R8
//
//  Hardware conectado:
//    · Cámara OV2640 (integrada, pines fijos en libcamera.h)
//    · Servo izquierdo SG90 → GPIO 14
//    · Servo derecho  SG90  → GPIO 21
//    · Botón BOOT (GPIO 0)  → manter 3 s al encender = factory reset
//
//  AJUSTE FINO DE SERVOS:
//    Si el carrito gira solo estando "parado", edita SERVO_DEAD_BAND_US
//    en libmotor.h en pasos de 5 µs hasta que los servos queden quietos.
// ─────────────────────────────────────────────────────────────────

#ifndef FIRMWARE_VERSION
#  define FIRMWARE_VERSION "v1.0.0"
#endif

// ── ADC para batería (opcional) ───────────────────────────────────
// Conecta un divisor resistivo 100k/100k entre la celda LiPo y GPIO 3.
// Con ese divisor: V_adc = V_bat / 2  →  V_bat = analogRead(3) * 3.3 / 4095 * 2
#define BAT_ADC_GPIO    3
#define BAT_ENABLED     false   // Cambia a true si tienes el divisor conectado
#define BAT_INTERVAL_MS 30000   // Publicar batería cada 30 s

static unsigned long lastBatMs = 0;

static void readAndPublishBattery() {
#if BAT_ENABLED
    int raw       = analogRead(BAT_ADC_GPIO);
    float voltage = (float)raw * 3.3f / 4095.0f * 2.0f;   // ajustar si divisor ≠ 1:1
    int   level   = (int)constrain((voltage - 3.2f) / (4.2f - 3.2f) * 100.0f, 0, 100);
    publishBatteryCam(voltage, level);
    Serial.printf("[BAT] %.2f V  %d%%\n", voltage, level);
#endif
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("  CarrIoT — ESP32-S3-CAM-N16R8");
    Serial.print("  Firmware: ");
    Serial.println(getFirmwareVersion());
    Serial.println("========================================\n");

    // ── Factory reset al mantener BOOT 3 s ───────────────────────
    pinMode(0, INPUT_PULLUP);
    if (digitalRead(0) == LOW) {
        unsigned long t0 = millis();
        while (digitalRead(0) == LOW && (millis() - t0) < 3000) delay(10);
        if ((millis() - t0) >= 3000) {
            Serial.println("[BOOT] Factory reset...");
            factoryReset();  // borra NVS y reinicia
        }
    }

    // ── Motores: detener primero (seguridad) ─────────────────────
    setupMotors();

    // ── WiFi ──────────────────────────────────────────────────────
    listWiFiNetworks();
    delay(500);
    startWiFi("carrito-cam");

    if (WiFi.status() == WL_CONNECTED) {
        setTimeCam();
        setupIoTCam();

        // ── Cámara ───────────────────────────────────────────────
        if (!initCamera()) {
            Serial.println("[ERROR] Cámara no inicializada.");
            Serial.println("        Comprueba el conector FPC y reinicia.");
            Serial.println("        Reiniciando en 10 s...");
            delay(10000);
            ESP.restart();
        }

        // Stream MJPEG en Core 1 (no bloquea el loop de MQTT)
        startCameraStream();

        Serial.println("\n========================================");
        Serial.print("  IP del carrito: ");
        Serial.println(WiFi.localIP());
        Serial.println("  Stream:  http://" + WiFi.localIP().toString() + ":81/stream");
        Serial.println("  Control: carrito/cmd/movimiento");
        Serial.println("           {\"dir\":\"forward\",\"speed\":70}");
        Serial.println("  OTA:     carrito/ota/update");
        Serial.println("           {\"url\":\"https://...\",\"version\":\"v1.1.0\"}");
        Serial.println("========================================\n");

    } else {
        Serial.println("[WiFi] Sin conexión. Portal de configuración activo.");
        Serial.println("       Conecta al AP 'ESP32-Setup-XXXXXX' y abre http://192.168.4.1");
        startProvisioningAP();
    }
}

void loop() {
    // ── Modo de aprovisionamiento WiFi ────────────────────────────
    if (isProvisioning()) {
        provisioningLoop();
        return;
    }

    // ── Mantener WiFi y MQTT ──────────────────────────────────────
    checkWiFi();
    checkMQTTCam();

    // ── Telemetría de batería (si está habilitada) ─────────────────
    if (millis() - lastBatMs > BAT_INTERVAL_MS) {
        lastBatMs = millis();
        readAndPublishBattery();
    }
}
