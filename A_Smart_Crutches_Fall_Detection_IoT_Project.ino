/********************************************************************************
 *                                                                              *
 *                                                                              *
 *                                                                              *
 ********************************************************************************/
#include <Arduino.h>
#include <Notecard.h>
#include <string.h>

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
#define ENABLE  1
#define DISABLE 0 //

#define BUTTON_PIN USER_BTN
// Set this to 1 to disable debugging logs
#define RELEASE 1
#define VERSION_NUMBER "1.0.0"

#define LED_RED       PA7
#define LED_YELLOW    PB0       
#define LED_GREEN     PB1       
#define BUZZER        PB4       
#define BUILTIN_LED   PC13   

#define usbSerial Serial
#define productUID "com.gmail.elijahmalukes:a_smart_crutches_fall_detection_iot_project"

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
Notecard notecard;

volatile bool locationRequested = false;
static bool led_state = false;

//
bool connected = false;
int i = 0;
int total = 0;

// card.location
double lat = 0;
double lon = 0;
size_t gps_time_s;

// card.temp
int value = 0;

// card.motion
bool alert_motion = false;
J *alert_motion_str = NULL;
char *status_motion = "face-up";
int motion = 0;
J *motion_str = NULL;

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
void alert(uint delay_ms, uint repeat);
void locationRequest(void);
void sendMessage(double lat, double lon);

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
void setup()
{
  delay(100);                  // wait for a second
  locationRequested = false;

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

  // Set Notecard Settings
  #if  ENABLE //
    J *req = notecard.newRequest("hub.set");
    JAddStringToObject(req, "product", productUID);
    JAddStringToObject(req, "mode", "continuous");
    JAddBoolToObject(req, "sync", true);
    if (!notecard.sendRequest(req)) {
      //JDelete(req);
    }
  #endif

  // Notify Notehub of the current firmware version
  #if  DISABLE //
    req = notecard.newRequest("dfu.status");
    JAddStringToObject(req, "version", VERSION_NUMBER);
    if (!notecard.sendRequest(req)) {
      //JDelete(req);
    }
  #endif

  // Enable Notecard Outboard Firmware Update
  #if  ENABLE //
    req = notecard.newRequest("card.dfu");
    JAddBoolToObject(req, "off", true);
    JAddStringToObject(req, "name", "stm32");
    if (!notecard.sendRequest(req)) {
      //JDelete(req);
    }
  #endif

  // Add temperature monitoring
  #if  DISABLE //
    req = notecard.newRequest("card.temp");
    JAddNumberToObject(req, "minutes", 60);
    if (!notecard.sendRequest(req)) {
      //JDelete(req);
    }
  #endif

  // Pull AUX1 low during DFU
  #if  DISABLE //
    req = notecard.newRequest("card.aux");
    JAddStringToObject(req, "mode", "dfu");
    if (!notecard.sendRequest(req)) {
      //JDelete(req);
    }
  #endif

  // 
  #if ENABLE //
    req = notecard.newRequest("card.location.mode");
    JAddStringToObject(req, "mode", "periodic");
    JAddNumberToObject(req, "seconds", 60 * 5);
    if (!notecard.sendRequest(req)) {
      //JDelete(req);
    }
  #endif

  // 
  #if ENABLE 
    req = notecard.newRequest("card.location.track");
    JAddBoolToObject(req, "stop", true);
    JAddBoolToObject(req, "heartbeat", false);
    JAddNumberToObject(req, "hours", 12);
    if (!notecard.sendRequest(req)) {
      //JDelete(req);
    }
  #endif
}

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
void loop() {
  char status[20];

  //
  if (led_state == false) {
    digitalWrite(LED_GREEN, LOW); 
    led_state = true;
  } else {
    digitalWrite(LED_GREEN, HIGH); 
    led_state = false;
  }

  // Card Status
  #if  ENABLE //
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
  #endif
  
  // Check if GPS/GNSS has acquired location information
  #if  ENABLE //
  Serial.println();
  if(J *req = notecard.newRequest("card.location")) {
    J *rsp = notecard.requestAndResponse(req);
    if (JGetInt(rsp, "time") != gps_time_s) {
      lat = JGetNumber(rsp, "lat");
      lon = JGetNumber(rsp, "lon");
      sendMessage(lat, lon);
    }
  }
  #endif

  // Notecard temperature readings
  #if  ENABLE //
  Serial.println();
  if (J *req = notecard.newRequest("card.temp")) {
    J *rsp = notecard.requestAndResponse(req);
    value = JGetNumber(rsp, "temp");
    notecard.logDebug(JConvertToJSONString(rsp));
  }
  #endif

  // Motion monitoring readings
  #if  ENABLE //
  Serial.println();
  if (J *req = notecard.newRequest("card.motion")) {
    J *rsp = notecard.requestAndResponse(req);
    notecard.logDebug(JConvertToJSONString(rsp));
    alert_motion_str = JGetObjectItemCaseSensitive(rsp, "alert");
    if(alert_motion_str != NULL) {
      Serial.print("\nalert: ");
      notecard.logDebug(JConvertToJSONString(alert_motion_str));
      alert(250, 5);
      alert(250, 5);
      alert_motion = true;
      locationRequested = true;
      alert_motion = false;
      alert_motion_str = NULL;
      sendMessage(lat, lon);
    }
    else { 
      Serial.print("\nalert: ");
      notecard.logDebug(JConvertToJSONString(alert_motion_str));
      locationRequested = false;
      alert_motion = false;
    } 
    // 
    motion_str = JGetObjectItemCaseSensitive(rsp, "motion");
    if(alert_motion_str != NULL) {
      motion = JGetNumber(rsp, "motion");
      Serial.print("\nmotion: ");
      notecard.logDebug(JConvertToJSONString(motion_str));
    }else { 
      Serial.print("\nmotion: ");
      notecard.logDebug(JConvertToJSONString(motion_str));
    } 
  }
  #endif

  //
  Serial.println("\n\n");
  delay(5000);
}

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
void locationRequest(void) {
  if (!locationRequested) {
    return;
  }
  
  // Save the time from the last location reading.
  J *rsp = notecard.requestAndResponse(notecard.newRequest("card.location"));
  gps_time_s = JGetInt(rsp, "time");
  NoteDeleteResponse(rsp);

  // Set the location mode to "continuous" mode to force the
  // Notecard to take an immediate GPS/GNSS reading.
  J *req = notecard.newRequest("card.location.mode");
  JAddStringToObject(req, "mode", "continuous");
  notecard.sendRequest(req);

  // How many seconds to wait for a location before you stop looking
  size_t timeout_s = 600;

  // Block while resolving GPS/GNSS location
  for (const size_t start_ms = ::millis();;) {
    // Check for a timeout, and if enough time has passed, break out of the loop
    // to avoid looping forever
    if (::millis() >= (start_ms + (timeout_s * 1000))) {
      notecard.logDebug("Timed out looking for a location\n");
      locationRequested = false;
      break;
    }
  
    // Check if GPS/GNSS has acquired location information
    J *rsp = notecard.requestAndResponse(notecard.newRequest("card.location"));
    if (JGetInt(rsp, "time") != gps_time_s) {
      lat = JGetNumber(rsp, "lat");
      lon = JGetNumber(rsp, "lon");
      sendMessage(lat, lon);
      //NoteDeleteResponse(rsp);

      // Restore previous configuration
      {
        J *req = notecard.newRequest("card.location.mode");
        JAddStringToObject(req, "mode", "periodic");
        notecard.sendRequest(req);
      }

      locationRequested = false;
      break;
    }
  
    // If a "stop" field is on the card.location response, it means the Notecard
    // cannot locate a GPS/GNSS signal, so we break out of the loop to avoid looping
    // endlessly
    if (JGetObjectItem(rsp, "stop")) {
      notecard.logDebug("Found a stop flag, cannot find location\n");
      locationRequested = false;
      break;
    }
  
    NoteDeleteResponse(rsp);
    // Wait 2 seconds before trying again
    delay(2000);
  }
}

