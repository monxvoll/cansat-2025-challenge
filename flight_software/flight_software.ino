#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <RadioLib.h>

SX1262 radio = new Module(8, 14, 12, 13);

TwoWire I2Cbus = TwoWire(0);
Adafruit_BMP280 bme(&I2Cbus);
Adafruit_MPU6050 mpu;

int packetCounter = 0;
bool txDone = false;

void txCallback(void) { txDone = true; }

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("CanSat Flight — Iniciando...");

  I2Cbus.begin(41, 42, 400000);
  delay(200);

  // MPU6050
// Prueba con 0x68 primero, si falla prueba 0x69
if (!mpu.begin(0x68, &I2Cbus)) {
    if (!mpu.begin(0x69, &I2Cbus)) {
        Serial.println("MPU6050 no encontrado en 0x68 ni 0x69!");
    } else {
        Serial.println("MPU6050 OK! (0x69)");
    }
} else {
    Serial.println("MPU6050 OK! (0x68)");
}

  // BMP280
  if (!bme.begin(0x76)) {
    if (!bme.begin(0x77)) {
      Serial.println("BMP280 no encontrado!");
    } else {
      Serial.println("BMP280 OK! (0x77)");
    }
  } else {
    Serial.println("BMP280 OK! (0x76)");
  }

  // LoRa — mismos parámetros que el ground
  int state = radio.begin(915.0, 125.0, 7, 5, 0xAB, 20);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("LoRa falló, código: ");
    Serial.println(state);
    while (true);
  }
  radio.setDio1Action(txCallback);
  Serial.println("LoRa OK! Transmitiendo...");
}

void loop() {
  // Leer MPU6050
  sensors_event_t a, g, temp_mpu;
  mpu.getEvent(&a, &g, &temp_mpu);

  float ax = a.acceleration.x, ay = a.acceleration.y, az = a.acceleration.z;
  float gx = g.gyro.x,         gy = g.gyro.y,         gz = g.gyro.z;

  // Leer BMP280
  float temp  = bme.readTemperature();
  float press = bme.readPressure() / 100.0;
  float alt   = bme.readAltitude(1013.25);

  // Armar CSV
  String payload = String(packetCounter)    + "," +
                   String(ax, 2) + "," + String(ay, 2) + "," + String(az, 2) + "," +
                   String(gx, 4) + "," + String(gy, 4) + "," + String(gz, 4) + "," +
                   String(temp, 2) + "," + String(press, 2) + "," + String(alt, 2);

  // Transmitir
  txDone = false;
  int state = radio.startTransmit(payload);

  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("Error TX, código: ");
    Serial.println(state);
  } else {
    unsigned long t = millis();
    while (!txDone && millis() - t < 3000) delay(1);

    if (txDone) {
      radio.finishTransmit();
      Serial.println("TX OK: " + payload);
    } else {
      radio.standby();
      Serial.println("Timeout TX");
    }
  }

  packetCounter++;
  delay(1000);
}