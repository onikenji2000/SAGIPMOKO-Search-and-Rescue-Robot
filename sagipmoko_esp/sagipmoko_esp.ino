#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "Bogart v2.0"   //SSID
#define APPSK  "123456789" //Password
#endif

const char *ssid = APSSID;
const char *password = APPSK;
const char index_html[] = R"rawliteral(
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
<canvas width="800px" height="500px" id="mainViewPort" ontouchstart="cavstart(event)" ontouchmove="cavmove(event)" ontouchend="cavend(event)" />
<script>
let mV = document.getElementById('mainViewPort');
let obj = null;
let isDragging = null;
let drag = {x : 0, y : 0, xto : 0, xto : 0, id : -1};
let drag2 = {x : 0, y : 0, xto : 0, xto : 0, id : -1};
let temp = "";
let movReq = new XMLHttpRequest();
function repaint() {
let ctx = mV.getContext("2d");
ctx.clearRect(0, 0, mV.width, mV.height);
ctx.fillStyle = "black";
ctx.fillText("Temparature: NULL", 100, 350);
ctx.fillStyle = "blue";
ctx.beginPath();
if(drag.id >= 0) ctx.arc(drag.x, drag.y, 50, 0, 2 * Math.PI);
else ctx.arc(200, 175, 50, 0, 2 * Math.PI);
ctx.fill();
ctx.fillStyle = "white";
if(drag.id >= 0) ctx.fillText("MOVE", drag.x - 25, drag.y - 5);
else ctx.fillText("MOVE", 175, 170);
ctx.fillStyle = "green";
ctx.beginPath();
if(drag2.id >= 0) ctx.arc(drag2.x, drag2.y, 50, 0, 2 * Math.PI);
else ctx.arc(600, 175, 50, 0, 2 * Math.PI);
ctx.fill();
ctx.fillStyle = "white";
if(drag2.id >= 0) ctx.fillText("Camera", drag2.x - 25, drag2.y - 5);
else ctx.fillText("Camera", 575, 170);
}
function cavstart(event) {
let evt = event.targetTouches;
for(let i = 0; i < evt.length; i++) {
let x = evt[i].clientX;
let y = evt[i].clientY;
if(x > 150 && x < 250 && y > 125 && y < 225) {
event.preventDefault();
drag.x = drag.xto = x;
drag.y = drag.yto = y;
drag.id = event.targetTouches[i].identifier;
}
else if(x > 550 && x < 650 && y > 125 && y < 225) {
event.preventDefault();
drag2.x = drag2.xto = x;
drag2.y = drag2.yto = y;
drag2.id = event.targetTouches[i].identifier;
}
}
}
function cavmove(event) {
let evt = event.targetTouches;
for(let i = 0; i < evt.length; i++) {
let x = evt[i].clientX;
let y = evt[i].clientY;
if(drag.id == evt[i].identifier) {
event.preventDefault();
if(x < 150) x = 150;
if(x > 250) x = 250;
if(y < 125) y = 125;
if(y > 225) y = 225;
drag.x = drag.xto = x;
drag.y = drag.yto = y;
console.log(drag);
repaint();
}
if(drag2.id == evt[i].identifier) {
if(x < 550) x = 550;
if(x > 650) x = 650;
if(y < 125) y = 125;
if(y > 225) y = 225;
drag2.x = drag2.xto = x;
drag2.y = drag2.yto = y;
console.log(drag2);
if(drag2.id >= 0) {
if(drag2.x < 560 && drag2.y < 235) command = "/npanptilt";
else if(drag2.x < 560 && drag2.y > 315) command = "/npanntilt";
else if(drag2.x > 640 && drag2.y > 315) command = "/ppanntilt";
else if(drag2.x > 640 && drag2.y < 235) command = "/ppanptilt";
else if(drag2.x < 560) command = "/npan";
else if(drag2.x > 640) command = "/ppan";
else if(drag2.y < 230) command = "/ptilt";
else if(drag2.y > 315) command = "/ntilt";
movReq.open("GET", command);
movReq.send();
}
repaint();
}
}
}
function cavend(event) {
let tl = event.changedTouches;
for(let i = 0; i < tl.length; i++) {
if(tl[i].identifier == drag.id) {
drag.id = -1;
repaint();
}
else if(tl[i].identifier == drag2.id) {
drag2.id = -1;
repaint();
}
}
}
function loop() {
let command = "";
if(drag.id >= 0) {
if(drag.x < 160 && drag.y < 235) command = "/forleft";
else if(drag.x < 160 && drag.y > 315) command = "/backleft";
else if(drag.x > 240 && drag.y > 315) command = "/backright";
else if(drag.x > 240 && drag.y < 235) command = "/foright";
else if(drag.x < 160) command = "/left";
else if(drag.x > 240) command = "/right";
else if(drag.y < 230) command = "/for";
else if(drag.y > 315) command = "/back";
else command = "break";
movReq.open("GET", command);
movReq.send();
}
else {
command = "break";
movReq.open("GET", "/" + command);
movReq.send();
}
}
repaint();
setInterval(loop, 500);
setInterval(camLoop, 700);
</script>
</body>
</html>)rawliteral";
ESP8266WebServer server(80);

int leftmotor = 0, rightmotor = 0;
int pan = 0, tilt = 0;

