/*  Written by Matthew Ryan Schimmel in December of 2023
  matthewschimmel.site
  Github: github.com/MatthewSchimmel/Distance-Alarm
*/
//TODO: issues with the website crashing, not present in the last 3.0 version despite no changes to any of the wifi code
#define blueLight 34
#define redLight 33

#include <TM1637.h>

// Instantiation and pins configurations for digit display
// Pin 3 - > DIO
// Pin 2 - > CLK
TM1637 tm(2, 3);

#define echoPin 4  // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 5  // attach pin D3 Arduino to pin Trig of HC-SR04
//-- Wifi Segment --//
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
// Set these to your desired credentials.
const char *ssid = "Nittany";
const char *password = "";
//the website to visit is: http://192.168.4.1
WiFiServer server(80);
String html ="<!DOCTYPE html> \
<html> \
<body> \
<center><h1>Distance Alarm Remote Control</h1></center> \
<center><h2>ESP32S2 Web Server Access Point</h2></center> \
<form> \
<button name=\"NOISE\" button style=\"color:green\" value=\"ON\" type=\"submit\">NOISE ON</button> \
<button name=\"NOISE\" button style=\"color=red\" value=\"OFF\" type=\"submit\">NOISE OFF</button><br><br> \
<button name=\"THRESHOLD\" button style=\"color:green\" value=\"UP\" type=\"submit\">THRESHOLD UP</button> \
<button name=\"THRESHOLD\" button style=\"color:red\" value=\"DOWN\" type=\"submit\">THRESHOLD DOWN</button> \
</form> \
</body> \
</html>";
long duration;  // variable for the duration of sound wave travel
int distance;   // variable for the distance measurement
bool noise = true;

bool alternator = false;  // variable for the alarmLights method
const int alarmButton = 0;
const int UpButton = 16;   // GP Pin 16
const int DownButton = 15;   // GP Pin 16
const int speedOfSound = 0.034;
int threshold = 10;  //in centimeters

const int buzzer = 19;  // GP Pin 3

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(redLight, OUTPUT);
  pinMode(blueLight, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  setupServer();
  //given code
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  tm.begin();
  tm.setBrightness(3);

  delay(100);
}

void changeThreshold(bool direction){
  if (direction == false) {// if direction is downward
    if (threshold > 10) {
      threshold -= 10;
      Serial.printf("Threshold decreased to %scm", threshold);
      flash(threshold / 10);
    }
  } else {// if direction is upward
    threshold += 10;
    Serial.printf("Threshold increased to %scm", threshold);
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

//prints the message right after distance is printed
void triggerAlarm() {
  bool trigger = distance <= threshold;
  Serial.printf("Distance: %dcm", distance);
  if (trigger == true) {
    Serial.println(" >:(");
  } else if (distance <= (threshold * 2)){
    Serial.println(" :|");
  } else {
    Serial.println(" :)");
  }
  alarmLights();
}

// if triggered, illuminates two lights in an alternating fashion, plus a buzzer.
void alarmLights() {
  bool trigger = distance  <= threshold;
  if (trigger == true) {
    if (alternator == false) {
      if (noise) {
        digitalWrite(buzzer, HIGH);  //buzzer on
      }
      digitalWrite(redLight, LOW);   //red led off
      digitalWrite(blueLight, HIGH); //blue led on
      alternator = true;
    } else {
      if (noise) {
        digitalWrite(buzzer, HIGH);  //buzzer on
      }
      digitalWrite(blueLight, LOW);  //blue led off
      digitalWrite(redLight, HIGH);  //red led on
      alternator = false;
    }
  } else {// if alarm not triggered
    digitalWrite(buzzer, LOW);   // buzzer off
    digitalWrite(blueLight, LOW);// blue led off
    digitalWrite(redLight, LOW); // red led off
  }
}

void setupServer(){
  Serial.begin(115200);
  Serial.println("Setting A.P...");
  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.print(myIP);
  Serial.println("Server started");
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
  tm.clearScreen();   // To remove old display artefacts
  tm.display(distance); //if distance = distance in last last loop iteration, doesn't print anything?
  triggerAlarm();

  //-- WiFi management --// 
  WiFiClient client = server.available();  // listen for incoming clients

  if (client) {// if you get a client,
    String request = client.readStringUntil('\r');
    if(request.indexOf("NOISE=ON") != -1) noise = true;
    if(request.indexOf("NOISE=OFF") != -1) noise = false;
    if(request.indexOf("THRESHOLD=UP") != -1) changeThreshold(true);
    if(request.indexOf("THRESHOLD=DOWN") != -1) changeThreshold(false);            
    client.print(html);
    client.stop();
    Serial.println("Client Disconnected.");
  }
  delay(250);
}