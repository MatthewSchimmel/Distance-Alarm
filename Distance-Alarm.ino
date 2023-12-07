/*  Written by Matthew Ryan Schimmel in December of 2023
  matthewschimmel.site, 
*/
#include <Arduino_GFX_Library.h>
#define TFT_SCK    3
#define TFT_MOSI   17
#define TFT_MISO   8
#define TFT_CS     16
#define TFT_DC     15
#define TFT_RESET  7
Arduino_ESP32SPI bus = Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, TFT_MISO);
Arduino_ILI9341 display = Arduino_ILI9341(&bus, TFT_RESET);

#define blueLight 34
#define redLight 33

//Led's for the distance indicator
#define led1 1
#define led2 2
#define led3 42
#define led4 41
#define led5 40
#define led6 39
#define led7 38
#define led8 37
#define led9 36
#define led10 35

#define echoPin 4  // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 5  // attach pin D3 Arduino to pin Trig of HC-SR04
//-- Wifi Segment --//
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
// Set these to your desired credentials.
const char *ssid = "Nittany";
const char *password = "";
//remember: the website to visit is: http://192.168.4.1
WiFiServer server(80);
//------------------//
//touchPin: lecture 21
//screen: lecture 21
long duration;  // variable for the duration of sound wave travel
int distance;   // variable for the distance measurement
bool noise = true;
String output[20] = {"","","","","","","","","","","","","","","","","","","",""};

bool alternator = false;  // variable for the alarmLights method
const int alarmButton = 0;
const int UpButton = 16;   // GP Pin 16
const int DownButton = 15;   // GP Pin 16
const int speedOfSound = 0.034;
int threshold = 5;  //in centimeters

const int buzzer = 19;  // GP Pin 3

void setup() {
  pinMode(alarmButton, INPUT_PULLUP);
  pinMode(UpButton, INPUT_PULLUP);
  pinMode(DownButton, INPUT_PULLUP);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);
  pinMode(led7, OUTPUT);
  pinMode(led8, OUTPUT);
  pinMode(led9, OUTPUT);
  pinMode(led10, OUTPUT);

  pinMode(buzzer, OUTPUT);
  pinMode(redLight, OUTPUT);
  pinMode(blueLight, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  setupDisplay();
  setupServer();
  //given code
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  delay(100);
}
void clearScreen(){
  display.fillScreen(BLACK);
  display.setCursor(0, 1);
}

void setupDisplay(){
  display.begin();
  display.fillScreen(BLACK);
  display.setCursor(0, 1);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Display Initialized");
}

void setupServer(){
  Serial.begin(115200);
  displayPrint("Setting A.P...");
  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }
  IPAddress myIP = WiFi.softAPIP();
  //This part cannot be obfuscated through displayPrint, so don't try.
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  Serial.println("Server started");
  display.print("AP IP address: ");
  display.println(myIP);
  display.println("Server started");
  server.begin();
}

void loop() {
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds. Given code.
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;  // Calculating the distance
  //checkThreshold();
  //Prints the distance, all on one
  clearScreen();
  illuminateDistance(distance);
  triggerAlarm((distance <= threshold) || buttonIsPressed());

  //-- WiFi management --//
  WiFiClient client = server.available();  // listen for incoming clients

  if (client) {                     // if you get a client,
    Serial.println("New Client.");  // print a message out the serial port
    display.println("New Client.");  // print a message out the display
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
            client.print("<span style=font-size:6em>Click <a href=\"/H\">here</a> to turn ON the buzzer.<br>");
            client.print("<span style=font-size:6em>Click <a href=\"/L\">here</a> to turn OFF the buzzer.<br>");

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

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          noise = true;
        }
        if (currentLine.endsWith("GET /L")) {
          noise = false;
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
    display.println("Client Disconnected.");
  }
  delay(250);
}

void displayPrint(String newline){
  shiftLines();
  output[0] = newline;
  printOutput();
}
void shiftLines(){
  for (int i = 19; i > 0; i--){//iterates through every index but the last
    output[i] = output[i - 1];
  }
  output[0] = "";
}
void printOutput(){
  clearScreen();
  for (String line: output){
    display.println(line);
  }
  Serial.println(output[0]);
}

