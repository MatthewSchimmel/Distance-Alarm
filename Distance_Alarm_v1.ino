#define blueLight 0
#define redLight 1
#define echoPin 11 // attach pin D2 Arduino to pin Echo of HC-SR04
#define trigPin 10 // attach pin D3 Arduino to pin Trig of HC-SR04

long duration; // variable for the duration of sound wave travel
int distance;  // variable for the distance measurement

bool alternator = false; // variable for the alarmLights method
const int button = 16;   // GP Pin 16
const int buzzer = 3;    // GP Pin 3
const int speedOfSound = 0.034;
const int threshold = 10;//in centimeters

void setup() {
  Serial.begin(9600); // Serial Communication is starting with 9600 of baudrate speed
  Serial.println("Ultrasonic Sensor HC-SR04 Test"); // print some text in Serial Monitor
  Serial.println("with Arduino UNO R3");
  //given code
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //configure pin 16 as an input and enable the internal pull-up resistor
  pinMode(button, INPUT_PULLUP);

  pinMode(buzzer, OUTPUT);
  pinMode(redLight, OUTPUT);
  pinMode(blueLight, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); //led on

}
bool buttonIsPressed(){
  int sensorVal = digitalRead(button); //param = button GP pin
  if (sensorVal == LOW) { //if button is pressed
    return true;
  } else {
    return false;
  }
}

void triggerAlarm(){
  triggerAlarm(false);
}
//prints the message right after distance is printed
void triggerAlarm(bool trigger){
  if(trigger == true){
    Serial.println(", Alarm Triggered! >:(");
  } else {
    Serial.println("  :)");
  }
  alarmLights(trigger);
}
// illuminates two lights in an alternating fashion. Buzzer included as well.
void alarmLights(bool trigger){
  if (trigger == true) {
    if (alternator == false) {
      digitalWrite(buzzer, 10); // turn buzzer on
      digitalWrite(redLight, LOW);//red off
      digitalWrite(blueLight, HIGH);//blue on
      alternator = true;
    } else {
      digitalWrite(buzzer, 5); // turn buzzer on
      digitalWrite(blueLight, LOW);//blue off
      digitalWrite(redLight, HIGH);//red on
      alternator = false;
    }
  } else { //if alarm not triggered
    digitalWrite(buzzer, LOW); // turn buzzer off
    digitalWrite(blueLight, LOW);//blue off
    digitalWrite(redLight, LOW);//red off
  }
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
  distance = duration * 0.034 / 2; // Calculating the distance
  //Prints the distance, all on onel
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm");
  triggerAlarm((distance <= threshold) || buttonIsPressed());
  delay(500);
}
