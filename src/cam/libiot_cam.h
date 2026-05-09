#ifndef LIBIOT_CAM_H
#define LIBIOT_CAM_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <time.h>

// ── Topics MQTT del carrito ────────────────────────────────────────
#define TOPIC_CMD_MOV       "carrito/cmd/movimiento"   // suscribe
#define TOPIC_STATUS_CONN   "carrito/status/conexion"  // publica
#define TOPIC_STATUS_BAT    "carrito/status/bateria"   // publica
#define TOPIC_OTA_CMD       "carrito/ota/update"       // suscribe
#define TOPIC_OTA_STATUS    "carrito/ota/status"       // publica

// ── Variables globales (definidas en libiot_cam.cpp) ───────────────
// Requeridas por libwifi.cpp (extern const char* ssid / password)
extern const char* ssid;
extern const char* password;

extern const char* mqtt_server;
extern const char* mqtt_server_ip;
extern const int   mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const char* root_ca;

extern WiFiClientSecure espClient;
extern PubSubClient     client;
extern time_t           now;

// ── Funciones ──────────────────────────────────────────────────────
void   setupIoTCam();
void   checkMQTTCam();
time_t setTimeCam();
void   publishStatusCam();
void   publishBatteryCam(float voltage, int levelPct);
String getMacAddress();

#endif /* LIBIOT_CAM_H */
