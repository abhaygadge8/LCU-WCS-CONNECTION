import configparser
import time
import sys
import paho.mqtt.client as mqtt

# -------------------------------------------------------
# Load config.ini
# -------------------------------------------------------
cfg = configparser.ConfigParser()
cfg.read("config.ini")

MQTT_BROKER_IP   = cfg.get("MQTT", "MQTT_BROKER_IP")
MQTT_BROKER_PORT = cfg.getint("MQTT", "MQTT_BROKER_PORT")
MQTT_CLIENT_ID   = "WCS_TEST_CLIENT"

TOPIC_HEARTBEAT  = cfg.get("MQTT", "MQTT_TOPIC_HEARTBEAT")
TOPIC_TELEMETRY  = cfg.get("MQTT", "MQTT_TOPIC_TELEMETRY")

# -------------------------------------------------------
# MQTT Callbacks
# -------------------------------------------------------
def on_connect(client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print("[WCS] Connected to MQTT broker")
        client.subscribe(TOPIC_HEARTBEAT, qos=1)
        client.subscribe(TOPIC_TELEMETRY, qos=1)
        client.subscribe("lcu/ack", qos=1)
        print("[WCS] Subscribed to:")
        print(f"      {TOPIC_HEARTBEAT}")
        print(f"      {TOPIC_TELEMETRY}")
    else:
        print("[WCS] Connection failed, reason:", reason_code)

def on_disconnect(client, userdata, reason_code, properties):
    print("[WCS] Disconnected from broker, reason:", reason_code)

def on_message(client, userdata, msg):
    print("\n[WCS] Message received")
    print(" Topic   :", msg.topic)
    print(" QoS     :", msg.qos)
    try:
        print(" Payload :", msg.payload.decode())
    except Exception:
        print(" Payload : <binary>")

# -------------------------------------------------------
# MQTT Client Setup
# -------------------------------------------------------
#client = mqtt.Client(client_id=MQTT_CLIENT_ID, clean_session=True)
client = mqtt.Client(
    client_id=MQTT_CLIENT_ID,
    protocol=mqtt.MQTTv311,
    clean_session=True,
    callback_api_version=mqtt.CallbackAPIVersion.VERSION2
)

client.on_connect    = on_connect
client.on_disconnect = on_disconnect
client.on_message    = on_message

print("[WCS] Connecting to broker...")
client.connect(MQTT_BROKER_IP, MQTT_BROKER_PORT, keepalive=30)

# -------------------------------------------------------
# Loop forever
# -------------------------------------------------------
try:
    client.loop_forever()
except KeyboardInterrupt:
    print("\n[WCS] Exiting...")
    client.disconnect()
    sys.exit(0)
