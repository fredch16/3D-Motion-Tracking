# MPU6050 Digital Twin

A real-time 3D orientation tracker built using an MPU6050 IMU and an ESP32.

This project demonstrates how accelerometers and gyroscopes can be combined using sensor fusion to estimate orientation in real time and visualize it as a digital twin.

---

## Build Tutorial

Full video walkthrough and explanation:  

[https://youtu.be/ZMkNSZ6DQhE?si=or2y50Fbl3jJmBzm]

---

## Features

- Real-time orientation tracking
- MPU6050 accelerometer + gyroscope fusion
- Complementary filter implementation
- 3D visualization using Processing
- Beginner-friendly hardware setup

---

## Hardware

- ESP32 (or compatible microcontroller)
- MPU6050 IMU
- Breadboard
- Jumper wires
- USB cable

---

## Software

### Arduino IDE
Used to upload firmware to the ESP32.

Download:  
[https://www.arduino.cc/en/software/]

### Processing
Used for the real-time 3D visualization.

Download:  
[(https://processing.org/download)]

---

## Wiring

| MPU6050 | ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

> Note: SDA/SCL pins may vary depending on your microcontroller.

## How It Works

The MPU6050 contains:
- An accelerometer
- A gyroscope

The accelerometer provides a long-term gravity reference, while the gyroscope tracks rotational motion.

A complementary filter combines both sensor readings to produce a more stable orientation estimate than either sensor alone.

---

## Future Improvements

- Magnetometer integration
- Kalman filtering
- Wireless telemetry
- Quaternion-based orientation tracking
- 3D printed enclosure

---

## License

MIT License
