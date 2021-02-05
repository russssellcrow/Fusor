/*********
  Russell Crow
  Fusor Data Logger
*********/

// Load Wi-Fi library
#include <WiFi.h>
#include <heltec.h>
#include <ESP32Servo.h>

// Auxiliar variables to store the current output state
String DAC26mfcV = "off";
String output27State = "off";
String ADCinput36State = "off";
String ADCinput38State = "off";
// Assign output variables to GPIO pins
const int DACoutMFC = 26;
const int output27 = 27;
const int ADCinput0 = 36;
const int ADCinput2 = 38; 

// Replace with your network credentials
const char* ssid     = "DMS Member";
const char* password = "dms--109238";
// Set web server port number to 80
WiFiServer server(80);

const int dataMaxIndex = 1000;
unsigned long statTime[dataMaxIndex];  // timestamps
int statData01[dataMaxIndex]; // scintillator
//int statData02[dataMaxIndex]; // photospectro
int statData03[dataMaxIndex]; // geiger counter
//int statData04[dataMaxIndex]; // other
int data01Pin = 13;           // scintillator
//int data02Pin;                // photospectro
int data03Pin = 12;           // geiger
//int data04Pin;                // other
int dataIndex = 0;            // index for stat data. changes to zero after client gets data.

// Russell Crow this is your controlls
double dacMic     = 0;
boolean mfcStatus = 0;


void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(DACoutMFC, OUTPUT);
  pinMode(output27, OUTPUT);
  
  // Set outputs to LOW
  digitalWrite(DACoutMFC, LOW);
  digitalWrite(output27, LOW);

  //Display setup Heltec OLED
  //Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, 470E6 /**/);


  /*logo();
  delay(1000);
  Heltec.display->clear();*/

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
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
  server.begin();

  //Display IP Address on Heltec OLED
  Heltec.display -> clear();
  Heltec.display -> drawString(0, 0, "Fusor DataLoggerMFC v0.6");
  Heltec.display -> drawString(0, 15, (WiFi.localIP().toString()));
  Heltec.display -> drawString(0, 30, "USB-Serial 115200");
  Heltec.display -> display();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    String header = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
//        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            if (isCollectData(header)) {
              client.println("Content-type:application/json");
            } else {
              client.println("Content-type:text/html"); 
            }
            client.println("Connection: close");
            client.println();
            
            // - make response - 
            client.println(makeResponse(header));
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;                  
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      } else {
        // no user request
        // collect interesting data 
        collectData();
        // TODO: sleep for 10 millis
      }
    }
    
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

bool isCollectData(String cmd) {
  return cmd.indexOf("GET /collect-data") >= 0;
}

bool isSetDACvolts(String cmd) {
  return cmd.indexOf("GET /setDVolts/?DACvolts=") >= 0;
}

String makeResponse(String cmd) {
    // read json data
    if (isCollectData(cmd)) {
      return readData(true);
    }
    // set DACvolts 
    else if (isSetDACvolts(cmd)) {
      String rawValue = cmd.substring(25,27);
      int volts = rawValue.toInt() * 255 / 100;
      Serial.println("DAC-Volts-Raw = " + rawValue);
      Serial.println("DAC-Volts-Set = " + volts);
      //digitalWrite(DACoutMFC, HIGH);
      analogWrite(DACoutMFC, volts);
    }
    // on off controlls
    else if (cmd.indexOf("GET /26/on") >= 0) {
      Serial.println("DAC-MFC on");
      DAC26mfcV = "on";
      digitalWrite(DACoutMFC, LOW);
    } else if (cmd.indexOf("GET /26/off") >= 0) {
      Serial.println("DAC-MFC off");
      DAC26mfcV = "off";
      digitalWrite(DACoutMFC, LOW);
    } else if (cmd.indexOf("GET /27/on") >= 0) {
      Serial.println("GPIO 27 on");
      output27State = "on";
      digitalWrite(output27, HIGH);
    } else if (cmd.indexOf("GET /27/off") >= 0) {
      Serial.println("GPIO 27 off");
      output27State = "off";
      digitalWrite(output27, LOW);
    } else if (cmd.indexOf("GET /36/on") >= 0) {
      Serial.println("ADCinput0 off");
      ADCinput36State = "on";
      //call Function ADCinput0 analogRead() and analyze
    } else if (cmd.indexOf("GET /36/off") >= 0) {
      Serial.println("ADCinput0 off");
      ADCinput36State = "off";
    } else if (cmd.indexOf("GET /38/on") >= 0) {
      Serial.println("ADCinput4 off");
      ADCinput38State = "on";
      //call Function ADCinput0 analogRead() and analyze
    } else if (cmd.indexOf("GET /38/off") >= 0) {
      Serial.println("ADCinput4 off");
      ADCinput38State = "off";
    } else {
      Serial.println("Undef = "+cmd);
      return showWebPage(cmd);
    }
}

