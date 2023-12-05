#define blueLight 1
#define redLight 2
#define echoPin 11  // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 10  // attach pin D3 Arduino to pin Trig of HC-SR04
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

long duration;  // variable for the duration of sound wave travel
int distance;   // variable for the distance measurement
bool noise = true;

bool alternator = false;  // variable for the alarmLights method
const int UpButton = 16;   // GP Pin 16
const int DownButton = 15;   // GP Pin 16
const int speedOfSound = 0.034;
int threshold = 10;  //in centimeters

const int buzzer = 3;  // GP Pin 3
const int buzzerFreq = 10;
const int fadeAmount = 5;

void setup() {
  //-- SERVER SETUP --//
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");
  // You can remove the password parameter if you want the AP to be open.
  // a valid password must have more than 7 characters
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1)
      ;
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  Serial.println("Server started");
  //-- END OF SERVER SETUP --//

  Serial.println("Ultrasonic Sensor HC-SR04 Test");  // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
  //given code
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //configure pin 16 as an input and enable the internal pull-up resistor
  pinMode(UpButton, INPUT_PULLUP);
  pinMode(DownButton, INPUT_PULLUP);

  pinMode(buzzer, OUTPUT);
  pinMode(redLight, OUTPUT);
  pinMode(blueLight, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
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
  checkThreshold();
  //Prints the distance, all on onel
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm");
  triggerAlarm((distance <= threshold) || buttonIsPressed());

  //-- WiFi management --//
  WiFiClient client = server.available();  // listen for incoming clients

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
  }
  delay(250);
}

void checkThreshold() {
  int UpButtonVal = digitalRead(UpButton);  //param = button GP pin
  int DownButtonVal = digitalRead(DownButton);  //param = button GP pin
  if (UpButtonVal == LOW && threshold > 10) {                //if button is pressed
    threshold -= 10;
    Serial.print("Threshold decreased to ");
    Serial.println(threshold);
    flash(threshold / 10);
  }
  if (DownButtonVal == LOW) {  //if button is pressed
    threshold += 10;
    Serial.print("Threshold increased to ");
    Serial.print(threshold);
    Serial.println("cm");
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
  return false;
  /*
  int sensorVal = digitalRead(UpButton);  //param = button GP pin
  if (sensorVal == LOW) {                //if button is pressed
    return true;
  } else {
    return false;
  }
  */
}



void triggerAlarm() {
  triggerAlarm(false);
}
//prints the message right after distance is printed
void triggerAlarm(bool trigger) {
  if (trigger == true) {
    Serial.println(", Alarm Triggered! >:(");
  } else {
    Serial.println("  :)");
  }
  alarmLights(trigger);
}
// illuminates two lights in an alternating fashion. Buzzer included as well.
void alarmLights(bool trigger) {
  if (trigger == true) {
    if (alternator == false) {
      if (noise) {
        digitalWrite(buzzer, 10);  // turn buzzer on
      }
      digitalWrite(redLight, LOW);    //red off
      digitalWrite(blueLight, HIGH);  //blue on
      alternator = true;
    } else {
      if (noise) {
        digitalWrite(buzzer, 5);  // turn buzzer on
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
