#include "libiot_cam.h"
#include "libmotor.h"
#include <libota.h>
#include <libstorage.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// ══════════════════════════════════════════════════════════════════
//  CREDENCIALES
//  Inyectadas por scripts/add_env_defines.py desde el archivo .env
//  o desde los GitHub Secrets en el pipeline de CI/CD.
//  Nunca escribas valores reales aquí directamente.
// ══════════════════════════════════════════════════════════════════

#ifndef MQTT_SERVER
#  define MQTT_SERVER ""
#endif
#ifndef MQTT_SERVER_IP
#  define MQTT_SERVER_IP ""
#endif
#ifndef MQTT_PORT
#  define MQTT_PORT 8883
#endif
#ifndef MQTT_USER
#  define MQTT_USER "carrito"
#endif
#ifndef MQTT_PASSWORD
#  define MQTT_PASSWORD ""
#endif
#ifndef WIFI_SSID
#  define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
#  define WIFI_PASSWORD ""
#endif

// Certificado raíz Let's Encrypt ISRG Root X1
// El mismo que usa el broker EMQX con TLS de Let's Encrypt.
#ifndef ROOT_CA
#  define ROOT_CA \
"-----BEGIN CERTIFICATE-----\n" \
"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\n" \
"-----END CERTIFICATE-----"
#endif

// ── Definición de variables globales ──────────────────────────────
const char* ssid          = WIFI_SSID;
const char* password      = WIFI_PASSWORD;
const char* mqtt_server   = MQTT_SERVER;
const char* mqtt_server_ip= MQTT_SERVER_IP;
const int   mqtt_port     = MQTT_PORT;
const char* mqtt_user     = MQTT_USER;
const char* mqtt_password = MQTT_PASSWORD;
const char* root_ca       = ROOT_CA;

WiFiClientSecure espClient;
PubSubClient     client(espClient);
time_t           now;

// ── Estado interno ─────────────────────────────────────────────────
static String clientId;
static unsigned long lastHealthcheck = 0;

// ── Callback MQTT ──────────────────────────────────────────────────
static void onMQTTMessage(char* topic, byte* payload, unsigned int length) {
    String topicStr(topic);
    String data;
    data.reserve(length);
    for (unsigned int i = 0; i < length; i++) data += (char)payload[i];

    Serial.println("[MQTT] Recibido — topic: " + topicStr);
    Serial.println("[MQTT] Payload: " + data);

    // ── Comando de movimiento ──────────────────────────────────────
    if (topicStr == TOPIC_CMD_MOV) {
        StaticJsonDocument<128> doc;
        DeserializationError err = deserializeJson(doc, data);
        if (err) { Serial.println("[MQTT] JSON inválido"); return; }
        const char* dir = doc["dir"] | "stop";
        int speed       = doc["speed"] | 60;
        handleMovement(dir, speed);
        return;
    }

    // ── Actualización OTA ─────────────────────────────────────────
    if (topicStr == TOPIC_OTA_CMD) {
        Serial.println("[MQTT] Disparando OTA...");
        checkOTAUpdate(data.c_str());
        return;
    }

    Serial.println("[MQTT] Topic no reconocido, ignorando.");
}

// ── Reconexión MQTT ───────────────────────────────────────────────
static void reconnectCam() {
    int attempts = 0;
    while (!client.connected() && WiFi.status() == WL_CONNECTED && attempts < 5) {
        Serial.print("[MQTT] Conectando a " + String(mqtt_server) + ":" +
                     String(mqtt_port) + " ... ");
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
            Serial.println("OK");

            // Suscripciones
            client.subscribe(TOPIC_CMD_MOV,  1);
            client.subscribe(TOPIC_OTA_CMD,  1);
            Serial.println("[MQTT] Suscrito a " TOPIC_CMD_MOV " y " TOPIC_OTA_CMD);

            // Progreso de OTA visible en Serial
            Update.onProgress([](unsigned int done, unsigned int total) {
                Serial.printf("[OTA] %u/%u bytes (%.0f%%)\r",
                              done, total, (float)done / total * 100.0f);
            });

            publishStatusCam();
        } else {
            int st = client.state();
            Serial.printf("Error %d. Reintento en 5s...\n", st);
            if (st == MQTT_CONNECT_UNAUTHORIZED) {
                Serial.println("[MQTT] Credenciales incorrectas. Deteniéndose.");
                return;
            }
            delay(5000);
        }
        attempts++;
    }
}

// ── Funciones públicas ─────────────────────────────────────────────
String getMacAddress() {
    uint8_t mac[6];
    char buf[20];
    WiFi.macAddress(mac);
    snprintf(buf, sizeof(buf), "CARRITO-%02X%02X%02X", mac[3], mac[4], mac[5]);
    return String(buf);
}

void setupIoTCam() {
    clientId = getMacAddress();
    espClient.setCACert(root_ca);
    espClient.setTimeout(45);

    // Intentar resolver DNS antes de conectar (detecta DNS secuestrado)
    IPAddress brokerIp;
    if (mqtt_server && strlen(mqtt_server) > 0 &&
        !WiFi.hostByName(mqtt_server, brokerIp)) {
        Serial.println("[MQTT] DNS no resuelve " + String(mqtt_server));
    }

    client.setServer(mqtt_server, mqtt_port);
    client.setBufferSize(1024);
    client.setCallback(onMQTTMessage);

    Serial.println("[MQTT] Broker: " + String(mqtt_server) +
                   "  Puerto: " + String(mqtt_port) +
                   "  ClientID: " + clientId);
}

void checkMQTTCam() {
    if (WiFi.status() != WL_CONNECTED) return;
    if (!client.connected()) reconnectCam();
    client.loop();

    // Healthcheck cada 30 s
    if (millis() - lastHealthcheck > 30000) {
        lastHealthcheck = millis();
        Serial.println("[MQTT] Estado: " + String(client.connected() ? "UP" : "DOWN"));
    }
}

time_t setTimeCam() {
    Serial.print("[NTP] Sincronizando");
    configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");
    now = time(nullptr);
    int attempts = 0;
    while (now < 1700000000 && attempts < 60) {
        delay(500); Serial.print(".");
        now = time(nullptr); attempts++;
    }
    Serial.println(now >= 1700000000 ? " OK" : " timeout");
    return now;
}

void publishStatusCam() {
    if (!client.connected()) return;
    StaticJsonDocument<128> doc;
    doc["online"] = true;
    doc["ip"]     = WiFi.localIP().toString();
    doc["fw"]     = getFirmwareVersion();
    char buf[128];
    serializeJson(doc, buf);
    client.publish(TOPIC_STATUS_CONN, buf, true);  // retained=true
    Serial.println("[MQTT] Status publicado: " + String(buf));
}

void publishBatteryCam(float voltage, int levelPct) {
    if (!client.connected()) return;
    StaticJsonDocument<64> doc;
    doc["voltage"] = serialized(String(voltage, 2));
    doc["level"]   = levelPct;
    char buf[64];
    serializeJson(doc, buf);
    client.publish(TOPIC_STATUS_BAT, buf);
}
