#include <WiFi.h>
#include <WebServer.h>
#include <heltec.h>

/* Put your SSID & Password */
//const char* ssid = "DMS Member";
//const char* password = "dms--10923";

//const char* ssid = "TL35207 7831";
//const char* password = "p-13C916";

const char* ssid = "MySpectrumWiFi95-2G";
const char* password = "acreinput627";

int PWMval = 1;
int PEMval = 1;
int TEMPval = 1;

WebServer server(80);

uint8_t LED1pin = 26;
bool LED1status = LOW;

uint8_t LED2pin = 27;
bool LED2status = LOW;

uint8_t PEMpin = 38;
bool PEMstatus = LOW;

uint8_t TEMPpin = 36;
bool TEMPstatus = LOW;

void setup() {
  Serial.begin(115200);

  pinMode(LED1pin, OUTPUT);
  pinMode(LED2pin, OUTPUT);
  pinMode(PEMpin, OUTPUT);
  pinMode(TEMPpin, OUTPUT);

  // Connect to Wi-Fi network with SSID and password
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(100);

    server.on("/", handle_OnConnect);
  server.on("/led1on", handle_led1on);
  server.on("/led1off", handle_led1off);
  server.on("/led2on", handle_led2on);
  server.on("/led2off", handle_led2off);
  server.on("/PEMon", handle_PEMon);
  server.on("/PEMoff", handle_PEMoff);
  server.on("/TEMPon", handle_TEMPon);
  server.on("/TEMPoff", handle_TEMPoff);
  server.on("/reload", handle_reload); // <-- test with arguments!
  server.on("/PWMChange", handle_PWMChange);
  server.on("/PEMChange", handle_PEMChange);
  server.on("/TEMPChange", handle_TEMPChange);
  server.onNotFound(handle_NotFound);
  server.begin();

  Serial.println("HTTP server started");

  //Display IP Address on Heltec OLED
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, 470E6 /**/);
  Heltec.display -> clear();
  Heltec.display -> drawString(0, 0, "Fusor PEM Gas Dryer 0.47");
  Heltec.display -> drawString(0, 15, (WiFi.localIP().toString()));
  Heltec.display -> drawString(0, 30, "USB-Serial 115200");
  Heltec.display -> display();
  
}

void loop() {
  server.handleClient();
  if (LED1status)
  {
    digitalWrite(LED1pin, HIGH);
  }
  else
  {
    digitalWrite(LED1pin, LOW);
  }

  if (LED2status)
  {
    digitalWrite(LED2pin, HIGH);
  }
  else
  {
    digitalWrite(LED2pin, LOW);
  }

  if (PEMstatus)
  {
    digitalWrite(PEMpin, HIGH);
  }
  else
  {
    digitalWrite(PEMpin, LOW);
  }

  if (TEMPstatus)
  {
    digitalWrite(TEMPpin, HIGH);
  }
  else
  {
    digitalWrite(TEMPpin, LOW);
  }
}

void handle_reload() {
  for (int i = 0; i < server.args(); i++) {
    String argName = server.argName(i);
    String argValue = server.arg(i);
    Serial.println(i + "  " + argName + "  " + argValue);
  }
  server.send(200, "text/html", "<html><h5>RELOAD</h5></html>");
}

void handle_PWMChange() {
  String argName = server.argName(0); // If you need it
  String argValue = server.arg(0);
  Serial.println(argName + "  " + argValue);
  server.send(200, "text/html", SendHTML(LED1status, LED2status, PEMstatus, TEMPstatus));
}

void handle_PEMChange() {
  String argName = server.argName(0); // If you need it
  String argValue = server.arg(0);
  Serial.println(argName + "  " + argValue);
  server.send(200, "text/html", SendHTML(LED1status, LED2status, PEMstatus, TEMPstatus));
}

void handle_TEMPChange() {
  String argName = server.argName(0); // If you need it
  String argValue = server.arg(0);
  Serial.println(argName + "  " + argValue);
  server.send(200, "text/html", SendHTML(LED1status, LED2status, PEMstatus, TEMPstatus));
}

