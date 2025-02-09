#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>

#ifndef APSSID
#define APSSID "Bogart v2.0"   //SSID
#define APPSK  "123456789" //Password
#endif

const char *ssid = APSSID;
const char *password = APPSK;
const char index_html[] = R"rawliteral(
<!DOCTYPE html>
<html>
    <head>
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            .main-panel {
                background-color: blue;
            }
            .left-track-panel {
                background-color: green;
            }
            .control-panel {
                width: 20vw;
                display: inline-flex;
                border-radius: 10px;
            }
            .right-track-panel {
                background-color: green;
            }
            .camera-knob-panel {
                background-color: red;
            }
            .bottom-hud-panel {
                background-color: yellow;
                border-radius: 5px;
            }
        </style>
    </head>
    <body>
        <canvas width="800px" height="500px" id="mainViewPort" ontouchstart="cavstart(event)" ontouchmove="cavmove(event)" ontouchend="cavend(event)" />
        
        <div class="main-panel">
            </div>
            <div class="control-panel left-track-panel">
                <div class="track-control">
                    <svg width="80" height="150" x="10" y="10">
                        <rect width="100" height="300" style="fill:rgb(0, 0, 255); stroke-width:10; stroke:rgb(0, 100, 0)" />
                    </svg>
                </div>
            </div>
            <div class="control-panel camera-knob-panel">
                C
            </div>
            <div class="control-panel right-track-panel">
                R
            </div>
            <div class="bottom-hud-panel">
                B
            </div>
        </div>
        <script>
            let mV = document.getElementById('mainViewPort');
            let obj = null;
            let isDragging = null;
            let drag = {x : 0, y : 0, xto : 0, xto : 0, id : -1};
            let drag2 = {x : 0, y : 0, xto : 0, xto : 0, id : -1};
            let temp = "NULL";
            let movReq = new XMLHttpRequest();
            let tempReq = new XMLHttpRequest();
            let ledLight = false;
            tempReq.onreadystatechange = function() {
              if(this.readyState == 4 && this.status == 200) {
                temp = tempReq.responseText;
              }
            }
            function repaint() {
                let ctx = mV.getContext("2d");
                ctx.clearRect(0, 0, mV.width, mV.height);
                if(ledLight) ctx.fillStyle = "green";
                else ctx.fillStyle = "red";
                ctx.fillRect(375, 50, 50, 15);
                ctx.fillStyle = "white";
                ctx.fillText("LIGHTS", 377, 60);
                ctx.fillStyle = "black";
                ctx.fillText("Temparature: " + temp, 100, 350);
                ctx.fillText("GPS Position:", 100, 380);
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
                    if(tl[i].clientX >= 350 && tl[i].clientX <= 450 && tl[i].clientY >= 25 && tl[i].clientY <= 75) {
                        ledLight = !ledLight;
                        repaint();
                    }
                }
            }
            function loop() {
                let command = "/move";
                let left, right, pan, tilt, sa = false;
                if(drag2.id >= 0) {
                    if(drag2.x < 560 && drag2.y < 135) {
                        pan = 10;
                        tilt = 10;
                    } //command = "/npanptilt";
                    else if(drag2.x < 560 && drag2.y > 215) {
                        pan = 10;
                        tilt = 1010;
                    } //command = "/npanntilt";
                    else if(drag2.x > 640 && drag2.y > 215) {
                        pan = 1010;
                        tilt = 1010;
                    } //command = "/ppanntilt";
                    else if(drag2.x > 640 && drag2.y < 135) {
                        pan = 1010;
                        tilt = 10;
                    } //command = "/ppanptilt";
                    else if(drag2.x < 560) pan = 10; //command = "/npan";
                    else if(drag2.x > 640) pan = 1010; //command = "/ppan";
                    else if(drag2.y < 135) tilt = 10; //command = "/ptilt";
                    else if(drag2.y > 215) tilt = 1010; //command = "/ntilt";
                }
                if(drag.id >= 0) {
                  if(drag.x < 160 && drag.y < 135) {
                      left = 1075;
                      right = 150;
                  } //command = "/forleft";
                  else if(drag.x < 160 && drag.y > 215) {
                      left = 75;
                      right = 1150;
                  } //command = "/backleft";
                  else if(drag.x > 240 && drag.y > 215) {
                      left = 1150;
                      right = 75;
                  } //command = "/backright";
                  else if(drag.x > 240 && drag.y < 135) {
                      left = 75;
                      right = 1150;
                  } //command = "/foright";
                  else if(drag.x < 160) {
                      left = 1150;
                      right = 150;
                  } //command = "/left";
                  else if(drag.x > 240) {
                      left = 150;
                      right = 1150;
                  } //command = "/right";
                  else if(drag.y < 135) {
                      left = 150;
                      right = 150;
                  } //command = "/for";
                  else if(drag.y > 215) {
                      left = 1150;
                      right = 1150;
                  } //command = "/back";
              }
              if(pan) {
                  let ledStatus = '0';
                  if(ledLight) ledStatus = '1';
                  command = command + '?p=' + pan;
                  command = command + '&lt=' + ledStatus;
                  sa = true;
              }
              if(tilt) {
                  if(sa) command = command + '&t=' + tilt;
                  else command = command + '?t=' + tilt;
                  sa = true;
              }
              if(left) {
                  if(sa) command = command + '&l=' + left;
                  else command = command + '?l=' + left;
                  sa = true;
              }
              if(right) {
                  if(sa) command = command + '&r=' + right;
                  else command = command + '?r=' + right;
              }
              movReq.open('GET', command);
              movReq.send();
              tempReq.open('GET', '/temp');
              tempReq.send();
            }
            repaint();
            setInterval(loop, 500);
        </script>
    </body>
