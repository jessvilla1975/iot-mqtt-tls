#include <WiFi.h>
#include <libwifi.h>
#include <libdisplay.h>
#include <libstorage.h>
#include <Arduino.h>
#include "esp_netif.h"

static void applyPublicDnsOverride();

static bool waitForWiFiConnection(int maxAttempts = 20) {
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  return WiFi.status() == WL_CONNECTED;
}

static bool connectWiFiWithCredentials(const char *targetSsid, const char *targetPassword, const char *label) {
  if (targetSsid == nullptr || strlen(targetSsid) == 0) {
    return false;
  }
  Serial.println(label);
  WiFi.disconnect(false, true);
  delay(200);
  WiFi.begin(targetSsid, targetPassword);
  bool connected = waitForWiFiConnection();
  if (connected) {
    Serial.println("\nWiFi connected");
    Serial.print("SSID conectado: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    WiFi.setSleep(false);
    applyPublicDnsOverride();
    return true;
  }
  Serial.println("\nWiFi connection failed");
  return false;
}

/**
 * El DHCP del campus a veces entrega DNS que “secuestran” dominios (p. ej. FreeDNS → 208.91.112.55).
 * Forzar DNS públicos en la STA evita conectar MQTT a una IP equivocada (ECONNABORTED / errno 113).
 */
static void applyPublicDnsOverride() {
  esp_netif_t *sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (sta == nullptr) {
    Serial.println("DNS público: interfaz STA no disponible");
    return;
  }
  esp_netif_dns_info_t mainDns{};
  mainDns.ip.type = ESP_IPADDR_TYPE_V4;
  mainDns.ip.u_addr.ip4.addr = esp_ip4addr_aton("8.8.8.8");
  esp_netif_dns_info_t backupDns{};
  backupDns.ip.type = ESP_IPADDR_TYPE_V4;
  backupDns.ip.u_addr.ip4.addr = esp_ip4addr_aton("1.1.1.1");
  esp_err_t e1 = esp_netif_set_dns_info(sta, ESP_NETIF_DNS_MAIN, &mainDns);
  esp_err_t e2 = esp_netif_set_dns_info(sta, ESP_NETIF_DNS_BACKUP, &backupDns);
  if (e1 == ESP_OK && e2 == ESP_OK) {
    Serial.println("DNS: 8.8.8.8 (main) y 1.1.1.1 (backup) — anula DNS del AP");
  } else {
    Serial.printf("DNS público: error main=%s backup=%s\n", esp_err_to_name(e1), esp_err_to_name(e2));
  }
}


/**
 * Verifica si el dispositivo está conectado al WiFi.
 * Si no está conectado intenta reconectar a la red.
 * Si está conectado, intenta conectarse a MQTT si aún 
 * no se tiene conexión.
 */
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.reconnect();
    
    bool reconnected = waitForWiFiConnection();
    
    if (reconnected) {
      Serial.println("\nWiFi reconnected");
      Serial.print("SSID: ");
      Serial.println(WiFi.SSID());
      WiFi.setSleep(false);
      applyPublicDnsOverride();
    } else {
      Serial.println("\nWiFi reconnection failed");
      // Si la reconexión falla, intentar secuencia completa con fallback de credenciales.
      startWiFi("");
    }
  }
}

/**
 * Imprime en consola la cantidad de redes WiFi disponibles y
 * sus nombres.
 */
void listWiFiNetworks() {
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.println("dBm)");
    }
  }
  
  // Delete the scan result to free memory
  WiFi.scanDelete();
}

/**
 * Adquiere la dirección MAC del dispositivo y la retorna en formato de cadena.
 */
String getHostname() {
  uint8_t mac[6];
  char macStr[18];
  WiFi.macAddress(mac);
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", 
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

/**
 * Inicia el servicio de WiFi e intenta conectarse a la red WiFi específicada en las constantes.
 */
void startWiFi(const char* hostname) {
  if (hostname && strlen(hostname) > 0) {
    WiFi.setHostname(hostname);
  }
  
  String s, p;
  bool connected = false;
  bool hasNvsCredentials = loadWiFiCredentials(s, p);
  if (hasNvsCredentials) {
    connected = connectWiFiWithCredentials(s.c_str(), p.c_str(), "Using stored WiFi credentials from NVS");
    if (!connected) {
      Serial.println("Fallo con credenciales NVS, intentando credenciales de secrets.cpp...");
    }
  }

  if (!connected) {
    connected = connectWiFiWithCredentials(ssid, password, "Using built-in WiFi credentials (secrets.cpp)");
    if (connected) {
      // Persistir fallback exitoso para próximos reinicios.
      saveWiFiCredentials(String(ssid), String(password));
    }
  }

  if (!connected) {
    Serial.println("\nWiFi connection failed");
  }
}

bool hasStoredWiFi() {
  return hasWiFiCredentials();
}

bool saveWiFi(const String &s, const String &pwd) {
  return saveWiFiCredentials(s, pwd);
}

bool clearStoredWiFi() {
  return clearWiFiCredentials();
}

void factoryReset() {
  Serial.println("Factory reset: clearing WiFi credentials and restarting...");
  clearWiFiCredentials();
  WiFi.disconnect(true, true);
  delay(500);
  ESP.restart();
}
