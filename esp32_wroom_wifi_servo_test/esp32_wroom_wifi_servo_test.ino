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