void checkThreshold() {
  int UpButtonVal = digitalRead(UpButton);  //param = button GP pin
  int DownButtonVal = digitalRead(DownButton);  //param = button GP pin
  if (UpButtonVal == LOW && threshold > 10) {                //if button is pressed
    threshold -= 10;
    //displayPrint("Threshold decreased to " + threshold + "cm")
    flash(threshold / 10);
  }
  if (DownButtonVal == LOW) {  //if button is pressed
    threshold += 10;
    //displayPrint("Threshold increased to " + threshold + "cm")
    flash(threshold / 10);
  }
}

void flash(int iterations){
  for (int i = 0; i < iterations ; i++) {
    digitalWrite(BUILTIN_LED, HIGH);
    delay(100);
    digitalWrite(BUILTIN_LED, LOW);
    delay(100);
  }
}

bool buttonIsPressed() {
  int buttonVal = digitalRead(alarmButton);  //param = button GP pin
  if (buttonVal == LOW) {                    //if button is pressed
    return true;
  } else {
    return false;
  }
}



void triggerAlarm() {
  triggerAlarm(false);
}
//prints the message right after distance is printed
void triggerAlarm(bool trigger) {
  String output = "Distance: ";
  output += String(distance);
  output += "cm";
  if (trigger == true) {
    display.setTextColor(RED);
    output += " >:(";
    displayPrint(output);
  } else {
    if (getNumOfLights() > 6) {
      output += " :|";
      display.setTextColor(YELLOW);
    } else {
    output += " :)";
      display.setTextColor(GREEN);
    }
    displayPrint(output);
  }
  Serial.print(output[0]);
  display.print(output[0]);
  alarmLights(trigger);
}
// illuminates two lights in an alternating fashion. Buzzer included as well.
void alarmLights(bool trigger) {
  if (trigger == true) {
    if (alternator == false) {
      if (noise) {
        digitalWrite(buzzer, HIGH);  //buzzer on
      }
      digitalWrite(redLight, LOW);   //red off
      digitalWrite(blueLight, HIGH); //blue on
      alternator = true;
    } else {
      if (noise) {
        digitalWrite(buzzer, HIGH);  //buzzer on
      }
      digitalWrite(blueLight, LOW);  //blue off
      digitalWrite(redLight, HIGH);  //red on
      alternator = false;
    }
  } else {                         //if alarm not triggered
    digitalWrite(buzzer, LOW);     // turn buzzer off
    digitalWrite(blueLight, LOW);  //blue off
    digitalWrite(redLight, LOW);   //red off
  }
}

void illuminateDistance(){
   illuminateDistance(.3);
}
void illuminateDistance(double ratio){
  int numOfLights = getNumOfLights(ratio);
  clearTheLights();
  if(numOfLights > 0){
    digitalWrite(led1, HIGH);  //led on
  }
  if(numOfLights > 1){
    digitalWrite(led2, HIGH);  //led on
  }
  if(numOfLights > 2){
    digitalWrite(led3, HIGH);  //led on
  }
  if(numOfLights > 3){
    digitalWrite(led4, HIGH);  //led on
  }
  if(numOfLights > 4){
    digitalWrite(led5, HIGH);  //led on
  }
  if(numOfLights > 5){
    digitalWrite(led6, HIGH);  //led on
  }
  if(numOfLights > 6){
    digitalWrite(led7, HIGH);  //led on
  }
  if(numOfLights > 7){
    digitalWrite(led8, HIGH);  //led on
  }
  if(numOfLights > 8){
    digitalWrite(led9, HIGH);  //led on
  }
  if(numOfLights > 9){
    digitalWrite(led10, HIGH);  //led on
  }
}
int getNumOfLights(){
  return getNumOfLights(0.3);
}
int getNumOfLights(double ratio){//algorithm determines how many thresholds away the user is
double interval = threshold * ratio;
  if(buttonIsPressed()){
    return 10;
  } else {
    return 10 - ((distance - threshold) / interval); //the closer to threshold, the more lights
  }
}
void clearTheLights(){
  digitalWrite(led1, LOW);  //led off
  digitalWrite(led2, LOW);  //led off
  digitalWrite(led3, LOW);  //led off
  digitalWrite(led4, LOW);  //led off
  digitalWrite(led5, LOW);  //led off
  digitalWrite(led6, LOW);  //led off
  digitalWrite(led7, LOW);  //led off
  digitalWrite(led8, LOW);  //led off
  digitalWrite(led9, LOW);  //led off
  digitalWrite(led10, LOW);  //led off
}