void handleRoot() {
  
  server.send(200, "text/html", index_html);
}

void sendCommand() {
  Serial.print("left=");
  Serial.println(leftmotor);
  delay(200);
  Serial.print("right=");
  Serial.println(rightmotor);
  delay(200);
}

void sendCamCom() {
  Serial.print("pan=");
  Serial.println(pan);
  Serial.print("tilt=");
  Serial.println(tilt);
}

void forward() {
  leftmotor += 20;
  if(leftmotor > 255) leftmotor = 250;
  rightmotor += 20;
  if(rightmotor > 255) rightmotor = 250;
  if(leftmotor < 0 && rightmotor < 0) leftmotor = rightmotor = 0;
  sendCommand();
  server.send(200, "text/html", "<h1>forward</h1>");
}

void backward() {
  leftmotor -= 20;
  if(leftmotor < -255) leftmotor = -250;
  rightmotor -= 20;
  if(rightmotor < -255) rightmotor = -250;
  if(leftmotor > 0 && rightmotor > 0) leftmotor = rightmotor = 0;
  sendCommand();
  server.send(200, "text/html", "<h1>backward</h1>");
}

void left() {
  leftmotor -= 20;
  if(leftmotor < -255) leftmotor = -250;
  rightmotor += 20;
  if(rightmotor > 255) rightmotor = 250;
  if(leftmotor > 0) leftmotor -= 30;
  sendCommand();
  server.send(200, "text/html", "<h1>left</h1>");
}

void right() {
  leftmotor += 20;
  if(leftmotor > 255) leftmotor = 250;
  rightmotor -= 20;
  if(rightmotor < -255) rightmotor = -250;
  if(rightmotor > 0) rightmotor -= 30;
  sendCommand();
  server.send(200, "text/html", "<h1>right</h1>");
}

void forwardleft() {
  rightmotor += 20;
  if(rightmotor > 255) rightmotor = 250;
  if(leftmotor > 0) leftmotor -= 30;
  if(leftmotor < 0) leftmotor = 0;
  sendCommand();
  server.send(200, "text/html", "<h1>forwardleft</h1>");
}

void forwardright() {
  leftmotor += 20;
  if(leftmotor > 255) leftmotor = 250;
  if(rightmotor < -255) rightmotor = -250;
  if(rightmotor > 0) rightmotor -= 30;
  if(rightmotor < 0) rightmotor = 0;
  sendCommand();
  server.send(200, "text/html", "<h1>forwardright</h1>");
}

void backwardleft() {

}

void backwardright() {

}

void breaks() {
  if(leftmotor < 0) {
    leftmotor += 20;
    if(leftmotor > 0) leftmotor = 0;
  }
  else if(leftmotor > 0) {
    leftmotor -= 20;
    if(leftmotor < 0) leftmotor = 0;
  }
  if(rightmotor < 0) {
    rightmotor += 20;
    if(rightmotor > 0) rightmotor = 0;
  }
  else if(rightmotor > 0) {
    rightmotor -= 20;
    if(rightmotor < 0) rightmotor = 0;
  }
  sendCommand();
  server.send(200, "text/html", "<h1>break</h1>");
}

void npanntilt() {
  if(pan > 0) pan -= 10;
  if(tilt > 0) tilt -= 10;
  sendCamCom();
  server.send(200, "text/html", "<h1>pan left, tilt down");
}

void npanptilt() {
  if(pan > 0) pan -= 10;
  if(tilt < 180) tilt += 10;
  sendCamCom();
  server.send(200, "text/html", "<h1>pan left, tilt up");
}

void ppanptilt() {
  if(pan < 180) pan += 10;
  if(tilt < 180) tilt += 10;
  sendCamCom();
  server.send(200, "text/html", "<h1>pan right, tilt up");
}

void ppanntilt() {
  if(pan < 180) pan += 10;
  if(tilt > 0) tilt -= 10;
  sendCamCom();
  server.send(200, "text/html", "<h1>pan right, tilt down");
}

void npan() {
  if(pan > 0) pan -= 10;
  sendCamCom();
  server.send(200, "text/html", "<h1>pan left</h1>");
}

void ppan() {
  if(pan < 180) pan += 10;
  sendCamCom();
  server.send(200, "text/html", "<h1>pan right</h1>");
}

void ntilt() {
  if(tilt > 0) tilt -= 10;
  sendCamCom();
  server.send(200, "text/html", "<h1>tilt up</h1>");
}

void ptilt() {
  if(tilt < 180) tilt += 10;
  sendCamCom();
  server.send(200, "text/html", "<h1>tilt down</h1>");
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");

  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/for", forward);
  server.on("/back", backward);
  server.on("/left", left);
  server.on("/right", right);
  server.on("/forleft", forwardleft);
  server.on("/foright", forwardright);
  server.on("/backleft", backwardleft);
  server.on("/backright", backwardright);
  server.on("/npanntilt", npanntilt);
  server.on("/npanptilt", npanptilt);
  server.on("/ppanptilt", ppanptilt);
  server.on("/ppanntilt", ppanntilt);
  server.on("/npan", npan);
  server.on("/ppan", ppan);
  server.on("/ntilt", ntilt);
  server.on("/ptilt", ptilt);
  server.on("/break", breaks);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
