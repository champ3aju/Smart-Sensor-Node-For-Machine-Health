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
const char* AWS_IOT_ENDPOINT = "a3dnxsu1qgagkq-ats.iot.ap-south-1.amazonaws.com";
const char* AWS_IOT_TOPIC    = "system/params";
const char* CLIENT_ID        = "ESP32_Node";

/* ================= CERTS ================= */
// 🔴 Replace with real certificates
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF
ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6
b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL
MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv
b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj
ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM
9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw
IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6
VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L
93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm
jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC
AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA
A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI
U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs
N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv
o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU
5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy
rqXRfboQnoZsG4q5WTP468SQvvG5
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDWTCCAkGgAwIBAgIUXmWX5xDbNLS0EOgNJfGUXkkTjawwDQYJKoZIhvcNAQEL
BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g
SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTI2MDEyNTExNTI1
MloXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0
ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAPNbvvgbsBYaduFPGFrQ
GcnDkoNj9KBbJgDz1RZwYLyXfaHz4KrqOY3XGlUXRNHUEQft44ezZ2M16RF+ASON
jCgFPxSmX8ZdcSYBnAVXtw5O5FECFLT59TOyTMjkNyzkQNeDEOPXaI1TFa6LUY1G
aC/35Y+AOfVAp99vdhhx4AFMIdcjQ6E4p+yjhZiJt/6bHVz4wiVvsVcWe9GWFmIF
7Xvdlknwg135QfmrRJNe2bgnzBHJ9UalqzEWDMFrTDUKumkETzPxv9d0r7dEJOIP
Jn+kbwrjEJLL43FOTWVds814EARNR7BXYO/Z/+w1jTpKKbrBSRiOabNrtBKvciWk
ojUCAwEAAaNgMF4wHwYDVR0jBBgwFoAUppu4wP2bXeqxGFkjqcAjJXO4yS4wHQYD
VR0OBBYEFCaw1DkTQUl5AcSQIGbMuoHjzMHYMAwGA1UdEwEB/wQCMAAwDgYDVR0P
AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQA9TfbIiILhWH4YpJiWPCueiSsA
pFETaNebGMRtxXvYRksE5II7+1RtG3gGSyJNRSYdXC1J/FfpdkApn1TuefxGsPM9
8JlQA4DnacBwMDl1T3kCHMYras+Zpr6NyPK0bR2sB6d+psR/OoYOM6XED+88F+9T
j41SbIJnq1RTrvc8SmgxzBq7q18UgrrC3fAs54twUUqLgoj7nGOQGebfpL6oRRSJ
/h7zcUC/plWcVCV4AdAXP7PQV4u8eir453d01v5pNXl+Y1VicCkaAB3ux6zrniFa
9CFJ7v07xv5DKBX0JRQTuuIE6tcJ50xsvdGXfFKnsV4Z8ryrn0byuWJ66F9m
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
MIIEpAIBAAKCAQEA81u++BuwFhp24U8YWtAZycOSg2P0oFsmAPPVFnBgvJd9ofPg
quo5jdcaVRdE0dQRB+3jh7NnYzXpEX4BI42MKAU/FKZfxl1xJgGcBVe3Dk7kUQIU
tPn1M7JMyOQ3LORA14MQ49dojVMVrotRjUZoL/flj4A59UCn3292GHHgAUwh1yND
oTin7KOFmIm3/psdXPjCJW+xVxZ70ZYWYgXte92WSfCDXflB+atEk17ZuCfMEcn1
RqWrMRYMwWtMNQq6aQRPM/G/13Svt0Qk4g8mf6RvCuMQksvjcU5NZV2zzXgQBE1H
sFdg79n/7DWNOkopusFJGI5ps2u0Eq9yJaSiNQIDAQABAoIBAQDCzpiqqA9nQjwT
1GsclyVtm/726tbyXsSnUokrcz4c+e/56u3YI7N61cwgfYX7qhUQEeBnyrdidc9i
z0k+GpstljWO5Dga5MYktHke1b6V15VxO4cz8DvW7E4HKpj4eKdnXWXx3Ye2SeI7
XQ8lVk0kyP6cxvqeVumLNQCGRPIaVjXLYh1kOpb6lnmi1HFAUFHFa9Bt46eTJxME
yiYQNfUDGerA/W8vOPVIj6o1WDkC9N6u4q68JHmQ/IPT3xAD6s7V6TY6OLry0U3Q
urzdK9K4l1oYjteCiVBCtmXosLXWeZI5I3hGCCS8bpj89Wn6b4ATWvc7mCCaE/0F
VZtoi1oBAoGBAPqNx2C6Gyb853n894EkE/2k6JjdCI73MP7LRQnVHvxAmZ0gvQsD
YyEKvwprAsAnRbQ9X2/yDzhoKaIW/+ytOZYg7ACMF/TaRSWVkgEoKjoWbFAl2qwY
0WWiJbN3yx24A+Y9X/4o0NaiQnDa48IGsZYzi2ntw3LAxP6iSWqIL23BAoGBAPil
7X5UfUvl3FQYMGobRiokspGkWx992tIKNHWea5Ozms3EGh++oTuAN3z+w03j7Ud3
f+BXzWo8+4Az8fdaevZPpgYCVe1nLAroR0GPq+jJyd/oWy54grM7dzhoBXZNH+CR
aFjQq6lTV/WaQJfRnRynaVmWP1hWG1koNHK5Z7l1AoGBAKTIZh5Jh94Gar6iDq2S
astgj9EpXZ2ZGVIZodgroGtKmTzr8SizG5j5lldltbNmoMM/serQCg6RAwClm7+u
1bbVBCIAASl5/AwR6Kp2Hf+CC85SdxTEZcMv9xena6CqC3Z7WwyvD1p58LSEYVKU
UGt2jvQl+WsZOPY3CY3U70yBAoGADq7NrW4T0A2MyzJ6eWmO5SIxoUQRVXS7HTgZ
DPED7T7n30q7ODCrmLXjkPACXHEfK980n+zPZnyfZVyr5y1ioXTmmug8Zr527VX2
DzsKrb5su1ofOElJfof+bEhIOkonnb04ajY4TNFtGkW3lr98BZslL3Md3nuHNeaX
4kZsh5kCgYAJBSNEUaaLTQdyMlT/eRlus9mrCq9WrGR5ZNUvogNVfk6LttMUMMuZ
Fhi4fvj63QklNoJ52tsrZENClk1ou3KHVJuGaeYA4L6E56nuyece7qtX6aTh9DJY
9Uyu/HvsGBNKpmMK+WggGX3N58B9FxBUnt8iJeCImmR70SZb95fAjw==
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
