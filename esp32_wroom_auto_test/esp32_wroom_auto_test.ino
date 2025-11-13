#include <Arduino.h>

// ===== PWM/LEDC basic =====
const int LEDC_HZ = 50;
const int LEDC_BITS = 16;
const uint32_t TOP = (1UL << LEDC_BITS) - 1;
const uint32_t PERIOD_US = 20000;

// ===== test target: one wheel (RF) =====
const int PIN_LF = 23;
const int PIN_RF = 22;
const int PIN_LR = 19;
const int PIN_RR = 18;

// ===== control in microseconds =====
// Here are the default neutral pulse width

// int NEUTRAL_US = 1270;  // halt LF
// int NEUTRAL_US = 1273;  // halt RF
int NEUTRAL_US = 1179; // halt LR
// int NEUTRAL_US = 1175;  // halt RR

const int PIN_TEST = PIN_LR;

int DELTA_US = 30; // defaulf offset.

// ===== helpers =====
uint32_t usToDuty(uint16_t us)
{
  return (uint32_t)((uint64_t)us * TOP / PERIOD_US);
}
void writeUs(int pin, int us)
{
  us = constrain(us, 500, 2500);
  ledcWrite(pin, usToDuty(us));
}
void stopRF() { writeUs(PIN_TEST, NEUTRAL_US); }
void fwdRF() { writeUs(PIN_TEST, NEUTRAL_US + DELTA_US); }

void printStatus(const char *tag)
{
  Serial.print(tag);
  Serial.print("  NEUTRAL_US=");
  Serial.print(NEUTRAL_US);
  Serial.print("  DELTA_US=");
  Serial.print(DELTA_US);
  Serial.print("  (stop=");
  Serial.print(NEUTRAL_US);
  Serial.print(", fwd=");
  Serial.print(NEUTRAL_US + DELTA_US);
  Serial.println(")");
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\n=== One-wheel test (RF@GPIO22) ===");
  Serial.println("Keys: s=Stop, f=Forward, [ / ] = Neutral -/+1us, < / > = Delta -/+1us, p=print");

  ledcAttach(PIN_TEST, LEDC_HZ, LEDC_BITS);
  stopRF(); // at the start, use neutral pulse width given above.
  printStatus("INIT");
}

void loop()
{
  // wait for the input through serial monitor
  if (Serial.available())
  {
    char c = Serial.read();
    if (c == 's')
    {
      stopRF();
      printStatus("STOP");
    }
    if (c == 'f')
    {
      fwdRF();
      printStatus("FWD ");
    }
    if (c == '[')
    {
      NEUTRAL_US--;
      stopRF();
      printStatus("NEUT-");
    }
    if (c == ']')
    {
      NEUTRAL_US++;
      stopRF();
      printStatus("NEUT+");
    }
    if (c == '<')
    {
      if (DELTA_US > 0)
        DELTA_US--;
      printStatus("DLTA-");
    }
    if (c == '>')
    {
      DELTA_US++;
      printStatus("DLTA+");
    }
    if (c == 'p')
    {
      printStatus("STAT");
    }
  }

  // do nothing.
  delay(5);
}
