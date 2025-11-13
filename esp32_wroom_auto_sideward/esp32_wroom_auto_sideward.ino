#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "../secrets.h"

// ==== PWM/LEDC ====
const int LEDC_HZ=50, LEDC_BITS=16;
const uint32_t TOP=(1UL<<LEDC_BITS)-1, PERIOD_US=20000;

// Wheels: LF, RF, LR, RR
const int PIN_LF=23, PIN_RF=22, PIN_LR=19, PIN_RR=18;

// base pulse width for all wheels
int NEUTRAL_US=1000; 
int DELTA_US  =25;   

// offsets for each wheel 
int TRIM_STOP_US[4]={301,160,175,173}; 
int TRIM_FWD_US [4]={42,40,32,36};     

// direction of roatete. 1 : one direction, -1 : the other direction
int DIR[4]={ -1, +1, -1, +1 };  // {LF,RF,LR,RR}

uint32_t usToDuty(uint16_t us){ return (uint32_t)((uint64_t)us*TOP/PERIOD_US); }
void writeUs(int pin,int us){ us=constrain(us,500,2500); ledcWrite(pin, usToDuty(us)); }

void stopAll(){
  writeUs(PIN_LF, NEUTRAL_US + TRIM_STOP_US[0]);
  writeUs(PIN_RF, NEUTRAL_US + TRIM_STOP_US[1]);
  writeUs(PIN_LR, NEUTRAL_US + TRIM_STOP_US[2]);
  writeUs(PIN_RR, NEUTRAL_US + TRIM_STOP_US[3]);
}

// run forward
void forwardAll(){
  int uLF = NEUTRAL_US + TRIM_STOP_US[0] + DIR[0]* (DELTA_US + TRIM_FWD_US[0]);
  int uRF = NEUTRAL_US + TRIM_STOP_US[1] + DIR[1]* (DELTA_US + TRIM_FWD_US[1]);
  int uLR = NEUTRAL_US + TRIM_STOP_US[2] + DIR[2]* (DELTA_US + TRIM_FWD_US[2]);
  int uRR = NEUTRAL_US + TRIM_STOP_US[3] + DIR[3]* (DELTA_US + TRIM_FWD_US[3]);
  writeUs(PIN_LF,uLF); writeUs(PIN_RF,uRF); writeUs(PIN_LR,uLR); writeUs(PIN_RR,uRR);
}

// right strafe
void rightAll(){
  const int S[4] = { +1, -1, -1, +1 }; // parity（LF,RF,LR,RR）
  int uLF = NEUTRAL_US + TRIM_STOP_US[0] + S[0]*DIR[0]* (DELTA_US + TRIM_FWD_US[0]);
  int uRF = NEUTRAL_US + TRIM_STOP_US[1] + S[1]*DIR[1]* (DELTA_US + TRIM_FWD_US[1]);
  int uLR = NEUTRAL_US + TRIM_STOP_US[2] + S[2]*DIR[2]* (DELTA_US + TRIM_FWD_US[2]);
  int uRR = NEUTRAL_US + TRIM_STOP_US[3] + S[3]*DIR[3]* (DELTA_US + TRIM_FWD_US[3]);
  writeUs(PIN_LF,uLF); writeUs(PIN_RF,uRF); writeUs(PIN_LR,uLR); writeUs(PIN_RR,uRR);
}

// ==== Web ====
WebServer server(80);
const char PAGE[] PROGMEM = R"HTML(
<!doctype html><meta name=viewport content="width=device-width,initial-scale=1">
<title>Andechser-1</title>
<style>
body{font-family:sans-serif;margin:20px;text-align:center}
button{font-size:1.2rem;padding:.7rem 1.4rem;margin:8px}
pre{background:#f4f4f4;padding:8px;white-space:pre-wrap;text-align:left}
</style>
<h1>Andechser-1</h1>
<div>
  <button onclick="fetch('/forward').then(()=>update())">Forward</button>
  <button onclick="fetch('/stop').then(()=>update())">Stop</button>
  <button onclick="fetch('/right').then(()=>update())">Right</button>
</div>
<pre id=st>loading...</pre>
<script>
function update(){ fetch('/status').then(r=>r.text()).then(t=>st.textContent=t); }
update();
</script>
)HTML";

void handleRoot(){ server.send_P(200,"text/html",PAGE); }
void handleForward(){ forwardAll(); server.send(200,"text/plain","FWD"); }
void handleRight(){ rightAll(); server.send(200,"text/plain","RIGHT"); }
void handleStop(){ stopAll(); server.send(200,"text/plain","STOP"); }
void handleStatus(){
  String s;
  s+="IP="+WiFi.localIP().toString();
  s+="\nNEUTRAL_US="+String(NEUTRAL_US)+"  DELTA_US="+String(DELTA_US);
  s+="\nDIR={"+String(DIR[0])+","+String(DIR[1])+","+String(DIR[2])+","+String(DIR[3])+"}";
  s+="\nPins  LF="+String(PIN_LF)+" RF="+String(PIN_RF)+" LR="+String(PIN_LR)+" RR="+String(PIN_RR);
  server.send(200,"text/plain",s);
}

void setup(){
  Serial.begin(115200);
  ledcAttach(PIN_LF,LEDC_HZ,LEDC_BITS);
  ledcAttach(PIN_RF,LEDC_HZ,LEDC_BITS);
  ledcAttach(PIN_LR,LEDC_HZ,LEDC_BITS);
  ledcAttach(PIN_RR,LEDC_HZ,LEDC_BITS);
  stopAll();

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  while(WiFi.status()!=WL_CONNECTED){ delay(300); Serial.print("."); }
  Serial.printf("\nIP: %s\n", WiFi.localIP().toString().c_str());
  if(MDNS.begin("esp32")) Serial.println("mDNS: http://esp32.local/");

  server.on("/",handleRoot);
  server.on("/forward",handleForward);
  server.on("/right",handleRight);
  server.on("/stop",handleStop);
  server.on("/status",handleStatus);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(){ server.handleClient(); }
