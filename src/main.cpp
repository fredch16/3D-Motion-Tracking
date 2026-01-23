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

float roll_acc = 0.0f;
float pitch_acc = 0.0f;

float roll = 0.0f;
float pitch = 0.0f;

float roll_offset = 0.0f;
float pitch_offset = 0.0f;

float gyroBiasX = 0.0f;
float gyroBiasY = 0.0f;
float gyroBiasZ = 0.0f;




// Functions
void	readAccel();
void	readGyro();
void	computeAccelAngles();
float	wrapAngle(float angle);
void	calibrateGyro();

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

	calibrateGyro();

	delay(200);
	readAccel();
	computeAccelAngles();
	
	roll_offset = roll_acc;
	pitch_offset = pitch_acc;
	roll = 0.0f;
	pitch = 0.0f;
	yaw_g = 0.0f;

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
	gyroX -= gyroBiasX;
	gyroY -= gyroBiasY;
	gyroZ -= gyroBiasZ;
	computeAccelAngles();

	float roll_acc_rel = roll_acc - roll_offset;
	float pitch_acc_rel = pitch_acc - pitch_offset;

	roll_acc_rel = wrapAngle(roll_acc_rel);
	pitch_acc_rel = wrapAngle(pitch_acc_rel);

	// Gyro integration
	roll  = 0.98f * (roll + gyroX * dt) + 0.02f * roll_acc_rel;
	pitch = 0.98f * (pitch - gyroY * dt) + 0.02f * pitch_acc_rel;
	roll = wrapAngle(roll);
	pitch = wrapAngle(pitch);
	yaw_g   += gyroZ * dt;
	yaw_g = wrapAngle(yaw_g);

	// Print at ~40 Hz
	// Print at ~40 Hz for Processing
	static int printDiv = 0;
	if (++printDiv >= 10) {
		printDiv = 0;

		Serial.print(roll, 3);
		Serial.print("/");
		Serial.print(pitch, 3);
		Serial.print("/");
		Serial.println(yaw_g, 3);
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

void computeAccelAngles() {
	roll_acc = atan2(accY, accZ) * 180.0f / PI;
	pitch_acc = atan2(accX, sqrt(accY * accY + accZ * accZ)) * 180.0f / PI;

}

float wrapAngle(float angle) {
	while (angle > 180.0f) angle -= 360.0f;
	while (angle < -180.0f) angle += 360.0f;
	return angle;
}

void calibrateGyro() {
	int samples = 500;
	float sumX = 0, sumY = 0, sumZ = 0;

	Serial.println("Calibrating gyro... keep device still");

	for (int i = 0; i < samples; i++) {
		readGyro();
		sumX += gyroX;
		sumY += gyroY;
		sumZ += gyroZ;
		delay(3);
	}

	gyroBiasX = sumX / samples;
	gyroBiasY = sumY / samples;
	gyroBiasZ = sumZ / samples;

	Serial.println("Gyro calibration done");
}
