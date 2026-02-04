#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

/* ================= PIN ================= */
#define LED_PIN 2
#define BTN_PIN 0

/* ================= PWM ================= */
#define LED_CHANNEL 0
#define LED_FREQ    5000
#define LED_RES     8   // 8-bit (0–255)

/* ================= MQTT ================= */
const char* mqttServer = "public.cloud.shiftr.io";
const int   mqttPort   = 1883;
const char* mqttUser   = "public";
const char* mqttPass   = "public";

/* ================= OBJECT ================= */
WebServer server(80);
Preferences prefs;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* ================= GLOBAL STATE ================= */
String ssid, password;

bool lampuKamarState = false;
bool lampuTamuState  = false;

bool *lampuKamar = &lampuKamarState;
bool *lampuTamu  = &lampuTamuState;

/* ================= INTERRUPT ================= */
void IRAM_ATTR toggleLampuKamar() {
  *lampuKamar = !(*lampuKamar);
  Serial.println("[INTERRUPT] Toggle Lampu Kamar");
}

/* ================= MQTT CALLBACK ================= */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (uint8_t i = 0; i < length; i++) msg += (char)payload[i];

  Serial.printf("[MQTT] %s => %s\n", topic, msg.c_str());

  if (String(topic) == "smarthome/esp32/lampu/kamar") {
    *lampuKamar = (msg == "ON");
    Serial.println("[MQTT] Lampu Kamar " + msg);
  }

  if (String(topic) == "smarthome/esp32/lampu/tamu") {
    *lampuTamu = (msg == "ON");
    Serial.println("[MQTT] Lampu Tamu " + msg);
  }
}

/* ================= MQTT CONNECT ================= */
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.println("[MQTT] Connecting...");
    if (mqttClient.connect("ESP32_SMARTHOME", mqttUser, mqttPass)) {
      Serial.println("[MQTT] Connected");
      mqttClient.subscribe("smarthome/esp32/lampu/kamar");
      mqttClient.subscribe("smarthome/esp32/lampu/tamu");
    } else {
      delay(2000);
    }
  }
}

