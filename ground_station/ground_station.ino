#include <SPI.h>
#include <LoRa.h>

// Pins used by most ESP32 LoRa boards (TTGO, Heltec)
#define ss 18
#define rst 14
#define dio0 26

void setup() {
  // Inicializamos a 115200 baudios (¡Importante para cuando programemos el script de Python!)
  Serial.begin(115200);
  while (!Serial);

  Serial.println("CanSat Ground Station - Initializing...");

  LoRa.setPins(ss, rst, dio0);
  
  // Initialize LoRa at 915 MHz (Must match the flight software)
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed! Check wiring/antenna.");
    while (1);
  }
  
  Serial.println("LoRa Initialization OK!");
  Serial.println("Waiting for CanSat telemetry...");
}

void loop() {
  // Intentar leer si ha llegado un paquete por el aire
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // Si llegó un paquete, lo leemos
    String incoming = "";

    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    // Imprimir lo que recibimos por puerto Serial
    // Como el CanSat nos envía un CSV, simplemente lo "pasamos" a la computadora
    Serial.println(incoming);
    
    // (Opcional) La intensidad de señal (RSSI) sirve para saber qué tan lejos está
    // Serial.print(" [RSSI: ");
    // Serial.print(LoRa.packetRssi());
    // Serial.println("]");
  }
}
