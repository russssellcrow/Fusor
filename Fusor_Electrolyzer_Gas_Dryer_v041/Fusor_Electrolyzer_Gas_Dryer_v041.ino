/*********
  Russell Crow
  Fusor Data Logger
*********/

// Load Wi-Fi library
#include <WiFi.h>
#include <heltec.h>

// Replace with your network credentials
//const char* ssid     = "DMS Member";
//const char* password = "dms--10923";

const char* ssid     = "MySpectrumWiFi95-2G";
const char* password = "acreinput627";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String PeltierBlockState = "off";
String PeltierStackState = "off";
String TempPeltierState = "off";
String PEMelecState = "off";

// Assign output variables to GPIO pins
const int PeltierBlockPWM = 26;
const int PeltierStackPWM = 27;
const int ADCPeltierTemp = 36;
const int PEMelecPWM = 38;

//Init the 4 circuits
int PeltierBlockValPWM = 128;
int PeltierStackValPWM = 128;
int PEMelecValPWM = 128;
int ADCPeltierTempVal = 20;


  
void setup() {

  //set the driver pins for PWM OUTPUTS
ledcAttachPin(PeltierBlockPWM, 0);
ledcAttachPin(PeltierStackPWM, 1);
ledcAttachPin(PEMelecPWM, 2);
  
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(PeltierBlockPWM, OUTPUT);
  pinMode(PeltierStackPWM, OUTPUT);
  
  // Set outputs to LOW
  digitalWrite(PeltierBlockPWM, LOW);
  digitalWrite(PeltierStackPWM, LOW);

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
    Heltec.display -> drawString(0, 0, "Fusor PEM Gas Dryer 0.41");
    Heltec.display -> drawString(0, 15, (WiFi.localIP().toString()));
    Heltec.display -> drawString(0, 30, "USB-Serial 115200");
    Heltec.display -> display();
    
}


void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("DAC-MFC on");
              PeltierBlockState = "on";
              digitalWrite(PeltierBlockPWM, HIGH);
            } else if (header.indexOf("GET /26/off") >= 0) {
              Serial.println("DAC-MFC off");
              PeltierBlockState = "off";
              digitalWrite(PeltierBlockPWM, LOW);
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("PeltierStack ON");
              PeltierStackState = "on";
              digitalWrite(PeltierStackPWM, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("PeltierStack OFF");
              PeltierStackState = "off";
              digitalWrite(PeltierStackPWM, LOW);
            } else if (header.indexOf("GET /36/on") >= 0) {
              Serial.println("ADCPeltierTemp off");
              TempPeltierState = "on";
              //call Function ADCPeltierTemp analogRead() and analyze
            } else if (header.indexOf("GET /36/off") >= 0) {
              Serial.println("ADCPeltierTemp off");
              TempPeltierState = "off";
            } else if (header.indexOf("GET /38/on") >= 0) {
              Serial.println("ADCinput4 off");
              PEMelecState = "on";
              //call Function ADCPeltierTemp analogRead() and analyze
            } else if (header.indexOf("GET /38/off") >= 0) {
              Serial.println("ADCinput4 off");
              PEMelecState = "off";
            }

            

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #FF0000;}</style></head>");  
            
            // Web Page Heading
            client.println("<body><h1>Fusor PEM Gas Dryer v0.41</h1>");
            
            client.println("<p>" + (WiFi.localIP().toString()) + " and USB-Serial 115200 </p>");


            
            // Display current state, and ON/OFF buttons for PeltierBlockPWM  
            client.println("<p>PeltierBlock - State " + PeltierBlockState + "</p>");
            // If the PeltierBlockState is off, it displays the OFF button       
            if (PeltierBlockState=="off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">ON</button></a></p>");
              client.println("<table><tr> PWM level </tr><tr><form><input type='text'><id='PWMvalBlock'><name='PWMvalBlock'><br></form></tr></table>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            client.println("<p>PeltierStack - State " + PeltierStackState + "</p>");
            // If the PeltierStackState is off, it displays the OFF button       
            if (PeltierStackState=="off") {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">ON</button></a></p>");
              client.println("<table><tr> PWM level </tr><tr><form><input type='text'><id='PWMvalStack'><name='PWMvalStack'><br></form></tr></table>");
            }
            client.println("</body></html>");

            // Display current state, and ON/OFF buttons for ADC 0  
            client.println("<p>TempPeltier - State " + TempPeltierState + "</p>");
            // If the PeltierStackState is off, it displays the OFF button       
            if (TempPeltierState=="off") {
              client.println("<p><a href=\"/36/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/36/off\"><button class=\"button button2\">ON</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for ADC 0  
            client.println("<p>PEM Electrolyzer " + PEMelecState + "</p>");
            // If the PeltierStackState is off, it displays the OFF button       
            if (PEMelecState=="off") {
              client.println("<p><a href=\"/38/on\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.println("<p><a href=\"/38/off\"><button class=\"button button2\">ON</button></a></p>");
              client.println("<table><tr> PWM level </tr><tr><form><input type='text'><id='PWMvalPEM'><name='PWMvalPEM'><br></form></tr></table>");
            }

            
            client.println("</body></html>");
            
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
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

//*****FUNCTIONS*****//

void ADC0_get()
{
  ADCPeltierTempVal = analogRead (ADCPeltierTemp);
  return;
  }   

 void PeltierBlockWritePWM()
  {
  ledcWrite(0, PeltierBlockValPWM);
  Serial.print("PeltierBlockValPWM = ");
  Serial.println (PeltierBlockValPWM);
  return;
  }

 void PeltierStackWritePWM()
    {
  ledcWrite(1, PeltierStackValPWM);
  Serial.print("PeltierStackValPWM = ");
  Serial.println (PeltierStackValPWM);
  return;
  } 

  void PEMelecWritePWM()
  {
  ledcWrite(2, PEMelecValPWM);
  Serial.print("PEMelecValPWM = ");
  Serial.println (PEMelecValPWM);
  return;
  }
          
