#include <ESP32Servo.h>

Servo servo;

const int SERVO_PIN = 23;          // signal line to Servo. GPIO
// the with of the pulse. the pulse with is between 500 us and 2400 us for many models. 
const int MIN_US = 500;            // min pulse width in micro sec.
const int MAX_US = 2400;           // max pulse width in micro sec. 

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("Start");
  // initialize ESP32 PWM timer. the clock is usually 50Hz.
  // esp32 servo does this internally automatically, but we will
  // explicitly do it ourselves. 

  servo.setPeriodHertz(50);
  int ok = servo.attach(SERVO_PIN, MIN_US, MAX_US);
  Serial.printf("attach=%d auf GPIO %d\n", ok, SERVO_PIN);

  servo.write(90);
  delay(1000);
}

void loop() {
  // move the arm from 0 deg to 180 deg.
  for (int angle = 0; angle <= 180; angle++) {
    servo.write(angle);
    Serial.print(angle);
    delay(10); // wait 10 ms.
  }
  // move the arm from 180 deg to 0 deg.
  for (int angle = 180; angle >= 0; angle--) {
    servo.write(angle);
    Serial.print(angle);
    delay(10);
  }
}
