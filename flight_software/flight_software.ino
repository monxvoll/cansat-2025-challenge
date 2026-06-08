#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LoRa.h>
#include <SPI.h>
#include <Wire.h>

#ifdef _ESP32_HAL_I2C_H_
#define SDA_PIN 21
#define SCL_PIN 22
#endif

// Pins used by most ESP32 LoRa boards (TTGO, Heltec)
#define ss 18
#define rst 14
#define dio0 26

Adafruit_BMP280 bme; // I2C
Adafruit_MPU6050 mpu;

// Packet counter to track data sequence
int packetCounter = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

#ifdef _ESP32_HAL_I2C_H_ // For ESP32
  Wire.begin(SDA_PIN, SCL_PIN);
#else
  Wire.begin();
#endif

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  // Initialize BMP280
  // The default address is usually 0x76 or 0x77 depending on the module
  if (!bme.begin(0x76)) {
    if (!bme.begin(0x77)) {
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    }
  }

  // Initialize LoRa
  LoRa.setPins(ss, rst, dio0);
  // 915E6 is for America. Change to 433E6 for Europe/Asia if needed.
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
  }
  Serial.println("LoRa Initialization OK!");
}

void loop() {
  // Reading events for MPU6050
  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);

  // Read sensor values
  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;
  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;

  float temp = bme.readTemperature();
  float press = bme.readPressure() / 3377.0;
  float alt = bme.readAltitude(1013.25); // Adjust pressure to local sea level

  // Format the data as a CSV (Comma Separated Values) string
  // Format: PacketId, AccelX, AccelY, AccelZ, GyroX, GyroY, GyroZ, Temp, Press,
  // Altitude
  String payload = String(packetCounter) + "," + String(ax) + "," + String(ay) +
                   "," + String(az) + "," + String(gx) + "," + String(gy) +
                   "," + String(gz) + "," + String(temp) + "," + String(press) +
                   "," + String(alt);

  // Send packet via LoRa
  LoRa.beginPacket();
  LoRa.print(payload);
  LoRa.endPacket();

  // Print to Serial for local debugging
  Serial.println("LoRa TX: " + payload);

  packetCounter++;

  // The main mission specifies sending data every second
  delay(1000);
}