String showWebPage(String cmd) {
  // Display the HTML web page
  String html = "";
    html += "<html>";
    html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<link rel=\"icon\" href=\"data:,\">";
    // CSS to style the on/off buttons 
    // Feel free to change the background-color and font-size attributes to fit your preferences
    html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
    html += ".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;";
    html += "text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}";
    html += ".button2 {background-color: #FF0000;}</style></head>";  
    // Scripts
    html += "<script> function openWindowReload(link) {";
    html += "var href = link.href; window.open(href,'_blank'); document.location.reload(true)}"; 
    html += "</script>";
    
    // Web Page Heading
    html += "<body><h1>Fusor DataLogger MFC v-0.6</h1>";
    html += "<p>" + (WiFi.localIP().toString()) + " and USB-Serial 115200 </p>";

    // Display current state, and ON/OFF buttons for DAC-MFC  
    html += "<p>DAC-MFC - State " + DAC26mfcV + "</p>";
    // If the DAC26mfcV is off, it displays the OFF button       
    if (DAC26mfcV=="off") {
      html += "<p><a href=\"/26/on\" target=\"_blank\" onClick=\"openWindowReload(this)\"><button class=\"button\">OFF</button></a></p>";
    } else {
      html += "<p><a href=\"/26/off\" target=\"_blank\" onClick=\"openWindowReload(this)\"><button class=\"button button2\">ON</button></a></p>";
      // DACvolts control 
      html += "<form action=\"/setDVolts/\" method=\"get\">";
      html += "<input type=\"number\" name=\"DACvolts\" value=\"10\" min=\"10\" max=\"99\"><input type=\"submit\" value=\"Submit\">";
      html += "</form>";
    }
       
    // Display current state, and ON/OFF buttons for GPIO 27  
    html += "<p>GPIO 27 - State " + output27State + "</p>";
    // If the output27State is off, it displays the OFF button       
    if (output27State=="off") {
      html += "<p><a href=\"/27/on\" target=\"_blank\" onClick=\"openWindowReload(this)\"><button class=\"button\">OFF</button></a></p>";
    } else {
      html += "<p><a href=\"/27/off\" target=\"_blank\" onClick=\"openWindowReload(this)\"><button class=\"button button2\">ON</button></a></p>";
    }
    html += "</body></html>";

    // Display current state, and ON/OFF buttons for ADC 0  
    html += "<p>ADC 0 - State " + ADCinput36State + "</p>";
    // If the output27State is off, it displays the OFF button       
    if (ADCinput36State=="off") {
      html += "<p><a href=\"/36/on\" target=\"_blank\" onClick=\"openWindowReload(this)\"><button class=\"button\">OFF</button></a></p>";
    } else {
      html += "<p><a href=\"/36/off\" target=\"_blank\" onClick=\"openWindowReload(this)\"><button class=\"button button2\">ON</button></a></p>";
    }

    // Display current state, and ON/OFF buttons for ADC 0  
    html += "<p>ADC 4 - State " + ADCinput38State + "</p>";
    // If the output27State is off, it displays the OFF button       
    if (ADCinput38State=="off") {
      html += "<p><a href=\"/38/on\" target=\"_blank\" onClick=\"openWindowReload(this)\"><button class=\"button\">OFF</button></a></p>";
    } else {
      html += "<p><a href=\"/38/off\" target=\"_blank\" onClick=\"openWindowReload(this)\"><button class=\"button button2\">ON</button></a></p>";
    }

    html += "</body></html>";
    
    // The HTTP response ends with another blank line
    return html;
}

//*****FUNCTIONS*****//
String readData(boolean cleanData) {
  // create json from available data
  String response = "{";
  
  if (dataIndex > 0) {
    String exportTime = "[";
    String export01 = "[";
    //String export02 = "[";
    String export03 = "[";
    //String export04 = "[";
    for (int i=0;i<dataIndex;i++) {
      exportTime += String(statTime[i]);
      export01 += String(statData01[i]);
      //export02 += String(statData02[i]);
      export03 += String(statData03[i]);
      //export04 += String(statData04[i]);
      // prevent last invalid comma bug
      if (i != dataIndex-1) {
        exportTime += ", ";
        export01 += ", ";
        //export02 += ", ";
        export03 += ", ";
        //export04 += ", ";
      }
    }
    // close data array
    exportTime += "]";
    export01 += "]";
    //export02 += "]";
    export03 += "]";
    //export04 += "]";
    // export to json
    response += "\"statTime: \"" + exportTime +", \n";
    response += "\"stat01: \"" + export01 +", \n";
    //response += "\"stat02: \"" + export02 +", \n";
    response += "\"stat03: \"" + export03 +", \n";
    //response += "\"stat04: \"" + export04 +" \n";
  }

  response += "}";
  if (cleanData) {
    dataIndex = 0;
  }
  return response;
}



void collectData() {
  statTime[dataIndex] = micros();
  statData01[dataIndex] = analogRead(data01Pin);
  //statData02[dataIndex] = analogRead(data02Pin);
  statData03[dataIndex] = analogRead(data03Pin);
  //statData04[dataIndex] = analogRead(data04Pin);
  dataIndex++;
  if (dataIndex >= dataMaxIndex) {
    dataIndex = 0;
  }
}


/*
int ADC0_get(int Volts)
{
  int ADC0_Volts;
  analogRead (ADC0_Volts);
  return Volts;
}

int ADC4_get(int Volts)
{
  int ADC4_Volts;
  analogRead (ADC4_Volts);
  return Volts;
}  
*/
 