/* ================= RTOS TASK: MQTT ================= */
void taskMQTT(void *pv) {
  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      if (!mqttClient.connected()) connectMQTT();
      mqttClient.loop();
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

/* ================= RTOS TASK: LED (PWM) ================= */
void taskLED(void *pv) {
  for (;;) {
    int duty = (*lampuKamar || *lampuTamu) ? 0 : 255;
    ledcWrite(LED_CHANNEL, 0);
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

/* ================= WEB PAGE ================= */
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>ESP32 Smart Home</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body{font-family:Arial;background:linear-gradient(135deg,#dbeafe,#eef2ff);padding:20px}
.card{max-width:420px;margin:auto;background:#fff;padding:18px;border-radius:14px;box-shadow:0 6px 18px rgba(0,0,0,.15)}
.header{border:1px solid #c7d2fe;background:#e0e7ff;padding:12px;border-radius:10px;text-align:center;font-weight:bold;font-size:20px}
.box{border:1px solid #ddd;border-radius:10px;padding:12px;margin-bottom:14px;background:#f8fafc}
button,input{width:100%;padding:12px;margin:6px 0;border-radius:6px;border:none}
.on{background:#2ecc71;color:#fff}
.off{background:#e74c3c;color:#fff}
.save{background:#64748b;color:#fff}
</style>
</head>
<body>
<div class="card">
<div class="header">ESP32 Dashboard</div>

<div class="box">
<h3>Status</h3>
<p>WiFi : <b id="wifi">-</b></p>
<p>IP : <b id="ip">-</b></p>
<p>Connected To : <b id="ssid">-</b></p>
</div>

<div class="box">
<h3>LED Status</h3>
<p>Lampu Kamar : <b id="kamar">-</b></p>
<p>Lampu Ruang Tamu : <b id="tamu">-</b></p>
<p>LED ESP32 : <b id="led">-</b></p>
</div>

<div class="box">
<h3>WiFi Configuration</h3>
<input id="ssidInput" placeholder="SSID">
<input id="password" type="password" placeholder="Password">
<button class="save" onclick="saveWiFi()">Save WiFi</button>
</div>

<h3>Kontrol Lampu</h3>
<button class="on" onclick="fetch('/lampu/kamar/on',{method:'POST'})">Lampu Kamar ON</button>
<button class="off" onclick="fetch('/lampu/kamar/off',{method:'POST'})">Lampu Kamar OFF</button>
<button class="on" onclick="fetch('/lampu/tamu/on',{method:'POST'})">Lampu Ruang Tamu ON</button>
<button class="off" onclick="fetch('/lampu/tamu/off',{method:'POST'})">Lampu Ruang Tamu OFF</button>

</div>

<script>
function refresh(){
 fetch('/status').then(r=>r.json()).then(d=>{
  wifi.innerText=d.wifi;
  ip.innerText=d.ip;
  ssid.innerText=d.ssid;
  kamar.innerText=d.kamar?'ON':'OFF';
  tamu.innerText=d.tamu?'ON':'OFF';
  led.innerText=d.led?'ON':'OFF';
 });
}
function saveWiFi(){
 fetch('/wifi',{
  method:'POST',
  headers:{'Content-Type':'application/json'},
  body:JSON.stringify({ssid:ssidInput.value,password:password.value})
 });
 alert('WiFi disimpan');
}
setInterval(refresh,2000);refresh();
</script>
</body>
</html>
)rawliteral";

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32 BOOTING ===");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  attachInterrupt(BTN_PIN, toggleLampuKamar, FALLING);

  /* ===== PWM INIT ===== */
  ledcAttach(LED_PIN, LED_FREQ, LED_RES);

  prefs.begin("wifi", false);
  ssid = prefs.getString("ssid","");
  password = prefs.getString("password","");

  if (ssid == "") startAPMode();
  else connectWiFi();

  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);

  server.on("/", [](){ server.send_P(200,"text/html",webpage); });

  server.on("/wifi", HTTP_POST, [](){
    DynamicJsonDocument doc(256);
    deserializeJson(doc, server.arg("plain"));
    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();
    prefs.putString("ssid",ssid);
    prefs.putString("password",password);
    Serial.println("[WIFI] Credentials saved");
    Serial.println("[WIFI] SSID: " + ssid);
    server.send(200,"application/json","{\"status\":\"ok\"}");
    connectWiFi();
  });

  server.on("/lampu/kamar/on", HTTP_POST, [](){
    *lampuKamar=true;
    mqttClient.publish("smarthome/esp32/lampu/kamar","ON");
    Serial.println("[WEB] Lampu Kamar ON");
    server.send(200);
  });

  server.on("/lampu/kamar/off", HTTP_POST, [](){
    *lampuKamar=false;
    mqttClient.publish("smarthome/esp32/lampu/kamar","OFF");
    Serial.println("[WEB] Lampu Kamar OFF");
    server.send(200);
  });

  server.on("/lampu/tamu/on", HTTP_POST, [](){
    *lampuTamu=true;
    mqttClient.publish("smarthome/esp32/lampu/tamu","ON");
    Serial.println("[WEB] Lampu Tamu ON");
    server.send(200);
  });

  server.on("/lampu/tamu/off", HTTP_POST, [](){
    *lampuTamu=false;
    mqttClient.publish("smarthome/esp32/lampu/tamu","OFF");
    Serial.println("[WEB] Lampu Tamu OFF");
    server.send(200);
  });

  server.on("/status", [](){
    DynamicJsonDocument doc(256);
    doc["wifi"] = WiFi.status()==WL_CONNECTED?"Connected":"AP Mode";
    doc["ip"]   = WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():WiFi.softAPIP().toString();
    doc["ssid"] = WiFi.status()==WL_CONNECTED?WiFi.SSID():"ESP32-Config";
    doc["kamar"]= *lampuKamar;
    doc["tamu"] = *lampuTamu;
    doc["led"]  = (*lampuKamar || *lampuTamu);
    String res;
    serializeJson(doc,res);
    server.send(200,"application/json",res);
  });

  server.begin();

  xTaskCreatePinnedToCore(taskMQTT,"MQTT",4096,NULL,1,NULL,1);
  xTaskCreatePinnedToCore(taskLED,"LED",2048,NULL,1,NULL,1);
}

/* ================= LOOP ================= */
void loop() {
  server.handleClient();
}

/* ================= WIFI ================= */
void connectWiFi(){
  Serial.println("[WIFI] Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(),password.c_str());
  unsigned long t=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t<15000){
    delay(300);
  }
  if(WiFi.status()==WL_CONNECTED){
    Serial.println("[WIFI] Connected to: "+WiFi.SSID());
    Serial.println("[IP] "+WiFi.localIP().toString());
  } else {
    Serial.println("[WIFI] Failed, switching to AP");
    startAPMode();
  }
}

void startAPMode(){
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Config");
  Serial.println("[AP MODE] ESP32-Config");
  Serial.println("[IP] "+WiFi.softAPIP().toString());
}