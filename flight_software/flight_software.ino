#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#ifdef _ESP32_HAL_I2C_H_
#define SDA_PIN 21
#define SCL_PIN 22
#endif

Adafruit_BMP280 bme; // I2C
Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

#ifdef _ESP32_HAL_I2C_H_ // For ESP32
  Wire.begin(SDA_PIN, SCL_PIN);
#else
  Wire.begin();
#endif

  // Inicializar MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  // Inicializar BMP280
  // La dirección por defecto suele ser 0x76 o 0x77 dependiendo del módulo
  if (!bme.begin(0x76)) {
    if (!bme.begin(0x77)) {
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    }
  }
}

void loop() {
  // Eventos de lectura para MPU6050
  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);

  // Leer y enviar datos del acelerómetro
  Serial.print("accelX: " + String(a.acceleration.x));
  Serial.print("\taccelY: " + String(a.acceleration.y));
  Serial.print("\taccelZ: " + String(a.acceleration.z));

  // Leer y enviar datos del giroscopio
  Serial.print("\tgyroX: " + String(g.gyro.x));
  Serial.print("\tgyroY: " + String(g.gyro.y));
  Serial.print("\tgyroZ: " + String(g.gyro.z));

  // Leer y enviar datos del BMP280
  Serial.print("\tTemperature(*C): ");
  Serial.print(bme.readTemperature());

  Serial.print("\tPressure(Inches(Hg)): ");
  Serial.print(bme.readPressure() / 3377.0);

  Serial.print("\tApproxAltitude(m): ");
  Serial.print(
      bme.readAltitude(1013.25)); // Ajustar presión al nivel del mar local

  Serial.println(""); // Salto de línea para el siguiente paquete de datos

  // La misión principal especifica enviar datos cada segundo
  delay(1000);
}