# ESP32 Wireless Sensor Monitoring System

ESP32-based IoT sensor node that collects environmental data via I2C, publishes over MQTT, and visualizes on a web dashboard with SQLite history.

## Features

- **ESP32 firmware**: BME280 sensor reading (temp, humidity, pressure) over I2C
- **MQTT publishing**: JSON payloads every 10 seconds
- **Web dashboard**: Real-time cards + Chart.js line charts
- **Data persistence**: SQLite database for historical data
- **REST API**: `/api/latest` and `/api/history` endpoints

## Architecture

```
[BME280] --I2C--> [ESP32] --WiFi/MQTT--> [Mosquitto]
                                              |
                                         [Flask Server]
                                          /         \
                                     [SQLite]    [Dashboard]
```

## Hardware

| Component | Detail                     |
|-----------|----------------------------|
| MCU       | ESP32 DevKit v1            |
| Sensor    | BME280 (I2C, 0x76)         |
| Wiring    | SDA→GPIO21, SCL→GPIO22     |

See [docs/wiring.md](docs/wiring.md) for full details.

## Project Structure

```
esp32-sensor-monitor/
├── firmware/
│   ├── platformio.ini       # PlatformIO config + dependencies
│   └── src/
│       └── main.cpp         # ESP32 firmware (WiFi, MQTT, BME280)
├── server/
│   ├── app.py               # Flask + MQTT subscriber + SQLite
│   ├── requirements.txt     # Python dependencies
│   └── templates/
│       └── index.html       # Dashboard (Chart.js)
├── docs/
│   └── wiring.md            # Wiring & architecture
└── README.md
```

## Setup & Run

### 1. MQTT Broker

Install and start Mosquitto:

```bash
# Linux/macOS
sudo apt install mosquitto    # or: brew install mosquitto
sudo systemctl start mosquitto

# Windows
# Download from https://mosquitto.org/download/
# Run mosquitto.exe
```

### 2. ESP32 Firmware

Edit WiFi and MQTT settings in `firmware/src/main.cpp`:

```cpp
#define WIFI_SSID   "YOUR_WIFI_SSID"
#define WIFI_PASS   "YOUR_WIFI_PASS"
#define MQTT_BROKER "192.168.1.100"   // your server IP
```

Build and flash with PlatformIO:

```bash
cd firmware
pio run --target upload
pio device monitor    # view serial output
```

### 3. Web Server

```bash
cd server
pip install -r requirements.txt
python app.py
```

Open **http://localhost:5000** in your browser.

## MQTT Message Format

Topic: `sensors/esp32/bme280`

```json
{
    "temperature": 24.3,
    "humidity": 55.2,
    "pressure": 1013.4,
    "uptime_s": 120
}
```

## API Endpoints

| Endpoint               | Description                     |
|------------------------|---------------------------------|
| `GET /`                | Web dashboard                   |
| `GET /api/latest`      | Latest sensor reading (JSON)    |
| `GET /api/history?limit=100` | Last N readings from SQLite |

## Dashboard

Dark-themed dashboard showing:
- Live value cards (temperature, humidity, pressure)
- Historical line charts with auto-scroll
- Auto-refresh every 5 seconds
