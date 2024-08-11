/********************************************************************************
 *                                                                              *
 *                                                                              *
 ********************************************************************************/
#include <Arduino.h>
#include <Notecard.h>

#define LED_RED     PA7
#define LED_YELLOW  PB0       
#define LED_GREEN   PB1       
#define BUZZER      PB4       
#define BUILTIN_LED PC13        

#define usbSerial Serial
#define productUID "com.gmail.elijahmalukes:a_smart_crutches_fall_detection_iot_project"
Notecard notecard;

int i;
int total;
char *message_char = NULL;
String message;

/********************************************************************************
 * the setup function runs once when you press reset or power the board
 ********************************************************************************/
void setup() {

  // Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  
  /* initialize digital pins. */
  pinMode(LED_RED, OUTPUT);     // initialize digital pin PB0 as an output.
  pinMode(LED_YELLOW, OUTPUT);     // initialize digital pin PB1 as an output.
  pinMode(LED_GREEN, OUTPUT);     // initialize digital pin PB1 as an output.
  pinMode(BUZZER, OUTPUT);     // initialize digital pin PB4 as an output.
  pinMode(BUILTIN_LED, OUTPUT);    // initialize digital pin PC13 as an output.

  /* initialize digital pins. */
  for(i  = 0; i <= 5; i++) {

    digitalWrite(LED_RED, HIGH);   //
    digitalWrite(LED_YELLOW, HIGH);   //
    digitalWrite(LED_GREEN, HIGH);   //
    digitalWrite(BUZZER, HIGH);   //
    digitalWrite(BUILTIN_LED, HIGH);  //
    delay(250);                  // wait for a second
    digitalWrite(LED_RED, LOW);   //
    digitalWrite(LED_YELLOW, LOW);   //
    digitalWrite(LED_GREEN, LOW);   //
    digitalWrite(BUZZER, LOW);   //
    digitalWrite(BUILTIN_LED, LOW);  //
    delay(250);                  // wait for a second
  }

  /**/ 
  Serial.println("A_Smart_Crutches_Fall_Detection_IoT_Project.");

  delay(1000);                  // wait for a second

  usbSerial.begin(115200);
  notecard.begin();
  notecard.setDebugOutputStream(usbSerial);

  /* Set Notecard Settings */
  J *req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "product", productUID);
  JAddStringToObject(req, "mode", "continuous");
  JAddBoolToObject(req, "sync", true);
  /* Per a conversation with Blues they stated to remove the following line. */
  //JAddIntToObject(req, "interval", 2);
  notecard.sendRequest(req);
}

/********************************************************************************
 * the loop function runs over and over again forever
 ********************************************************************************/
void loop() {

  // ###########################################################
  if (message_char == NULL) {

    /* Turn Light On or Off */
    if (message == "on") {
      digitalWrite(PC13, HIGH);     //
    }
    else if (message == "off") {
      digitalWrite(PC13, LOW);      //
    }
    else{
      digitalWrite(PC13, HIGH);     //
      delay(1000);
      digitalWrite(PC13, LOW);      //
    }
  }

  // ###########################################################
  Serial.println();         // 
  Serial.println("Determine How Many Notes are in Queue."); 
  /* Determine How Many Notes are in Que.  If Over 1 Delete a Note */ 
  if (J *req = notecard.newRequest("file.changes")) {
    JAddStringToObject(req, "file", "data.qi");
    J *rsp = notecard.requestAndResponse(req);
    total = JGetInt(rsp, "total");
    Serial.print("total messages: ");
    Serial.println(total);
    notecard.deleteResponse(rsp);
  }

  Serial.println();         // 
  Serial.println("Total notes in the Queue."); 
  /* Total notes in the Queue. */
  if (total > 1) {
    if (J *req = notecard.newRequest("note.get")) {
      JAddStringToObject(req, "file", "data.qi");
      JAddBoolToObject(req, "delete", true);
      notecard.sendRequest(req);
    }
  }

  // ###########################################################
  Serial.println();         // 
  Serial.println("Motion monitoring.");
  /* Motion monitoring */ 
  if (J *req = notecard.newRequest("card.motion")) {
    J *rsp = notecard.requestAndResponse(req);
    notecard.logDebug(JConvertToJSONString(rsp));
    //Get Message Value from Note
    J *data = JGetObject(rsp, "body");
    char *message_char = JGetString(data, "alert");
    String message(message_char);
    Serial.println("  ");
    Serial.println("Message:");
    Serial.println(message);
    //notecard.deleteResponse(rsp);
  }

  // ###########################################################
  Serial.println();         // 
  Serial.println("Notecard temperature readings.");
  // Notecard temperature readings
  if (J *req = notecard.newRequest("card.temp")) {
    J *rsp = notecard.requestAndResponse(req);
    notecard.logDebug(JConvertToJSONString(rsp));
  }
  
  // ###########################################################
  Serial.println();         // 
  Serial.println("Notecard GPS in periodic mode.");
  /* Notecard GPS in periodic mode */ 
  if (J *req = notecard.newRequest("card.location.mode")) {
    J *rsp = notecard.requestAndResponse(req);
    notecard.logDebug(JConvertToJSONString(rsp));
  }

  // ###########################################################
  Serial.println();         // 
  /*  */
  delay(5000);              // wait for a second
}

/********************************************************************************
 *                                                                              *
 ********************************************************************************/