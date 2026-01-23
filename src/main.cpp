#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#define MPU 0x68

// ESP32 I2C pins (most boards)
#define SDA_PIN 21 // SDA
#define SCL_PIN 22 // SLC

// Functions
void readAccel();
void readGyro();
void readAccelRaw(float &x, float &y, float &z);
void readGyroRaw(float &x, float &y, float &z);
void computeAccAngles();
void calibrateIMU();

// Variables
float accX, accY, accZ;
float gyroX, gyroY, gyroZ;
float accRoll, accPitch;
float gyroRoll, gyroPitch, gyroYaw;
float roll, pitch, yaw;
float gyroBiasX, gyroBiasY, gyroBiasZ;
float accBiasX, accBiasY, accBiasZ;

unsigned long prevTime;

void setup()
{
	Serial.begin(115200);

	// Explicit ESP32 I2C pins
	Wire.begin(SDA_PIN, SCL_PIN);

	// Wake up MPU6050
	Wire.beginTransmission(MPU);
	Wire.write(0x6B);
	Wire.write(0x00);
	Wire.endTransmission(true);

	Serial.println("MPU6050 Ready");

	calibrateIMU();

	// Initialize gyro angles from accelerometer
	computeAccAngles();
	gyroRoll  = accRoll;
	gyroPitch = accPitch;
	gyroYaw   = 0.0f;

	prevTime = millis();
}

void loop()
{
	unsigned long now = millis();
	float dt = (now - prevTime) * 0.001f;
	prevTime = now;

	readAccel();
	readGyro();
	computeAccAngles();

	gyroRoll  += gyroX * dt;
	gyroPitch += gyroY * dt;
	gyroYaw   += gyroZ * dt;

	// Complementary filter
	pitch = 0.95f * gyroPitch + 0.05f * accPitch;
	roll  = 0.95f * gyroRoll  + 0.05f * accRoll;
	yaw   = gyroYaw;

	// Feedback
	gyroRoll  = roll;
	gyroPitch = pitch;

	// Output: roll / pitch / yaw
	Serial.print(roll);  Serial.print("/");
	Serial.print(pitch); Serial.print("/");
	Serial.println(yaw);

	delay(2);
}

// -------------------- IMU Functions --------------------

void readAccel()
{
	Wire.beginTransmission(MPU);
	Wire.write(0x3B);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 6, true);

	accX = (Wire.read() << 8 | Wire.read()) / 16384.0f - accBiasX;
	accY = (Wire.read() << 8 | Wire.read()) / 16384.0f - accBiasY;
	accZ = (Wire.read() << 8 | Wire.read()) / 16384.0f - accBiasZ;
}

void readGyro()
{
	Wire.beginTransmission(MPU);
	Wire.write(0x43);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 6, true);

	gyroX = (Wire.read() << 8 | Wire.read()) / 131.0f - gyroBiasX;
	gyroY = (Wire.read() << 8 | Wire.read()) / 131.0f - gyroBiasY;
	gyroZ = (Wire.read() << 8 | Wire.read()) / 131.0f - gyroBiasZ;
}

void computeAccAngles()
{
	accRoll  = atan2(accY, sqrt(accX * accX + accZ * accZ)) * 180.0f / PI;
	accPitch = atan2(-accX, sqrt(accY * accY + accZ * accZ)) * 180.0f / PI;
}

void readAccelRaw(float &x, float &y, float &z)
{
	Wire.beginTransmission(MPU);
	Wire.write(0x3B);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 6, true);

	x = (Wire.read() << 8 | Wire.read()) / 16384.0f;
	y = (Wire.read() << 8 | Wire.read()) / 16384.0f;
	z = (Wire.read() << 8 | Wire.read()) / 16384.0f;
}

void readGyroRaw(float &x, float &y, float &z)
{
	Wire.beginTransmission(MPU);
	Wire.write(0x43);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 6, true);

	x = (Wire.read() << 8 | Wire.read()) / 131.0f;
	y = (Wire.read() << 8 | Wire.read()) / 131.0f;
	z = (Wire.read() << 8 | Wire.read()) / 131.0f;
}

void calibrateIMU()
{
	const int samples = 500;
	float ax, ay, az;
	float gx, gy, gz;

	gyroBiasX = gyroBiasY = gyroBiasZ = 0.0f;
	accBiasX  = accBiasY  = accBiasZ  = 0.0f;

	for (int i = 0; i < samples; i++)
	{
		readAccelRaw(ax, ay, az);
		readGyroRaw(gx, gy, gz);

		gyroBiasX += gx;
		gyroBiasY += gy;
		gyroBiasZ += gz;

		accBiasX += ax;
		accBiasY += ay;
		accBiasZ += az;

		delay(2);
	}

	gyroBiasX /= samples;
	gyroBiasY /= samples;
	gyroBiasZ /= samples;

	accBiasX /= samples;
	accBiasY /= samples;
	accBiasZ  = (accBiasZ / samples) - 1.0f;

	Serial.println("IMU Calibrated");
}