void handle_OnConnect() {
  LED1status = LOW;
  LED2status = LOW;
  PEMstatus = LOW;
  TEMPstatus = LOW;
  Serial.println("GPIO7 Status: OFF | GPIO6 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status, LED2status, PEMstatus, TEMPstatus));
}

void handle_led1on() {
  LED1status = HIGH;
  Serial.println("Pin26 Status: ON");
  server.send(200, "text/html", SendHTML(true, LED2status, PEMstatus, TEMPstatus));
}

void handle_led1off() {
  LED1status = LOW;
  Serial.println("Pin26 Status: OFF");
  server.send(200, "text/html", SendHTML(false, LED2status, PEMstatus, TEMPstatus));
}

void handle_led2on() {
  LED2status = HIGH;
  Serial.println("Pin27 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status, true, PEMstatus, TEMPstatus));
}

void handle_led2off() {
  LED2status = LOW;
  Serial.println("Pin27 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status, false, PEMstatus, TEMPstatus));
}

void handle_PEMon() {
  PEMstatus = HIGH;
  Serial.println("Pin38 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status, LED2status, true, TEMPstatus));
}

void handle_PEMoff() {
  PEMstatus = LOW;
  Serial.println("Pin38 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status, LED2status, false, TEMPstatus));
}

void handle_TEMPon() {
  TEMPstatus = HIGH;
  Serial.println("Pin36 Status: ON");
  server.send(200, "text/html", SendHTML(LED1status, LED2status, PEMstatus, true));
}

void handle_TEMPoff() {
  TEMPstatus = LOW;
  Serial.println("Pin36 Status: OFF");
  server.send(200, "text/html", SendHTML(LED1status, LED2status, PEMstatus, false));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t led1stat, uint8_t led2stat, uint8_t PEMstat, uint8_t TEMPstat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n";
  ptr += "<title>Gas Dryer Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  //ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button  {border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer;}\n";
  ptr += ".button-on {background-color: #1abc9c;}\n";
  ptr += ".button-on:active {background-color: #16a085;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Fusor PEM Electrolyzer Gas Dryer v0.47</h1>\n";
  

  if (led1stat)
  {
    ptr += "<p>PeltierStack Status: ON</p><a class=\"button button-off\" href=\"/led1off\">OFF</a>";
  }
  else
  {
    ptr += "<p>PeltierStack Status: OFF</p><a class=\"button button-on\" href=\"/led1on\">ON</a>";
  }

  if (PEMstat)
  {
    ptr += "<p>PEM Status: ON</p><a class=\"button button-off\" href=\"/PEMoff\">OFF</a>\n";
  }
  else
  {
    ptr += "<p>PEM Status: OFF</p><a class=\"button button-on\" href=\"/PEMon\">ON</a>\n";
  }

  ptr += "'<form action=/PEMChange onchange='PEMval.value=parseInt(a.value)'><input type='range' id='a' value=a><target ='_self'></form>";
  ptr += "<table><tr> PEM level </tr><tr><form action=/PEMChange><input type='number' id='PEMval' name='PEMval' value=PEMval><input type=submit><output name='PEMval'></output></form></tr></table><br>";

  

  if (led2stat)
  {
    ptr += "<p>PeltierBase Status: ON</p><a class=\"button button-off\" href=\"/led2off\">OFF</a>";
  }
  else
  {
    ptr += "<p>PeltierBase Status: OFF</p><a class=\"button button-on\" href=\"/led2on\">ON</a>";
  }

  ptr += "'<form action=/PWMChange onchange='PWMval.value=parseInt(a.value)'><input type='range' id='a' value=a><target ='_self'></form>";
  ptr += "<table><tr> PWM level </tr><tr><form action=/PWMChange><input type='number' id='PWMval' name='PWMval' value=PWMval><input type=submit></form></tr></table><br>";

  
  if (TEMPstat)
  {
    ptr += "<p>TEMP Status: ON</p><a class=\"button button-off\" href=\"/TEMPoff\">OFF</a>\n";
  }
  else
  {
    ptr += "<p>TEMP Status: OFF</p><a class=\"button button-on\" href=\"/TEMPon\">ON</a>\n";
  }

  ptr += "'<form action=/TEMPChange onchange='TEMPval.value=parseInt(a.value)'><input type='range' id='a' value=a><target ='_self'></form>";
  //ptr += "'<form><input type="number" id="mynumber"></form><script> var  (TEMPval)=a.value; document.getElementById("mynumber").value=(PWMval); </script>";
  
  //<form><input type="number" id="mytext"></form><script>var test = 1234; var PWMval = test; document.getElementById("mytext").value = PWMval;</script>
  ptr += "<table><tr> TEMP </tr><tr><form action=/TEMPChange><input type='number' id='TEMPval' name='TEMPval' value=TEMPval><input type=submit><target='_self'></form></tr></table><br>";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
