# Script pre-build: carga .env y define MQTT_PORT (evita compilar sin variable de entorno).
# ROOT_CA sigue omitido por escapes; usa el valor por defecto en secrets.cpp o edítalo ahí.
Import("env")

import os

PROJECT_DIR = env["PROJECT_DIR"]
ENV_PATH = os.path.join(PROJECT_DIR, ".env")


def load_dotenv(path):
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
                os.environ.setdefault(key, val)


load_dotenv(ENV_PATH)

port_str = os.environ.get("MQTT_PORT", "").strip() or "8883"
try:
    mqtt_port = int(port_str)
except ValueError:
    mqtt_port = 8883

# Quitar definiciones rotas de MQTT_PORT que vengan de build_flags vacíos
cpdefs = env.get("CPPDEFINES", [])
if cpdefs:
    cleaned = []
    for item in cpdefs:
        if isinstance(item, tuple) and item[0] == "MQTT_PORT":
            continue
        if isinstance(item, str) and item.startswith("MQTT_PORT"):
            continue
        cleaned.append(item)
    env.Replace(CPPDEFINES=cleaned)

env.Append(CPPDEFINES=[("MQTT_PORT", mqtt_port)])
print(f"[add_env_defines] MQTT_PORT={mqtt_port}")
print("[add_env_defines] ROOT_CA omitido (usa valor por defecto en secrets.cpp)")
