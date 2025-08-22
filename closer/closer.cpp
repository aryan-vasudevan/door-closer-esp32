/*
 Door Controller WiFi Server with L298N

 A simple web server that receives door state events from the iOS app.
 When the app detects an open door, it sends a GET request to /open
 and the ESP32 activates the linear actuator to close the door.

 Circuit:
 * ESP32 connected to WiFi
 * L298N H-Bridge with linear actuator
 * ESP32 Pin 25 → L298N IN1
 * ESP32 Pin 26 → L298N IN2

 */

 #include <WiFi.h>

 const char *ssid = "Your WiFi name";
 const char *password = "Your WiFi password";
 
 WiFiServer server(80);
 
 // L298N H-Bridge pins
 const int IN1_PIN = 25;  // L298N IN1 (Forward/Extend)
 const int IN2_PIN = 26;  // L298N IN2 (Reverse/Retract)
 
 // Linear actuator control variables
 unsigned long lastActuatorUpdate = 0;
 bool isDoorClosing = false;
 bool extending = false;  // true when extending, false when retracting
 unsigned long doorCloseStartTime = 0;
 bool isInitialized = false;  // Track if actuator has been initialized
 unsigned long initStartTime = 0;  // Track initialization start time
 
 void setup() {
   Serial.begin(115200);
   
   // Set up L298N pins
   pinMode(IN1_PIN, OUTPUT);
   pinMode(IN2_PIN, OUTPUT);
   
   // Stop actuator initially
   digitalWrite(IN1_PIN, LOW);
   digitalWrite(IN2_PIN, LOW);
   
   Serial.println("L298N H-Bridge initialized");
   
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
   
   // Start initialization sequence - retract for 23 seconds to ensure starting position
   isInitialized = false;
   initStartTime = millis();
   digitalWrite(IN1_PIN, LOW);
   digitalWrite(IN2_PIN, HIGH);  // Start retracting
   Serial.println("Initializing: RETRACTING actuator to known position (23 seconds)...");
 }
 
 void loop() {
   // Handle initialization sequence first
   if (!isInitialized) {
     unsigned long currentTime = millis();
     unsigned long elapsed = currentTime - initStartTime;
     
     // Keep extending during initialization (to retract the actuator)
     digitalWrite(IN1_PIN, HIGH);  // Extend to retract actuator
     digitalWrite(IN2_PIN, LOW);
     
     if (elapsed >= 23000) {  // 23 seconds initialization
       // Stop actuator and mark as initialized
       digitalWrite(IN1_PIN, LOW);
       digitalWrite(IN2_PIN, LOW);
       isInitialized = true;
       Serial.println("Initialization complete - Actuator in retracted position");
       Serial.println("Ready to receive door events!");
     }
     return;  // Don't process door events during initialization
   }
   
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
           Serial.println("Door is OPEN! Activating linear actuator...");
           startDoorClosing();
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
   
   // Linear actuator control - runs when door needs to be closed
   controlLinearActuator();
 }
 
 
 
 void startDoorClosing() {
   if (!isInitialized) {
     Serial.println("Cannot start door closing - actuator not yet initialized");
     return;
   }
   
   if (isDoorClosing) return;  // Prevent multiple activations
   
   isDoorClosing = true;
   extending = true;
   doorCloseStartTime = millis();
   Serial.println("Starting linear actuator: EXTENDING to close door");
 }
 
 void controlLinearActuator() {
   if (!isDoorClosing) return;
   
   unsigned long currentTime = millis();
   unsigned long elapsed = currentTime - doorCloseStartTime;
   
   if (extending) {
     // Retract for 20 seconds to fully push door closed (200mm stroke)
     digitalWrite(IN1_PIN, LOW);  // Retract to close door
     digitalWrite(IN2_PIN, HIGH);
     
     if (elapsed >= 20000) {  // 20 seconds for full 200mm retraction
       extending = false;
       doorCloseStartTime = currentTime;  // Reset timer for extend phase
       Serial.println("Linear actuator: EXTENDING back to neutral (20 seconds)");
     }
   } else {
     // Extend for 20 seconds to fully return to neutral position
     digitalWrite(IN1_PIN, HIGH);  // Extend to return to neutral
     digitalWrite(IN2_PIN, LOW);
     
     if (elapsed >= 20000) {  // 20 seconds for full 200mm extension
       // Stop actuator and end sequence
       digitalWrite(IN1_PIN, LOW);
       digitalWrite(IN2_PIN, LOW);
       isDoorClosing = false;
       Serial.println("Door closing sequence complete - actuator stopped");
       Serial.println("Total cycle time: 40 seconds");
     }
   }
 }