
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define NUM_LEDS 1

Adafruit_NeoPixel led(
  NUM_LEDS,
  LED_PIN,
  NEO_GRB + NEO_KHZ800
);

const char* ssid = "RGB-TEST";
const char* password = "12345678";

WebServer server(80);

bool robotRunning = false;

void setRobotState(bool running) {
  robotRunning = running;

  if (running) {
    led.setPixelColor(0, led.Color(0, 255, 0));
    Serial.println("ROBOT OPERATIONAL - GREEN LED");
  } else {
    led.setPixelColor(0, led.Color(255, 0, 0));
    Serial.println("SAFETY HALT - RED LED");
  }

  led.show();
}

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">

<title>ESP32 Robot Control</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  background: #0f172a;
  color: white;
  font-family: Arial, sans-serif;
  text-align: center;
  padding: 25px 12px;
}

.card {
  max-width: 450px;
  margin: 40px auto;
  padding: 30px 20px;
  background: #1e293b;
  border-radius: 20px;
  box-shadow: 0 10px 30px #0005;
}

h1 {
  font-size: 27px;
  margin-bottom: 10px;
}

.subtitle {
  color: #94a3b8;
  font-size: 15px;
}

.student {
  background: #334155;
  padding: 15px;
  border-radius: 10px;
  margin-top: 20px;
  font-size: 18px;
  font-weight: bold;
  color: #93c5fd;
}

.status {
  margin: 30px 0;
  font-size: 22px;
  font-weight: bold;
}

.green {
  color: #4ade80;
}

.red {
  color: #f87171;
}

button {
  display: block;
  width: 100%;
  border: none;
  padding: 18px;
  margin: 12px 0;
  border-radius: 10px;
  font-size: 18px;
  font-weight: bold;
  color: white;
  cursor: pointer;
}

.start {
  background: #16a34a;
}

.stop {
  background: #dc2626;
}

.start:active {
  background: #15803d;
}

.stop:active {
  background: #b91c1c;
}

#message {
  color: #cbd5e1;
  font-size: 14px;
}

.footer {
  margin-top: 25px;
  font-size: 12px;
  color: #94a3b8;
}
</style>
</head>

<body>

<div class="card">

<h1>ESP32 Robot Control</h1>

<p class="subtitle">
Wireless Robot Control Dashboard
</p>

<div class="student">
Muhammad
</div>

<div id="status" class="status">
Loading...
</div>

<button class="start" onclick="control(1)">
START ROBOT
</button>

<button class="stop" onclick="control(0)">
SAFETY HALT
</button>

<p id="message"></p>

<div class="footer">
ESP32-S3 Local Web Server | Port 80
</div>

</div>

<script>
async function refreshStatus() {
  try {
    const response = await fetch('/robot/status');

    if (!response.ok) {
      throw new Error('Server Error');
    }

    const data = await response.json();
    const status = document.getElementById('status');

    if (data.running) {
      status.textContent = 'OPERATIONAL - GREEN LED';
      status.className = 'status green';
    } else {
      status.textContent = 'SAFETY HALT - RED LED';
      status.className = 'status red';
    }

  } catch (error) {
    document.getElementById('message').textContent =
      'Network Error';
  }
}

async function control(state) {
  try {
    const response = await fetch(
      '/robot/status?state=' + state
    );

    if (!response.ok) {
      throw new Error('Command Failed');
    }

    document.getElementById('message').textContent =
      'Command Successful';

    await refreshStatus();

  } catch (error) {
    document.getElementById('message').textContent =
      'Network Error';
  }
}

refreshStatus();
setInterval(refreshStatus, 2000);
</script>

</body>
</html>
)rawliteral";

void handleRobotStatus() {
  if (server.hasArg("state")) {
    String state = server.arg("state");

    if (state == "1") {
      setRobotState(true);
    } else if (state == "0") {
      setRobotState(false);
    } else {
      server.send(
        400,
        "application/json",
        "{\"error\":\"Invalid state\"}"
      );
      return;
    }
  }

  String response = "{\"running\":";

  response += robotRunning ? "true" : "false";
  response += "}";

  server.send(
    200,
    "application/json",
    response
  );
}

void setup() {
  Serial.begin(115200);

  led.begin();
  led.setBrightness(50);

  setRobotState(false);

  WiFi.mode(WIFI_AP);

  bool started = WiFi.softAP(
    ssid,
    password,
    1,
    0,
    4
  );

  if (!started) {
    Serial.println("WiFi Access Point Failed");
    return;
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 ROBOT CONTROL");
  Serial.println("Name: Muhammad");
  Serial.println("================================");

  Serial.println("WiFi Access Point Started");

  Serial.print("Network Name: ");
  Serial.println(ssid);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on(
    "/",
    HTTP_GET,
    []() {
      server.send_P(
        200,
        "text/html",
        webpage
      );
    }
  );

  server.on(
    "/robot/status",
    HTTP_GET,
    handleRobotStatus
  );

  server.onNotFound([]() {
    server.send(
      404,
      "text/plain",
      "Page Not Found"
    );
  });

  server.begin();

  Serial.println("HTTP SERVER STARTED ON PORT 80");
  Serial.println("Open: http://192.168.4.1");
}

void loop() {
  server.handleClient();
}
