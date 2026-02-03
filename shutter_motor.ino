constexpr int kEnPin = 4;
constexpr int kPauseMs = 1000;

void runMotorFor() {
  // TODO: add stepper control logic.
}

void setup() {
  pinMode(kEnPin, OUTPUT);
  digitalWrite(kEnPin, HIGH);
}

void loop() {
  digitalWrite(kEnPin, LOW);
  runMotorFor();
  digitalWrite(kEnPin, HIGH);
  delay(kPauseMs);
}