/********************************************************************************
 *                                                                              *
 ********************************************************************************/
void sendMessage(double lat, double lon) {
  notecard.logDebugf("Location: %.12lf, %.12lf\n", lat, lon);

  // http://maps.google.com/maps?q=<lat>,<lon>
  char buffer[100];
  snprintf(
    buffer,
    sizeof(buffer),
    "A right hand side crutch has fallen at the following location: https://maps.google.com/maps?q=%.12lf,%.12lf",
    lat,
    lon
   );
  notecard.logDebug(buffer);

  J *req = notecard.newRequest("note.add");
  //JAddStringToObject(req, "file", "alert.qo");
  //JAddBoolToObject(req, "sync", true);
  J *body = JCreateObject();
  JAddStringToObject(body, "message", buffer);
  JAddBoolToObject(body, "alert_motion", alert_motion);
  JAddStringToObject(body, "status_motion", status_motion);
  JAddNumberToObject(body, "motion", motion);
  JAddNumberToObject(body, "temp_value", value);
  JAddNumberToObject(body, "lat", lat);
  JAddNumberToObject(body, "lon", lon);
  JAddNumberToObject(body, "gps_time_s", gps_time_s);
  JAddItemToObject(req, "body", body);
  if (!notecard.sendRequest(req)) {
    //JDelete(req);
  }

  //
  req = notecard.newRequest("hub.sync");
  if (req) {
    notecard.sendRequest(req);
  }

  notecard.logDebug("Location sent successfully.\n");
}

/********************************************************************************
 *                                                                              *
 ********************************************************************************/