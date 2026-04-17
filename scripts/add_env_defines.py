# Pre-build: lee .env y define macros C/C++ (MQTT, WiFi, ubicación, puerto).
# ROOT_CA multilínea sigue en secrets.cpp por escapes; no se inyecta aquí.
Import("env")

import os

PROJECT_DIR = env["PROJECT_DIR"]
ENV_PATH = os.path.join(PROJECT_DIR, ".env")

# Macros que antes venían de platformio.ini con ${sysenv.*} (vacías al compilar desde la IDE)
_MANAGED_DEFINES = frozenset(
    {
        "COUNTRY",
        "STATE",
        "CITY",
        "MQTT_SERVER",
        "MQTT_SERVER_IP",
        "MQTT_USER",
        "MQTT_PASSWORD",
        "WIFI_SSID",
        "WIFI_PASSWORD",
        "MQTT_PORT",
    }
)


def strip_managed_build_flags():
    """Quita -D antiguos de estas claves (p. ej. caché o configs viejas)."""
    bf = env.get("BUILD_FLAGS", [])
    if isinstance(bf, str):
        bf = [x for x in bf.split() if x]
    elif not bf:
        bf = []
    new_bf = []
    for flag in bf:
        if not isinstance(flag, str):
            new_bf.append(flag)
            continue
        stripped = flag.strip()
        skip = False
        for name in _MANAGED_DEFINES:
            if stripped.startswith("-D " + name + "=") or stripped.startswith("-D" + name + "="):
                skip = True
                break
        if not skip:
            new_bf.append(flag)
    env["BUILD_FLAGS"] = new_bf

    cpdefs = env.get("CPPDEFINES", [])
    if not cpdefs:
        return
    cleaned = []
    for item in cpdefs:
        if isinstance(item, tuple) and item[0] in _MANAGED_DEFINES:
            continue
        if isinstance(item, str):
            for name in _MANAGED_DEFINES:
                if item.startswith(name + "=") or item == name:
                    break
            else:
                cleaned.append(item)
            continue
        cleaned.append(item)
    env.Replace(CPPDEFINES=cleaned)


def load_dotenv(path):
    """Carga .env en os.environ (el archivo manda sobre valores vacíos previos)."""
    if not os.path.isfile(path):
        return
    with open(path, encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, _, val = line.partition("=")
            key = key.strip()
            val = val.strip()
            if (val.startswith('"') and val.endswith('"')) or (
                val.startswith("'") and val.endswith("'")
            ):
                val = val[1:-1]
            if key:
                os.environ[key] = val


def env_str(key, default):
    v = os.environ.get(key, "").strip()
    return v if v else default


def append_cpp_string_macro(name, value):
    esc = value.replace("\\", "\\\\").replace('"', '\\"')
    env.Append(BUILD_FLAGS=['-D {}=\\"{}\\"'.format(name, esc)])


strip_managed_build_flags()
load_dotenv(ENV_PATH)

country = env_str("COUNTRY", "colombia")
state = env_str("STATE", "valle")
city = env_str("CITY", "tulua")
mqtt_server = env_str("MQTT_SERVER", "")
mqtt_server_ip = env_str("MQTT_SERVER_IP", "")
mqtt_user = env_str("MQTT_USER", "alvaro")
mqtt_password = env_str("MQTT_PASSWORD", "supersecreto")
wifi_ssid = env_str("WIFI_SSID", "MI_RED_WIFI")
wifi_password = env_str("WIFI_PASSWORD", "a1b2c3d4")

port_str = env_str("MQTT_PORT", "8883")
try:
    mqtt_port = int(port_str)
except ValueError:
    mqtt_port = 8883

append_cpp_string_macro("COUNTRY", country)
append_cpp_string_macro("STATE", state)
append_cpp_string_macro("CITY", city)
append_cpp_string_macro("MQTT_SERVER", mqtt_server)
append_cpp_string_macro("MQTT_SERVER_IP", mqtt_server_ip)
append_cpp_string_macro("MQTT_USER", mqtt_user)
append_cpp_string_macro("MQTT_PASSWORD", mqtt_password)
append_cpp_string_macro("WIFI_SSID", wifi_ssid)
append_cpp_string_macro("WIFI_PASSWORD", wifi_password)
env.Append(BUILD_FLAGS=["-D MQTT_PORT={}".format(mqtt_port)])

print(
    "[add_env_defines] MQTT_SERVER={!r} MQTT_PORT={} COUNTRY={!r}".format(
        mqtt_server, mqtt_port, country
    )
)
print("[add_env_defines] ROOT_CA omitido (usa valor por defecto en secrets.cpp)")
