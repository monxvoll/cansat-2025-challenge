#include <RadioLib.h>

SX1262 radio = new Module(8, 14, 12, 13);

bool rxFlag = false;

void rxCallback(void) { rxFlag = true; }

// Setup pins:
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("CanSat Ground Station — Iniciando...");

  int state = radio.begin(915.0, 125.0, 7, 5, 0xAB, 20);
  if (state != RADIOLIB_ERR_NONE) {
    Serial.print("LoRa falló, código: ");
    Serial.println(state);
    while (true)
      ;
  }

  radio.setDio1Action(rxCallback);
  radio.startReceive();
  Serial.println("LoRa OK! Esperando paquetes...");
}

// Main loop
void loop() {
  if (rxFlag) {
    rxFlag = false;

    String incoming = "";
    int state = radio.readData(incoming);

    if (state == RADIOLIB_ERR_NONE) {
      Serial.println(incoming);
    } else {
      Serial.print("Error RX, código: ");
      Serial.println(state);
    }

    // Waiting for new packets
    radio.startReceive();
  }
}
