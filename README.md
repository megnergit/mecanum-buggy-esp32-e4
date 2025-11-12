# Mecanum Buggy - Andechser 1

---
## Motivation

The beginnig was a video on this NVIDIA [web page](https://blogs.nvidia.com/blog/fraunhofer-research-robotics/) (at around 0:38). 


This web page was about NVIDIA Omniverse, and how one can 
use it to simulate a hardware to accelerate the development. 

But what caught my eyes is the [Mecanum wheel](https://en.wikipedia.org/wiki/Mecanum_wheel). 

Mecanum wheels enable a vehicle to run sideway straight, while 
looking exactly to front. 

This makes staring irrelevant for a vehicle. One can remove the staring mechanism as a whole. One can also forget about turning radius of a vehicle. 

We only need a corridor in a warehouse that is exactly the same width with the vehicle. 

I first thought how cool Fraunhofer Institute is, but it turns out Mecanum wheel was invented by a Swedish engineer Bengt Erland, and was patented in 1970s ('Mecanum'  is the name of the company). 

I wanted to reproduce a buggy with Mecanum wheels
and would like to see it runs sideway with my own eyes. 
I will use ESP32 to operate the buggy over WiFi. 

---
## Operate SG90 with wire

### What we need

- ESP32 Wroom
- SG90 servo motor
- 18650 Li-ion battery with shield
- bread board
- wires and pins

![wiring](./images/wiring-1.jpg)

---
### Wiring

| SG90 (servo motor)  |  ESP32  |
|---------------------|---------|
| brown               | GND     |
| red                 | 5V of Li-ion 18650|
| orange              | GPIO 23 |


Make sure **connect** GND of ESP32 and GND of Li-ion battery.


---
### Code

... is written by my Charlie (=ChatGPT).

```cpp

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

```

Choose the right board and the port to upload the code. In my environment they are following. 

|        |                        |   
|--------|------------------------|
| board  |ESP32 Dev Module        | 
| port   | /dev/cu.SLAB_USBtoUART | 


![Board](./images/esp32-board-1.png)

![Port](./images/esp32-port-1.png)

Then, check if the code compiles. 

![Compile](./images/compile-1.png)

Then upload it. 

![Uplaod](./images/upload-1.png)


**NOTE!**

There are two types of USB mini cable. 
One transmits data and power, and the other only power. The latter cannot be used to to write 
Arduino program on to  ESP32 chip. 

Two cables look exactly the same. 
Use the one that is "known" to be a data cable. 
Otherwise one can easily [waste a day](https://github.com/megnergit/AWS_IoT_ESP32_E2)

If the arm of SG90 motor waves (= go to 180 deg and then back to 0, and repeat the move forever),
the system is working fine. 


---
## Operate SG90 servo motor over WiFi

## Code

The wiring is the same.  

```cpp




```

---
# Appendix
##


---
# END
---
