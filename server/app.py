import json
import sqlite3
import threading
from datetime import datetime

from flask import Flask, render_template, jsonify, request
import paho.mqtt.client as mqtt

MQTT_BROKER = "localhost"
MQTT_PORT   = 1883
MQTT_TOPIC  = "sensors/esp32/bme280"
DB_PATH     = "sensor_data.db"

app = Flask(__name__)


def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            temperature REAL,
            humidity REAL,
            pressure REAL,
            uptime_s INTEGER
        )
    """)
    conn.commit()
    conn.close()


def insert_reading(data):
    conn = sqlite3.connect(DB_PATH)
    conn.execute(
        "INSERT INTO readings (timestamp, temperature, humidity, pressure, uptime_s) "
        "VALUES (?, ?, ?, ?, ?)",
        (
            datetime.now().isoformat(),
            data.get("temperature"),
            data.get("humidity"),
            data.get("pressure"),
            data.get("uptime_s"),
        )
    )
    conn.commit()
    conn.close()


def get_readings(limit=100):
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    rows = conn.execute(
        "SELECT * FROM readings ORDER BY id DESC LIMIT ?", (limit,)
    ).fetchall()
    conn.close()
    return [dict(r) for r in reversed(rows)]


latest_data = {}


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(f"MQTT: connected to {MQTT_BROKER}:{MQTT_PORT}")
        client.subscribe(MQTT_TOPIC)
    else:
        print(f"MQTT: connection failed, rc={rc}")


def on_message(client, userdata, msg):
    global latest_data
    try:
        data = json.loads(msg.payload.decode())
        latest_data = data
        insert_reading(data)
        print(f"T={data.get('temperature')}  H={data.get('humidity')}  P={data.get('pressure')}")
    except (json.JSONDecodeError, KeyError) as e:
        print(f"MQTT parse error: {e}")


def start_mqtt():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        client.loop_forever()
    except Exception as e:
        print(f"MQTT error: {e}")


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/latest")
def api_latest():
    return jsonify(latest_data)


@app.route("/api/history")
def api_history():
    limit = request.args.get("limit", 100, type=int)
    limit = min(limit, 1000)
    return jsonify(get_readings(limit))


if __name__ == "__main__":
    init_db()

    mqtt_thread = threading.Thread(target=start_mqtt, daemon=True)
    mqtt_thread.start()

    print("Server: http://localhost:5000")
    app.run(host="0.0.0.0", port=5000, debug=False)
