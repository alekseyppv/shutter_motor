// NEMA17 + TMC2209 v3.0 demo for ESP32-C3 Super Mini (Arduino IDE).
// DIR  -> GPIO5
// STEP -> GPIO2
// EN   -> GND (always enabled)
// RX   -> GPIO21 (reserved, not used yet)
// TX   -> GPIO20 (reserved, not used yet)
// DIAG -> GPIO10 (reserved, not used yet)

constexpr int kDirPin = 5;
constexpr int kStepPin = 2;

// Reserved UART/diagnostic pins for future use (not used in this sketch).
constexpr int kUartRxPin = 21;
constexpr int kUartTxPin = 20;
constexpr int kDiagPin = 10;

constexpr int kRunMs = 2000;
constexpr int kPauseMs = 5000;

// Step timing (tune for your motor/driver setup).
constexpr int kStepDelayMicros = 800; // 625 steps per second.

void setup() {
  pinMode(kDirPin, OUTPUT);
  pinMode(kStepPin, OUTPUT);

  digitalWrite(kDirPin, HIGH); // One direction.
  digitalWrite(kStepPin, LOW);
}

void runMotorFor(unsigned long durationMs) {
  const unsigned long totalSteps =
      (durationMs * 1000UL) / (kStepDelayMicros * 2UL);

  for (unsigned long i = 0; i < totalSteps; ++i) {
    digitalWrite(kStepPin, HIGH);
    delayMicroseconds(kStepDelayMicros);
    digitalWrite(kStepPin, LOW);
    delayMicroseconds(kStepDelayMicros);
  }
}

void loop() {
  runMotorFor(kRunMs);
  delay(kPauseMs);
}
