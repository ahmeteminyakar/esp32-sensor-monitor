# Wiring Diagram

## Target Board
- **ESP32 DevKit v1** (or any ESP32 board)

## BME280 Sensor (I2C)
| BME280 Pin | ESP32 Pin | Function |
|------------|-----------|----------|
| SDA        | GPIO 21   | I2C SDA  |
| SCL        | GPIO 22   | I2C SCL  |
| VCC        | 3.3V      | Power    |
| GND        | GND       | Ground   |

> I2C address: 0x76 (SDO to GND) or 0x77 (SDO to VCC).
> Default in firmware: 0x76.

## System Architecture

```
[BME280] --I2C--> [ESP32] --WiFi/MQTT--> [Mosquitto Broker]
                                                |
                                          [Flask Server]
                                           /         \
                                      [SQLite]    [Web Browser]
```

## Network Setup

1. **ESP32** connects to your WiFi network
2. **Mosquitto** MQTT broker runs on the server machine (localhost)
3. **Flask** server subscribes to MQTT, stores in SQLite, serves web dashboard
4. **Browser** connects to Flask on port 5000

## Pin Summary
```
GPIO 21 → BME280 SDA
GPIO 22 → BME280 SCL
3.3V    → BME280 VCC
GND     → BME280 GND
```
