#include "libcamera.h"
#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"

// ── Pinout ESP32-S3-CAM-N16R8 (Keyestudio MB0184 / ESP32-S3-EYE) ──
#define CAM_PIN_PWDN    -1
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    15
#define CAM_PIN_SIOD    4
#define CAM_PIN_SIOC    5
#define CAM_PIN_D7      16
#define CAM_PIN_D6      17
#define CAM_PIN_D5      18
#define CAM_PIN_D4      12
#define CAM_PIN_D3      10
#define CAM_PIN_D2      8
#define CAM_PIN_D1      9
#define CAM_PIN_D0      11
#define CAM_PIN_VSYNC   6
#define CAM_PIN_HREF    7
#define CAM_PIN_PCLK    13

#define STREAM_PORT     81
#define STREAM_BOUNDARY "--frame"
#define STREAM_DELAY_MS 40    // ~25 fps máximo

bool initCamera() {
    camera_config_t cfg;
    cfg.ledc_channel = LEDC_CHANNEL_0;   // canal dedicado a XCLK
    cfg.ledc_timer   = LEDC_TIMER_0;
    cfg.pin_d0       = CAM_PIN_D0;
    cfg.pin_d1       = CAM_PIN_D1;
    cfg.pin_d2       = CAM_PIN_D2;
    cfg.pin_d3       = CAM_PIN_D3;
    cfg.pin_d4       = CAM_PIN_D4;
    cfg.pin_d5       = CAM_PIN_D5;
    cfg.pin_d6       = CAM_PIN_D6;
    cfg.pin_d7       = CAM_PIN_D7;
    cfg.pin_xclk     = CAM_PIN_XCLK;
    cfg.pin_pclk     = CAM_PIN_PCLK;
    cfg.pin_vsync    = CAM_PIN_VSYNC;
    cfg.pin_href     = CAM_PIN_HREF;
    cfg.pin_sscb_sda = CAM_PIN_SIOD;
    cfg.pin_sscb_scl = CAM_PIN_SIOC;
    cfg.pin_pwdn     = CAM_PIN_PWDN;
    cfg.pin_reset    = CAM_PIN_RESET;
    cfg.xclk_freq_hz = 20000000;         // 20 MHz
    cfg.pixel_format = PIXFORMAT_JPEG;

    if (psramFound()) {
        cfg.frame_size   = FRAMESIZE_VGA;  // 640×480
        cfg.jpeg_quality = 12;             // 0-63 (menor = mejor calidad)
        cfg.fb_count     = 2;
        cfg.grab_mode    = CAMERA_GRAB_LATEST;
        Serial.println("[Cam] PSRAM detectada — VGA 640×480 Q12");
    } else {
        cfg.frame_size   = FRAMESIZE_QVGA; // 320×240
        cfg.jpeg_quality = 20;
        cfg.fb_count     = 1;
        cfg.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;
        Serial.println("[Cam] Sin PSRAM — QVGA 320×240 Q20");
    }

    esp_err_t err = esp_camera_init(&cfg);
    if (err != ESP_OK) {
        Serial.printf("[Cam] ERROR init: 0x%x\n", err);
        return false;
    }

    // Ajustes del sensor OV2640
    sensor_t* s = esp_camera_sensor_get();
    if (s) {
        s->set_vflip(s, 0);
        s->set_hmirror(s, 0);
        s->set_brightness(s, 1);
        s->set_saturation(s, 0);
    }

    Serial.println("[Cam] OK");
    return true;
}

// ── Tarea de streaming MJPEG (Core 1) ─────────────────────────────
static void streamTask(void* /*param*/) {
    WiFiServer server(STREAM_PORT);
    server.begin();
    Serial.printf("[Cam] Stream en http://%s:%d/stream\n",
                  WiFi.localIP().toString().c_str(), STREAM_PORT);

    for (;;) {
        WiFiClient client = server.available();
        if (!client) { delay(10); continue; }

        Serial.println("[Cam] Cliente conectado al stream");

        // Descartar cabeceras HTTP del cliente
        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r" || line.isEmpty()) break;
        }

        // Cabeceras MJPEG
        client.print(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: multipart/x-mixed-replace;boundary=" STREAM_BOUNDARY "\r\n"
            "Cache-Control: no-cache, no-store\r\n"
            "Pragma: no-cache\r\n"
            "Connection: close\r\n\r\n"
        );

        while (client.connected()) {
            camera_fb_t* fb = esp_camera_fb_get();
            if (!fb) { delay(10); continue; }

            // Cabecera de parte MJPEG
            client.printf(
                STREAM_BOUNDARY "\r\n"
                "Content-Type: image/jpeg\r\n"
                "Content-Length: %u\r\n\r\n",
                (unsigned)fb->len
            );
            client.write(fb->buf, fb->len);
            client.print("\r\n");

            esp_camera_fb_return(fb);
            delay(STREAM_DELAY_MS);
        }

        client.stop();
        Serial.println("[Cam] Cliente desconectado del stream");
    }
}

void startCameraStream() {
    xTaskCreatePinnedToCore(
        streamTask,
        "CamStream",
        8192,       // stack
        nullptr,
        2,          // prioridad (> loop de MQTT en Core 0)
        nullptr,
        1           // Core 1
    );
}
