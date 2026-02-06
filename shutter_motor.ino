// NEMA17 + TMC2209 v3.0 demo for ESP32-C3 Super Mini (Arduino IDE).
// DIR  -> GPIO5
// STEP -> GPIO2
// EN   -> GPIO4 (active low, disable in idle to reduce noise)
// RX   -> GPIO21 (UART to TMC2209)
// TX   -> GPIO20 (UART to TMC2209)
// DIAG -> GPIO10 (StallGuard output)

#include <TMC2209.h>

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
constexpr int8_t kStallGuardThreshold = -16; // -64..63 (higher = more sensitive).
constexpr uint32_t kTcoolThrs = 0xFFFFF; // Enable StallGuard for all speeds.
constexpr uint16_t kStallResultLimit = 1; // Lower value = closer to stall.
constexpr uint16_t kStallResultValidMin = 1; // Ignore SG_RESULT=0 as invalid.
constexpr uint16_t kStallReportIntervalMs = 200;
constexpr bool kLogSgResult = false;
constexpr uint16_t kStallIgnoreMs = 800; // Ignore stall checks just after start.
constexpr uint8_t kStallConfirmCount = 3;

TMC2209 driver;

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

  driver.setup(Serial1, 115200, kRsense, kDriverAddress);
  driver.setRunCurrent(kRunCurrentmA);
  driver.setHoldCurrent((kRunCurrentmA * kHoldCurrentPercent) / 100);
  driver.setHoldDelay(8);
  driver.setSpreadCycle(true); // StallGuard works in spreadCycle.
  driver.setCoolStepThreshold(kTcoolThrs);
  driver.setStallGuardThreshold(kStallGuardThreshold);
  // DIAG pin is configured on the driver by board defaults; we read it directly.
}

void runMotorFor(unsigned long durationMs) {
  const unsigned long totalSteps =
      (durationMs * 1000UL) / (kStepDelayMicros * 2UL);
  unsigned long lastReportMs = 0;
  const unsigned long startMs = millis();
  uint8_t sgLowCount = 0;
  uint8_t diagHighCount = 0;

  for (unsigned long i = 0; i < totalSteps; ++i) {
    const unsigned long nowMs = millis();
    const bool stallChecksEnabled = (nowMs - startMs) >= kStallIgnoreMs;
    if (stallChecksEnabled && (nowMs - lastReportMs) >= kStallReportIntervalMs) {
      lastReportMs = nowMs;
      const uint16_t sgResult = driver.getStallGuardResult();
      if (kLogSgResult) {
        Serial.print("SG_RESULT=");
        Serial.println(sgResult);
      }
      if (sgResult >= kStallResultValidMin) {
        if (sgResult <= kStallResultLimit) {
          ++sgLowCount;
        } else {
          sgLowCount = 0;
        }
      } else {
        sgLowCount = 0;
      }
    }

    if (stallChecksEnabled && digitalRead(kDiagPin) == HIGH) {
      ++diagHighCount;
      if (diagHighCount >= kStallConfirmCount) {
        Serial.println("STALL CANDIDATE (DIAG)");
      }
    } else {
      diagHighCount = 0;
    }

    if (sgLowCount >= kStallConfirmCount &&
        diagHighCount >= kStallConfirmCount) {
      Serial.println("STALL DETECTED (SG_RESULT + DIAG)");
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
