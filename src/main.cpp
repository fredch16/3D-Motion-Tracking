#include <Arduino.h>
#include <Wire.h>

#define MPU 0x68

// ARDUINO VERSION!!!!

// Functions
void	readAccel();
void	readGyro();
void	readAccelRaw(float &x, float &y, float &z);
void	readGyroRaw(float &x, float &y, float &z);
void	computeAccAngles();
void	calibrateIMU();

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
	Wire.begin();

	//Wake up MPU6050
	Wire.beginTransmission(MPU);
	Wire.write(0x6B);
	Wire.write(0x00);
	Wire.endTransmission(true);
	Serial.println("MPU6050 Ready");
	calibrateIMU();
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
	
	gyroRoll += gyroX * dt;
	gyroPitch += gyroY * dt;
	gyroYaw += gyroZ*dt;

	pitch = 0.95 * gyroPitch + 0.05 * accPitch;
	roll = 0.95 * gyroRoll + 0.05 * accRoll;
	yaw = gyroYaw;

	//Feedback
	gyroRoll = roll;
	gyroPitch = pitch;


	// Serial.println("ORIENTATION: ");
	// Serial.print("x: "); Serial.println(pitch);
	// Serial.print("y: "); Serial.println(roll);
	// Serial.print("z: "); Serial.println(yaw);
	Serial.print(roll); Serial.print("/");
	Serial.print(pitch); Serial.print("/");
	Serial.println(yaw);

	// Serial.print("ACC: ");
	// Serial.print(accX); Serial.print(" ");
	// Serial.print(accY); Serial.print(" ");
	// Serial.println(accZ);

	// Serial.print("GYRO: ");
	// Serial.print(gyroX); Serial.print(" ");
	// Serial.print(gyroY); Serial.print(" ");
	// Serial.println(gyroZ);
	// Serial.println();
	delay(2);
}

void	readAccel() {
	Wire.beginTransmission(MPU);
	Wire.write(0x3B);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 6, true);

	accX = (Wire.read() << 8 | Wire.read()) / 16384.0 - accBiasX;
	accY = (Wire.read() << 8 | Wire.read()) / 16384.0 - accBiasY;
	accZ = (Wire.read() << 8 | Wire.read()) / 16384.0 - accBiasZ;
}

void	readGyro() {
	Wire.beginTransmission(MPU);
	Wire.write(0x43);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 6, true);

	gyroX = (Wire.read() << 8 | Wire.read()) / 131.0 - gyroBiasX;
	gyroY = (Wire.read() << 8 | Wire.read()) / 131.0 - gyroBiasY;
	gyroZ = (Wire.read() << 8 | Wire.read()) / 131.0 - gyroBiasZ;
}

void computeAccAngles() {
	accRoll = atan2(accY, sqrt(accX*accX + accZ*accZ)) * 180 / PI;
	accPitch = atan2(-accX, sqrt(accY*accY + accZ*accZ)) * 180 / PI;
}

void readAccelRaw(float &x, float &y, float &z)
{
	Wire.beginTransmission(MPU);
	Wire.write(0x3B);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 6, true);

	x = (Wire.read() << 8 | Wire.read()) / 16384.0;
	y = (Wire.read() << 8 | Wire.read()) / 16384.0;
	z = (Wire.read() << 8 | Wire.read()) / 16384.0;
}

void readGyroRaw(float &x, float &y, float &z)
{
	Wire.beginTransmission(MPU);
	Wire.write(0x43);
	Wire.endTransmission(false);
	Wire.requestFrom(MPU, 6, true);

	x = (Wire.read() << 8 | Wire.read()) / 131.0;
	y = (Wire.read() << 8 | Wire.read()) / 131.0;
	z = (Wire.read() << 8 | Wire.read()) / 131.0;
}

void	calibrateIMU()
{
	const int samples = 500;
	float	ax, ay,az;
	float	gx,gy,gz;


	gyroBiasX = gyroBiasY = gyroBiasZ = 0;
	accBiasX = accBiasY = accBiasZ = 0;

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
	accBiasZ = (accBiasZ / samples) - 1.0f;
	Serial.println("IMU Calibrated");
}
