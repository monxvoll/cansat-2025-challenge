#include <LoRa.h>
#include <SPI.h>

// Pins used by most ESP32 LoRa boards (TTGO, Heltec)
#define ss 18
#define rst 14
#define dio0 26

void setup() {
  // Initialize serial communication at 115200 baud
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("CanSat Ground Station - Initializing...");

  LoRa.setPins(ss, rst, dio0);

  // Initialize LoRa at 915 MHz (Must match the flight software)
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed! Check wiring/antenna.");
    while (1)
      ;
  }

  Serial.println("LoRa Initialization OK!");
  Serial.println("Waiting for CanSat telemetry...");
}

void loop() {
  // Try to read if a packet has arrived through the air
  int packetSize = LoRa.parsePacket();

  if (packetSize) {
    // If a packet has arrived, we read it
    String incoming = "";

    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    // Print what we receive via Serial port
    // Since the CanSat sends us a CSV, we simply "pass it" to the computer
    Serial.println(incoming);
  }
