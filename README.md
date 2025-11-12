# Mecanum Buggy - Run Andechser 1

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

Note,

1. Authentication information is stored in ```secrets.h```.
2. ```secrets.h``` is not commited, but please edit your ```secrets_example.h``` to suit your WiFi.

The directory structure is so far as follows. 

```sh
$ tree . -L 2
.
├── esp32_wroom_wifi_servo_test
│   └── esp32_wroom_wifi_servo_test.ino
├── esp32_wroom_wired_servo_test
│   └── esp32_wroom_wired_servo_test.ino
├── images
│   ├── compile-1.png
│   ├── esp32-board-1.png
│   ├── esp32-port-1.png
│   ├── li-ion-battery-1.jpg
│   ├── Photo on 11.11.25 at 7.14 PM.jpg
│   ├── sg90-1.jpg
│   ├── upload-1.png
│   └── wiring-1.jpg
├── LICENSE
├── MECANUM
├── README.md
├── secrets_example.h
└── secrets.h

4 directories, 15 files
```

Charlie wrote the code for me. I included NTP, but it is a matter of taste. 

```cpp

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <time.h>

// WIFI_SSID and WIFI_PASSWORD are defined in secrets.h
// secrets.h will not be commited. 
#include "../secrets.h" 

const char* ntp = "pool.ntp.org";
// just in case, we set the clock correctly.
const long gmtOffset = 3600;
const int  dstOffset = 3600;

// ==== servo ====
const int SERVO_PIN = 23;
const int LEDC_HZ   = 50;
const int LEDC_BITS = 16;
const uint32_t TOP  = (1UL << LEDC_BITS) - 1;
const uint32_t PERIOD_US = 20000;

// ---- pulse range translates to which direction the motor should rotate and how fast. 
const uint16_t SERVO_MIN_US = 500;          // min pulse width in micro second
const uint16_t SERVO_MAX_US = 2500;         // max pulse width in micro second

// This is the server that runs inside ESP32
WebServer server(80);

uint32_t usToDuty(uint16_t us){
  return (uint32_t)((uint64_t)us * TOP / PERIOD_US);
}

uint16_t angleToUs(int angle) {
  angle = constrain(angle, 0, 180);
  return SERVO_MIN_US +
         (uint16_t)((SERVO_MAX_US - SERVO_MIN_US) * (angle / 180.0));
}
// ==== LEDC has two verwions, ver. 2 and 3. 
// here we use the functions available only in ver. 3
#define LEDC_ATTACH(pin,freq,bits)  ledcAttach((pin),(freq),(bits))
#define LEDC_WRITE(pin,duty)        ledcWrite((pin),(duty))

// ---- UI ----
const char PAGE[] PROGMEM = R"HTML(
<!doctype html><meta name=viewport content="width=device-width,initial-scale=1">
<title>ESP32 Servo</title>
<style>body{font-family:sans-serif;margin:24px}.v{font-weight:700}</style>
<h1>ESP32 Servo</h1>
<p>Angle: <span class=v id=v>90</span> deg</p>
<input id=s type=range min=0 max=180 value=90
 oninput="v.textContent=this.value;fetch('/set?angle='+this.value).catch(()=>{})">
)HTML";

void handleRoot(){ server.send_P(200,"text/html",PAGE); }
void handleSet(){
  if(!server.hasArg("angle")){ server.send(400,"text/plain","missing angle"); return; }
  int angle = constrain(server.arg("angle").toInt(),0,180);
  uint16_t us = angleToUs(angle);
  LEDC_WRITE(SERVO_PIN, usToDuty(us));
  server.send(200,"text/plain",String(angle));
}

void setup(){
  Serial.begin(115200);
  // initialize LEDC (ver. 3)
  LEDC_ATTACH(SERVO_PIN, LEDC_HZ, LEDC_BITS);
  LEDC_WRITE(SERVO_PIN, usToDuty(angleToUs(90))); // this should be the neutral position. arm should not move with this pulse. 

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){ delay(500); Serial.print("."); }
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());\
  Serial.println(WiFi.BSSIDstr());
  Serial.println(WiFi.channel());

  WiFi.setSleep(false);

  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("HTTP server started");



  // NTP
  configTime(gmtOffset, dstOffset, ntp);
  // if we want to make UTC to CET or CEST
  setenv("TZ", "CET-1CEST,M3.5.0/2,M10.5.0/3", 1); tzset();

  // synchronize the clock.
  struct tm t;
  for (int i=0;i<10 && !getLocalTime(&t); ++i) { delay(500); }
  if (getLocalTime(&t)) {
    char buf[32];
    strftime(buf,sizeof(buf),"%F %T",&t);
    Serial.printf("Time synced: %s\n", buf);
  } else {
    Serial.println("Time sync failed (yet).");
  }

}

void loop(){ server.handleClient(); }

```

Upload the code as before. Open serial monitor by clicking here. 

![Start serial monitor](./images/serial-monitor-1.png)

Then you will see something like this. 

![Serial monitor output](./images/serial-monitor-2.png)

Open the IP ```192.168.178.85``` in this case as you see 
in the serial monitor (yours will be different). 

![UI](./images/ui-1.png)

Move the slider, and see the arm of SG90 servo motor  
rotates accordingly. 

**NOTE!**

When you encounter that No Route or path found to 
the host, check, Local Network, and make sure
that Arduino IDE and your browser (here Google Chrome)
are allowed to access to the local network (= your home network). 

![Local Network](./images/local-network-1.png)

![Local Network](./images/local-network-2.png)







---
# Appendix
##


---
# END
---