</html>)rawliteral";

AsyncWebServer server(80);

int left = 0, right = 0, tilt = 0, pan = 0;
String temp = "ERROR! NO RESULTS";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setTimeout(100);
  WiFi.softAP(ssid, password);

  Serial.print("Access Point SSID: ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });
  
  server.on("/move", HTTP_GET, [](AsyncWebServerRequest *request) {
    bool a = false, b = false, c = false, d = false, slt = false;
    String show = "";
    int pp, tt;
    if(request->hasParam("l")) {
      String vval = request->getParam("l")->value();
      int val = vval.toInt();
      if(val > 1000) val = (val - 1000) * -1; 
      left += val;
      if(left > 255) left = 250;
      if(left < -255) left = -250;
      a = true;
    }
    if(request->hasParam("r")) {
      String vval = request->getParam("r")->value();
      int val = vval.toInt();
      if(val > 1000) val = (val - 1000) * -1;
      right += val;
      if(right > 255) right = 250;
      if(right < -255) right = -250;
      b = true;
    }
    if(request->hasParam("t")) {
      String vval = request->getParam("t")->value();
      int val = vval.toInt();
      if(val > 1000) val = (val - 1000) * -1;
      tt = val;
      c = true;
    }
    if(request->hasParam("p")) {
      String vval = request->getParam("p")->value();
      int val = vval.toInt();
      if(val > 1000) val = (val - 1000) * -1;
      pp = val;
      d = true;
    }
    if(request->hasParam("lt")) {
      String vval = request->getParam("lt")->value();
      show = show + ";light are " + vval;
      Serial.print("lights=");
      Serial.print(vval);
      slt = true;
    }
    if(a) {
      if(slt) Serial.print(":");
      show = show + ";left motor running;";
      Serial.print("left=");
      Serial.print(left);
      slt = true;
    }
    if(b) {
      show = show + ";right motor running;";
      if(slt) Serial.print(":");
      Serial.print("right=");
      Serial.print(right);
      slt = true;
    }
    if(!a && !b) {
      show = show + ";motor fully halts;";
      Serial.print("left=0:right=0");
      slt = true;
    }
    if(c) {
      show = show + ";tilting;";
      if(slt) Serial.print(":");
      Serial.print("tilt=");
      Serial.print(tt);
      slt = true;
    }
    if(d) {
      show = show + ";panning;";
      if(slt) Serial.print(":");
      Serial.print("pan=");
      Serial.print(pp);
      slt = true;
    }
    Serial.println();
    request->send(200, "text/html", "<h1>" + show + "</h1>");
    delay(200);
  });

  server.on("/temp", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", temp);
  });

  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available() > 0) {
    String message = Serial.readString();
    int index = message.indexOf(':');
    if(index > 0) {
      String com = message.substring(0, index);
      if(com == "Temperature") temp = message.substring(index + 1, message.length());
    }
  }
}
