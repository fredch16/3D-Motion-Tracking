#include <Arduino.h>
#include <Wire.h>

#define MPU_ADDR 0x68

#define SDA_PIN 21
#define SLC_PIN 22

//400 Hz update period
constexpr uint32_t IMU_PERIOD_US = 2500;

uint32_t lastUpdate = 0;

// Raw sensor data
float accX, accY, accZ;
float gyroX, gyroY, gyroZ;

float roll_g = 0.0f;
float pitch_g = 0.0f;
float yaw_g = 0.0f;


// Functions
void	readAccel();
void	readGyro();

void setup() {
	Serial.begin(115200);
	delay(1000);

	Wire.setPins(SDA_PIN, SLC_PIN);
	Wire.begin();
	Wire.setClock(400000); //400 kHz I2C

	// Wake MPU6050

	Wire.beginTransmission(MPU_ADDR);
	Wire.write(0x6B);
	Wire.write(0x00);
	Wire.endTransmission(true);

	// Set accelerometer to ±2g
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(0x1C);        // ACCEL_CONFIG
	Wire.write(0x00);        // ±2g
	Wire.endTransmission(true);

	// Set gyro to ±250 dps
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(0x1B);        // GYRO_CONFIG
	Wire.write(0x00);        // ±250 dps
	Wire.endTransmission(true);

	Serial.println("EPS32 + MPU6050 @ 400Hz");
	lastUpdate = micros();
}

void loop() {
	uint32_t now = micros();
	if (now - lastUpdate < IMU_PERIOD_US)
		return;

	float dt = (now - lastUpdate) * 1e-6f;
	lastUpdate = now;

	readAccel();
	readGyro();

	// Gyro integration
	roll_g  += gyroX * dt;
	pitch_g -= gyroY * dt; // Based on orientation relative to me IRL I changed this to -
	yaw_g   += gyroZ * dt;

	// Print at ~40 Hz
	static int printDiv = 0;
	if (++printDiv >= 10) {
		printDiv = 0;

		Serial.print("GYRO ANGLES | ");
		Serial.print("R: "); Serial.print(roll_g, 2);
		Serial.print(" P: "); Serial.print(pitch_g, 2);
		Serial.print(" Y: "); Serial.println(yaw_g, 2);
	}
}


// ------------------ IMU IO ------------------

void readAccel() {
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(0x3B);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU_ADDR, 6, true);

	int16_t rawX = (int16_t)(Wire.read() << 8 | Wire.read());
	int16_t rawY = (int16_t)(Wire.read() << 8 | Wire.read());
	int16_t rawZ = (int16_t)(Wire.read() << 8 | Wire.read());

	accX = rawX / 16384.0f;
	accY = rawY / 16384.0f;
	accZ = rawZ / 16384.0f;
}


void readGyro() {
	Wire.beginTransmission(MPU_ADDR);
	Wire.write(0x43);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU_ADDR, 6, true);

	int16_t rawX = (int16_t)(Wire.read() << 8 | Wire.read());
	int16_t rawY = (int16_t)(Wire.read() << 8 | Wire.read());
	int16_t rawZ = (int16_t)(Wire.read() << 8 | Wire.read());

	gyroX = rawX / 131.0f;
	gyroY = rawY / 131.0f;
	gyroZ = rawZ / 131.0f;
}