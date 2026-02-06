#include <WiFi.h>
#include "time.h"
#include <U8g2lib.h>
#include <Wire.h>

#include <WiFiClientSecure.h>
#include <PubSubClient.h>

/* ================= PINS ================= */
#define I2C_SDA 21
#define I2C_SCL 22

#define UART_RX 3
#define UART_TX 1

#define HEARTBEAT_LED 2

/* ================= WIFI ================= */
const char* ssid     = "Nav";
const char* password = "123456789";

/* ================= NTP ================== */
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;

/* ================= AWS ================== */
const char* AWS_IOT_ENDPOINT = "a8dnxsu1qgagkq-ats.iot.ap-south-1.amazonaws.com";
const char* AWS_IOT_TOPIC    = "system/params";
const char* CLIENT_ID        = "ESP32_Node";

/* ================= CERTS ================= */
// 🔴 Replace with real certificates
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
paste here
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
paste here
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
paste here
-----END RSA PRIVATE KEY-----
)EOF";

/* ================= OBJECTS =============== */
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(
  U8G2_R0, U8X8_PIN_NONE, I2C_SCL, I2C_SDA
);

WiFiClientSecure net;
PubSubClient client(net);

/* ================= DATA ================== */
float irms = 0.0f;
float vib  = 0.0f;
uint8_t anomalyStatus = 0;

String uartBuf = "";

/* ================= TIMERS ================ */
unsigned long lastToggle   = 0;
unsigned long lastAWSsend  = 0;
unsigned long lastOLED     = 0;

bool showMachineFrame = true;
bool frameChanged     = true;

/* ================================================= */
void setup() {
  Serial.begin(115200);
  pinMode(HEARTBEAT_LED, OUTPUT);

  /* ---- I2C + OLED ---- */
  Wire.begin(I2C_SDA, I2C_SCL, 100000);
  display.begin();
  display.setFont(u8g2_font_ncenB08_tr);
  display.clearBuffer();
  display.drawStr(0, 14, "System Booting...");
  display.sendBuffer();

  /* ---- UART ---- */
  Serial2.begin(115200, SERIAL_8N1, UART_RX, UART_TX);

  /* ---- WiFi ---- */
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print("...");
    delay(500);
  } 

  /* ---- NTP ---- */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) delay(500);

  /* ---- AWS ---- */
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  client.setServer(AWS_IOT_ENDPOINT, 8883);
  while (!client.connect(CLIENT_ID)) delay(500);

  Serial.println("System Ready");
}

/* ================================================= */
void loop() {
  heartbeatLED();
  readUART();
  toggleFrame();
  updateOLED();
  sendAWSData();

  if (!client.connected()) {
    client.connect(CLIENT_ID);
  }
  client.loop();
}

/* ================= UART ================= */
void readUART() {
  while (Serial2.available()) {
    char c = Serial2.read();

    if (c == '$') uartBuf = "";
    uartBuf += c;

    if (c == '#') {
      parsePacket(uartBuf);
      uartBuf = "";
      frameChanged = true;
    }
  }
}

void parsePacket(String pkt) {
  int iPos = pkt.indexOf("I:");
  int vPos = pkt.indexOf("V:");
  int aPos = pkt.indexOf("A:");

  if (iPos < 0 || vPos < 0 || aPos < 0) return;

  irms = pkt.substring(iPos + 2, pkt.indexOf(",", iPos)).toFloat();
  vib  = pkt.substring(vPos + 2, pkt.indexOf(",", vPos)).toFloat();
  anomalyStatus = pkt.substring(aPos + 2, pkt.indexOf("#", aPos)).toInt();
}

/* ================= OLED ================= */
void toggleFrame() {
  if (millis() - lastToggle >= 5000) {
    showMachineFrame = !showMachineFrame;
    lastToggle = millis();
    frameChanged = true;
  }
}

void updateOLED() {
  if (!frameChanged || millis() - lastOLED < 500) return;
  lastOLED = millis();

  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawFrame(0, 0, 128, 64);

  struct tm timeinfo;
  getLocalTime(&timeinfo);

  // Line 1: WiFi + AWS
  display.drawStr(4, 12,
    (WiFi.status() == WL_CONNECTED) ?
      (client.connected() ? "WiFi:OK  AWS:OK" : "WiFi:OK  AWS:--")
      : "WiFi:--  AWS:--"
  );

  // Line 2: Machine status
  display.drawStr(4, 24,
    anomalyStatus ? "STAT: ANOMALY" : "STAT: NORMAL"
  );

  // Line 3: Current
  char buf[32];
  sprintf(buf, "I: %.3f A", irms);
  display.drawStr(4, 36, buf);

  // Line 4: Vibration
  sprintf(buf, "V: %.3f", vib);
  display.drawStr(4, 48, buf);

  // Line 5: Date + Time
  char timeBuf[24];
  strftime(timeBuf, sizeof(timeBuf), "%d-%m %H:%M", &timeinfo);
  display.drawStr(4, 60, timeBuf);

  display.sendBuffer();
  frameChanged = false;
}

/* ================= AWS ================= */
void sendAWSData() {
  if (millis() - lastAWSsend < 5000) return;
  lastAWSsend = millis();

  char payload[256];
  snprintf(payload, sizeof(payload),
    "{"
    "\"device\":\"%s\","
    "\"current\":%.3f,"
    "\"vibration\":%.3f,"
    "\"anomaly\":%d,"
    "\"timestamp\":%lu"
    "}",
    CLIENT_ID, irms, vib, anomalyStatus, millis()
  );

  client.publish(AWS_IOT_TOPIC, payload);
}

/* ================= LED ================= */
void heartbeatLED() {
  static unsigned long last = 0;
  static bool state = false;

  if (millis() - last >= 500) {
    state = !state;
    digitalWrite(HEARTBEAT_LED, state);
    last = millis();
  }
}
