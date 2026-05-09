#ifndef LIBCAMERA_H
#define LIBCAMERA_H

/*
 * Cámara OV2640 — ESP32-S3-CAM-N16R8
 *
 * Pines (según documentación Keyestudio MB0184 / ESP32-S3-EYE):
 *   SIOD  → GPIO 4    SIOC  → GPIO 5
 *   VSYNC → GPIO 6    HREF  → GPIO 7
 *   XCLK  → GPIO 15   PCLK  → GPIO 13
 *   D7→16  D6→17  D5→18  D4→12  D3→10  D2→8  D1→9  D0→11
 *   PWDN = -1  RESET = -1
 *
 * El stream MJPEG se sirve en: http://<IP>:81/stream
 * Un solo cliente a la vez (suficiente para el dashboard).
 */

bool initCamera();
void startCameraStream();

#endif /* LIBCAMERA_H */
