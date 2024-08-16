/********************************************************************************
 *                                                                              *
 *                                                                              *
 *                                                                              *
 ********************************************************************************/
#include <Arduino.h>
#include <Notecard.h>
#include <string.h>

#define LED_RED       PA7
#define LED_YELLOW    PB0       
#define LED_GREEN     PB1       
#define BUZZER        PB4       
#define BUILTIN_LED   PC13   

#define usbSerial Serial
#define productUID "com.gmail.elijahmalukes:a_smart_crutches_fall_detection_iot_project"
Notecard notecard;

//
bool connected = false;
int i = 0;
int total = 0;

// card.location
int lat = 0;
int lon = 0;
int card_time = 0;

// card.temp
int value = 23.134;

// card.motion
bool alert_motion = false;
J *alert_motion_str = NULL;
char *status_motion = "face-up";
int motion = 0;
J *motion_str = NULL;

/*
 
  {"usb":true,"connected":true,"status":"{normal}","storage":2,"time":1723460815,"cell":true}
  {"status":"face-up","motion":1723463187}
  {"value":26.6875,"calibration":-1}
  {"status":"GPS is off {gps-inactive}","mode":"off","lat":-25.9229375,"lon":28.02085937500001,"dop":20,"time":1723460830}
  */
/********************************************************************************
 *                                                                              *
 ********************************************************************************/
void alert(uint delay_ms, uint repeat);

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
void alert(uint delay_ms, uint repeat){
  for(i  = 0; i <= repeat; i++) {
    digitalWrite(LED_RED, HIGH);   //
    digitalWrite(LED_YELLOW, HIGH);   //
    digitalWrite(LED_GREEN, HIGH);   //
    digitalWrite(BUZZER, HIGH);   //
    digitalWrite(BUILTIN_LED, LOW);  //
    delay(delay_ms);                  // wait for a second
    digitalWrite(LED_RED, LOW);   //
    digitalWrite(LED_YELLOW, LOW);   //
    digitalWrite(LED_GREEN, LOW);   //
    digitalWrite(BUZZER, LOW);   //
    digitalWrite(BUILTIN_LED, HIGH);  //
    delay(delay_ms);                  // wait for a second
  }
}

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
void setup()
{
  delay(100);                  // wait for a second

  /* initialize digital pins. */
  pinMode(LED_RED, OUTPUT);     // initialize digital pin PB0 as an output.
  pinMode(LED_YELLOW, OUTPUT);     // initialize digital pin PB1 as an output.
  pinMode(LED_GREEN, OUTPUT);     // initialize digital pin PB1 as an output.
  pinMode(BUZZER, OUTPUT);     // initialize digital pin PB4 as an output.
  pinMode(BUILTIN_LED, OUTPUT);    // initialize digital pin PC13 as an output.

  /* initialize digital pins. */
  for(i  = 0; i <=2; i++) {

    digitalWrite(LED_RED, HIGH);   //
    digitalWrite(LED_YELLOW, HIGH);   //
    digitalWrite(LED_GREEN, HIGH);   //
    digitalWrite(BUZZER, HIGH);   //
    digitalWrite(BUILTIN_LED, LOW);  //
    delay(1000);                  // wait for a second
    digitalWrite(LED_RED, LOW);   //
    digitalWrite(LED_YELLOW, LOW);   //
    digitalWrite(LED_GREEN, LOW);   //
    digitalWrite(BUZZER, LOW);   //
    digitalWrite(BUILTIN_LED, HIGH);  //
    delay(1000);                  // wait for a second
  }

  usbSerial.begin(115200);
  notecard.begin();
  notecard.setDebugOutputStream(usbSerial);

  //Set Notecard Settings
  J *req = notecard.newRequest("hub.set");
  JAddStringToObject(req, "product", productUID);
  JAddStringToObject(req, "mode", "continuous");
  JAddBoolToObject(req, "sync", true);
  notecard.sendRequest(req);
}

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
void loop() {
  char status[20];

  // Card Status
  Serial.println();
  if (J *req = notecard.newRequest("card.status")) {
    J *rsp = notecard.requestAndResponse(req);
    notecard.logDebug(JConvertToJSONString(rsp));
    bool connected = JGetBool(rsp, "connected");
    char *tempStatus = JGetString(rsp, "status");
    strlcpy(status, tempStatus, sizeof(status));
    int storage = JGetInt(rsp, "storage");
    int time = JGetInt(rsp, "time");
    bool cell = JGetBool(rsp, "cell");
    notecard.deleteResponse(rsp);

    // Turn Light On or Off
    if (connected == true) {
      digitalWrite(BUILTIN_LED, LOW);
      delay(1000);
      digitalWrite(BUILTIN_LED, HIGH);
      delay(1000);
    }
    else {
      digitalWrite(BUILTIN_LED, LOW);
      delay(250);
      digitalWrite(BUILTIN_LED, HIGH);
      delay(250);
    }
  }
  
  // Motion monitoring readings
  Serial.println();
  if (J *req = notecard.newRequest("card.motion"))
  {
    J *rsp = notecard.requestAndResponse(req);
    notecard.logDebug(JConvertToJSONString(rsp));
    alert_motion_str = JGetObjectItemCaseSensitive(rsp, "alert");
    if(alert_motion_str != NULL) {
      Serial.print("\nalert: ");
      notecard.logDebug(JConvertToJSONString(alert_motion_str));
      alert(250, 5);
      alert_motion = true;
    }else { 
      Serial.print("\nalert: ");
      notecard.logDebug(JConvertToJSONString(alert_motion_str));
      alert_motion = false;
    } 
    // 
    motion_str = JGetObjectItemCaseSensitive(rsp, "motion");
    if(alert_motion_str != NULL) {
      Serial.print("\nmotion: ");
      notecard.logDebug(JConvertToJSONString(motion_str));
      motion = 0;
    }else { 
      Serial.print("\nmotion: ");
      notecard.logDebug(JConvertToJSONString(motion_str));
      motion = 0;
    } 
    
    notecard.deleteResponse(rsp);
  }

  // Notecard temperature readings
  Serial.println();
  if (J *req = notecard.newRequest("card.temp"))
  {
    J *rsp = notecard.requestAndResponse(req);
    notecard.logDebug(JConvertToJSONString(rsp));
    notecard.deleteResponse(rsp);
  }

  // Card location readings
  Serial.println();
  if (J *req = notecard.newRequest("card.location"))
  {
    J *rsp = notecard.requestAndResponse(req);
    notecard.logDebug(JConvertToJSONString(rsp));
    notecard.deleteResponse(rsp);
  }

  //
  Serial.println();
  if (alert_motion == true)
  {
    if (J *req = notecard.newRequest("note.add")) {
      J *body = JCreateObject();
      JAddBoolToObject(body, "alert_motion", alert_motion);
      JAddStringToObject(body, "status_motion", status_motion);
      JAddNumberToObject(body, "motion", motion);
      JAddNumberToObject(body, "temp_value", value);
      JAddNumberToObject(body, "lat", lat);
      JAddNumberToObject(body, "lon", lon);
      JAddNumberToObject(body, "card_time", card_time);
      JAddItemToObject(req, "body", body);
      notecard.sendRequest(req);
    }

    //
    Serial.println();
    J *req = notecard.newRequest("hub.sync");
      if (req) {
        notecard.sendRequest(req);
    }
  }

  //
  Serial.println("\n\n");
  delay(5000);
}

/********************************************************************************
 *                                                                              *
 ********************************************************************************/