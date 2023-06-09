#include "HX711.h"
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>




//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "AndroidAPCBC7"
#define WIFI_PASSWORD "shaurya1"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBKChZBSyfRWPukRCR0tCSRIysk7jzXx5U"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://saline-monitor-system-default-rtdb.firebaseio.com/"


const int LOADCELL_DOUT_PIN = 12;
const int LOADCELL_SCK_PIN = 13;

HX711 scale;

//Define Firebase Data object
FirebaseData fbdo;
FirebaseJson json;
FirebaseAuth auth;
FirebaseConfig config;


String salineBottleIdPath = "/salineid";
String bedNumberPath = "/bednumber";
String salineLevelPath = "/salinelevel";




unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
bool flag = true;
void setup() {
  pinMode(16, OUTPUT);
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //Code Start
  Serial.begin(115200);
  Serial.println("HX711 Demo");
  Serial.println("Initializing the scale");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());  // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));  // print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight (not set) divided
                                          // by the SCALE parameter (not set yet)

  scale.set_scale(458.264);
  //scale.set_scale(-471.497);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();  // reset the scale to 0

  Serial.println("After setting up the scale:");

  Serial.print("read: \t\t");
  Serial.println(scale.read());  // print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  // print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));  // print the average of 5 readings from the ADC minus the tare weight, set with tare()

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);  // print the average of 5 readings from the ADC minus tare weight, divided
                                          // by the SCALE parameter set with set_scale

  Serial.println("Readings:");
  //code End
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {

  // put the ADC in sleep mode

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)) {
    scale.power_up();
    //code start
    double x = scale.get_units();
    Serial.print("one reading:\t");
    Serial.print(x, 1);

    Serial.print("\t| average:\t");
    Serial.println(x, 5);


    //code end
    if (x - 35 >= 0) {
      sendDataPrevMillis = millis();
      json.set(salineBottleIdPath.c_str(), 1);
      json.set(bedNumberPath.c_str(), 158);
      json.set(salineLevelPath.c_str(), x - 35);
      // Write an Int number on the database path test/int
      // Write an Float number on the database path test/float
      Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, "Hospital/salinedata", &json) ? "ok" : fbdo.errorReason().c_str());
    }
    if (flag == true) {
      digitalWrite(16, HIGH);
      flag = false;
      delay(10000);
      digitalWrite(16, LOW);
    }
    scale.power_down();
  }
}