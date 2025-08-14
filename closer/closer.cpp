/*
 Door Controller WiFi Server

 A simple web server that receives door state events from the iOS app.
 When the app detects an open door, it sends a GET request to /open
 and the ESP32 prints "open" to the serial monitor.

 Circuit:
 * ESP32 connected to WiFi

 */

 #include <WiFi.h>

 const char *ssid = "NETWORK_NAME";
 const char *password = "WIFI_PASSWORD";
 
 WiFiServer server(80);
 
 void setup() {
   Serial.begin(115200);
   
   delay(10);
 
   // We start by connecting to a WiFi network
   Serial.println();
   Serial.println();
   Serial.print("Connecting to ");
   Serial.println(ssid);
 
   WiFi.begin(ssid, password);
 
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
   }
 
   Serial.println("");
   Serial.println("WiFi connected.");
   Serial.println("IP address: ");
   Serial.println(WiFi.localIP());
 
   server.begin();
   Serial.println("Door controller server started!");
   Serial.println("Ready to receive door events from iOS app!");
 }
 
 void loop() {
   WiFiClient client = server.accept();  // listen for incoming clients
 
   if (client) {                     // if you get a client,
     Serial.println("New Client.");  // print a message out the serial port
     String currentLine = "";        // make a String to hold incoming data from the client
     while (client.connected()) {    // loop while the client's connected
       if (client.available()) {     // if there's bytes to read from the client,
         char c = client.read();     // read a byte, then
         Serial.write(c);            // print it out the serial monitor
         if (c == '\n') {            // if the byte is a newline character
 
           // if the current line is blank, you got two newline characters in a row.
           // that's the end of the client HTTP request, so send a response:
           if (currentLine.length() == 0) {
             // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
             // and a content-type so the client knows what's coming, then a blank line:
             client.println("HTTP/1.1 200 OK");
             client.println("Content-type:text/html");
             client.println();
 
             // the content of the HTTP response follows the header:
             client.print("Door Controller Ready<br>");
             client.print("Send GET /open to indicate door is open<br>");
             client.print("Send GET /closed to indicate door is closed<br>");
 
             // The HTTP response ends with another blank line:
             client.println();
             // break out of the while loop:
             break;
           } else {  // if you got a newline, then clear currentLine:
             currentLine = "";
           }
         } else if (c != '\r') {  // if you got anything else but a carriage return character,
           currentLine += c;      // add it to the end of the currentLine
         }
 
         // Check to see if the client request was "GET /open" or "GET /closed":
         if (currentLine.endsWith("GET /open")) {
           Serial.println("open");
           Serial.println("Door is OPEN!");
         }
         if (currentLine.endsWith("GET /closed")) {
           Serial.println("Door is CLOSED");
         }
       }
     }
     // close the connection:
     client.stop();
     Serial.println("Client Disconnected.");
   }
 }
 