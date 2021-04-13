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


WebServer server(80);

uint8_t GeigerPin = 38;
bool GeigerStatus = LOW;
int GeigerVal = 1;
int countsGeiger = 0;
int countsGeigerOld = 0;

void setup() {
  Serial.begin(115200);
  
  pinMode(GeigerPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(GeigerPin), GeigerHit, FALLING);

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
  server.on("/Geigeron", handle_Geigeron);
  server.on("/Geigeroff", handle_Geigeroff);
  server.on("/reload", handle_reload); // <-- test with arguments!
  server.on("/GeigerChange", handle_GeigerChange);
  server.onNotFound(handle_NotFound);
  server.begin();

  Serial.println("HTTP server started");

  //Display IP Address on Heltec OLED
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, 470E6 /**/);
  Heltec.display -> clear();
  Heltec.display -> drawString(0, 0, "Fusor Geiger Counter v.45");
  Heltec.display -> drawString(0, 15, (WiFi.localIP().toString()));
  Heltec.display -> drawString(0, 30, "USB-Serial 115200");
  Heltec.display -> display();
  
}

void loop() {
  server.handleClient();
  
  

  if (GeigerStatus)
  {
    digitalWrite(GeigerPin, HIGH);
  }
  else
  {
    digitalWrite(GeigerPin, LOW);
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


void handle_GeigerChange() {
  String argName = server.argName(0); 
  String argValue = server.arg(0);
  GeigerVal = argValue.toInt();
  // update volts
  Serial.println(argName + " = " + argValue);
  server.send(200, "text/html", sendHtmlPage());
}

void handle_OnConnect() {
  server.send(200, "text/html", sendHtmlPage());
}



void handle_Geigeron() {
  GeigerStatus = HIGH;
  Serial.println("Pin38 Status: ON");
  server.send(200, "text/html", sendHtmlPage());
}

void handle_Geigeroff() {
  GeigerStatus = LOW;
  Serial.println("Pin38 Status: OFF");
  server.send(200, "text/html", sendHtmlPage());
}

void GeigerHit()
{
  countsGeiger++;
  
  if (countsGeiger >= countsGeigerOld + 10)
  {
    Serial.print ("Count = ");
    Serial.println (countsGeiger);
    countsGeigerOld = countsGeiger;
    server.send(200, "text/html", sendHtmlPage());
  }
  else return;
  }

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String sendHtmlPage() {
  return SendHTML(GeigerStatus);
}

String SendHTML(uint8_t Geigerstat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n";
  ptr += "<title>Geiger Counter</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  //ptr += ".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button  {border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer;}\n";
  ptr += ".button-on {background-color: #1abc9c;}\n";
  ptr += ".button-on:active {background-color: #16a085;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: red; margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<h1>Fusor Geiger Counter v.45</h1>\n";
  ptr += "'<form action=/GeigerChange onchange='GeigerVal.value=parseInt(a.value)'><input type='range' id='a' value="+String(GeigerVal)+"><target ='_self'></form>";
  ptr += "<table><tr> Refresh Interval</tr><tr><form action=/GeigerChange><input type='number' id='GeigerVal' name='GeigerVal' value="+String(countsGeiger)+"></tr>";
    
  if (Geigerstat)
  {
    ptr += "<p>Geiger Status: ON</p><a class=\"button button-off\" href=\"/Geigeroff\">OFF</a>\n";
  }
  else
  {
    ptr += "<p>Geiger Status: OFF</p><a class=\"button button-on\" href=\"/Geigeron\">ON</a>\n";
  }

  ptr += "<style>html { width:120px; height:100px; font-family: Helvetica; font-size:extra-large; display: inline-block; margin: 10px auto; text-align: center; table, th, td {border: 1px solid black;border-collapse: collapse;}} </style>\n";
  ptr += "<table><tr><th>Count</th></tr><tr><td>"+String(countsGeiger)+"</td></tr></table>";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
