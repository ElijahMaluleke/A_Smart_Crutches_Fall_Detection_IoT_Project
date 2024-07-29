/*
  
 */

/********************************************************************************
 * the setup function runs once when you press reset or power the board
 ********************************************************************************/
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  
  pinMode(PB4, OUTPUT);digitalWrite(PB4, LOW);
  pinMode(PC13, OUTPUT);
  Serial.println("Turns an LED on for one second, then off for one second, repeatedly.");
}

/********************************************************************************
 * the loop function runs over and over again forever
 ********************************************************************************/
void loop() {
  //digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  digitalWrite(PB4, LOW);
  digitalWrite(PC13, LOW);
  Serial.println("LED on for one second");
  delay(1000);                      // wait for a second
  //digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  digitalWrite(PB4, HIGH);
  digitalWrite(PC13, LOW);
  Serial.println("LED off for one second");
  delay(1000);                      // wait for a second
}
