// NEMA17 + TMC2209 v3.0 demo for ESP32-C3 Super Mini (Arduino IDE).
// DIR  -> GPIO5
// STEP -> GPIO2
// EN   -> GPIO4 (active low, disable in idle to reduce noise)
// RX   -> GPIO21 (UART to TMC2209)
// TX   -> GPIO20 (UART to TMC2209)
// DIAG -> GPIO10 (StallGuard output)

#include <TMCStepper.h>

constexpr int kDirPin = 5;
constexpr int kStepPin = 2;
constexpr int kEnPin = 4;

constexpr int kUartRxPin = 21;
constexpr int kUartTxPin = 20;
constexpr int kDiagPin = 10;

constexpr int kRunMs = 2000;
constexpr int kPauseMs = 5000;

// Step timing (tune for your motor/driver setup).
constexpr int kStepDelayMicros = 800; // 625 steps per second.

// TMC2209 UART setup.
constexpr float kRsense = 0.11f;   // Typical for TMC2209 v3.0 modules.
constexpr uint8_t kDriverAddress = 0; // MS1/MS2 both low by default.
constexpr uint16_t kRunCurrentmA = 700;
constexpr uint8_t kHoldCurrentPercent = 20; // % of run current.
constexpr uint8_t kStallGuardThreshold = 8; // -64..63 (higher = more sensitive).
constexpr uint32_t kTcoolThrs = 0xFFFFF; // Enable StallGuard for all speeds.

TMC2209Stepper driver(&Serial1, kRsense, kDriverAddress);

void setup() {
  Serial.begin(115200);

  pinMode(kDirPin, OUTPUT);
  pinMode(kStepPin, OUTPUT);
  pinMode(kEnPin, OUTPUT);
  pinMode(kDiagPin, INPUT);

  digitalWrite(kEnPin, HIGH); // Disable driver until we start stepping.
  digitalWrite(kDirPin, HIGH); // One direction.
  digitalWrite(kStepPin, LOW);

  Serial1.begin(115200, SERIAL_8N1, kUartRxPin, kUartTxPin);

  driver.begin();
  driver.toff(4);
  driver.blank_time(24);
  driver.rms_current(kRunCurrentmA);
  const uint8_t runCurrent = 31;
  const uint8_t holdCurrent =
      (runCurrent * kHoldCurrentPercent) / 100;
  driver.IHOLD_IRUN(holdCurrent, runCurrent, 8);
  driver.en_spreadCycle(true); // StallGuard works in spreadCycle.
  driver.TCOOLTHRS(kTcoolThrs);
  driver.SGTHRS(kStallGuardThreshold);
  driver.diag1_stall(true);
  driver.diag1_active_high(true);
  driver.diag1_pushpull(true);
}

void runMotorFor(unsigned long durationMs) {
  const unsigned long totalSteps =
      (durationMs * 1000UL) / (kStepDelayMicros * 2UL);

  for (unsigned long i = 0; i < totalSteps; ++i) {
    if (digitalRead(kDiagPin) == HIGH) {
      Serial.println("STALL DETECTED");
      break;
    }

    digitalWrite(kStepPin, HIGH);
    delayMicroseconds(kStepDelayMicros);
    digitalWrite(kStepPin, LOW);
    delayMicroseconds(kStepDelayMicros);
  }
}

void loop() {
  digitalWrite(kEnPin, LOW); // Enable driver (active low).
  runMotorFor(kRunMs);
  digitalWrite(kEnPin, HIGH); // Disable driver in idle.
  delay(kPauseMs);
}
