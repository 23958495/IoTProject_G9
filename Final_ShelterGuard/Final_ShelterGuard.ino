//Libraries used along the project, including the script with secret data
#include "secrets.h"
#include <Preferences.h>
#include <AceButton.h>
using namespace ace_button;
Preferences pref;
#include <SPI.h>
#include <Wire.h>
#include "SparkFun_VEML6030_Ambient_Light_Sensor.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp32.h>

// Light Sensor VEML6030 Configuration
#define AL_ADDR 0x10
SparkFun_Ambient_Light light(AL_ADDR);
// Recommended Values for the actual scenario to calibrate the sensor
float gain = 0.25;
int _time = 100;
long luxVal = 0;

// BME280 Sensor Config
Adafruit_BME280 bme;
#define SEALEVELPRESSURE_HPA (1018.81)

// Screen Configuration
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Ports dedicated to the buttons and relays
const int BUTTON1_PIN = 13;
const int BUTTON2_PIN = 14;
const int relay = 25;
const int relaytwo = 4;

// Variables Used during the programm to store the sensor data, and states.
// Thresholds
float setTemp = 0;
float setHumi = 0;
float setLUX = 0;
//Current measures
float currentTemp = 0;
float currentHumi = 0;
float currentLUX = 0;
float currentPress = 0;
// Variables to check the state of the light 
float lastLUX = 0;
float wrongLUX = 0;
// Operation Mode
int modeState = 1;
// Alerts variables
int numberAlerts = 0;
bool TempAlertState = 0;
bool HumiAlertState = 0;
bool LUXAlertState = 0;
bool AlertMode = LOW;
int flag = 0;
String alerts = "Humidity: OK";
// Relay States variables
bool FanState = LOW;
bool LightState = LOW;
bool HumiState = LOW;


// Create a variable to initialize a timer
BlynkTimer timer;

// BLYNK Function to write and read information from the dashboard.
// Relay Status of the Light System, if it is in mode OFF not operate
BLYNK_WRITE(VPIN_Light) {
  if (modeState != 2) {
    LightState = param.asInt();
    digitalWrite(relaytwo, !LightState);
    pref.putBool("Light", LightState);
  }
}
// Relay Status of the Fan System, if it is in mode OFF not operate
BLYNK_WRITE(VPIN_Fan) {
  if (modeState != 2) {
    FanState = param.asInt();
    digitalWrite(relay, !FanState);
    pref.putBool("Fan", FanState);
  }
}
// Write and read the thresholds values
BLYNK_WRITE(VPIN_setTemp) {
  setTemp = param.asFloat();
  pref.putBool("setemp", setTemp);
}
BLYNK_WRITE(VPIN_setHumi) {
  setHumi = param.asFloat();
  pref.putBool("Humidity", setHumi);
}
BLYNK_WRITE(VPIN_setLUX) {
  setLUX = param.asFloat();
  pref.putBool("Light", setLUX);
}
// Write and read the operation mode
BLYNK_WRITE(VPIN_Mode) {
  modeState = param.asInt();
  pref.putBool("Mode", modeState);
}


// Blynk function to update the information in the Cloud Platform
BLYNK_CONNECTED() {
  // update the latest state to the server
  // Number of Alerts
  Blynk.virtualWrite(VPIN_Alerts, "Sensors Alerted: " + String(numberAlerts));
  // Operation Mode
  Blynk.virtualWrite(VPIN_Mode, modeState);
  // Temperature and LUX Alert State
  Blynk.virtualWrite(VPIN_Temp, TempAlertState );
  Blynk.virtualWrite(VPIN_LUX, LUXAlertState);
  // In case of humidity alert, we show the message
  Blynk.virtualWrite(VPIN_Text, alerts);
  // Sensor Data
  Blynk.syncVirtual(VPIN_currentTemp);
  Blynk.syncVirtual(VPIN_currentHumi);
  Blynk.syncVirtual(VPIN_currentLUX);
  Blynk.syncVirtual(VPIN_currentPress);
  // Thresholds
  Blynk.syncVirtual(VPIN_setTemp);
  Blynk.syncVirtual(VPIN_setHumi);
  Blynk.syncVirtual(VPIN_setLUX);
  // Relay Status, This condition applies only if the device is not in OFF mode.
  if (modeState != 2) {
    Blynk.virtualWrite(VPIN_Light, LightState);
    Blynk.virtualWrite(VPIN_Fan, FanState);
  }  
}

// We are using the library AceButton to handle the events of the buttons.
AceButton button1(BUTTON1_PIN);
AceButton button2(BUTTON2_PIN);
// Forward reference to prevent Arduino compiler becoming confused.
void handleEvent(AceButton*, uint8_t, uint8_t);

// Setup Configuration
void setup() {
  delay(1000); // some microcontrollers reboot twice
  // Initialize the serial interface to check the setup initialization.
  Serial.begin(115200);
  while (! Serial);
  Serial.println(F("Begin Setup Initialization ..."));

  // Initialize sensor BME280, if the sensor is not recognized will be a loop until is well connected.
  Wire.begin();
  while (!bme.begin()) {
    Serial.println(F("Could not find a valid BME280 sensor, check wiring, address, sensor ID!"));
    Serial.print(F("SensorID was: 0x")); Serial.println(bme.sensorID(), 16);
    Serial.println("Re-attempt in 10 seconds");
    delay(10000);
  }
  Serial.println("Sensor BME280 ready");
  // Initialize sensor VEML6030, if the sensor is not recognized will be a loop until is well connected.
  while (!light.begin()) {
    Serial.println("Could not communicate with the light sensor!");
    Serial.println("Re-attempt in 10 seconds");
    delay(10000);
  }
  light.setGain(gain);
  light.setIntegTime(_time);
  Serial.println("Sensor VEML6030 ready");
  
  // Initialize Screen, if not works the program can keep working.
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.display();
  delay(2000);
  display.clearDisplay();
  delay(2000);

  // Initialize GPIOs for the relays and buttons
  pinMode(relay, OUTPUT);
  pinMode(relaytwo, OUTPUT);
  Serial.println(F("Setup Relay 1 and 2 as outputs ..."));
  // Initialize in OFF MODE the relays
  digitalWrite(relay, !FanState);
  digitalWrite(relaytwo, !LightState);


  // Buttons use the built-in pull up register.
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  Serial.println(F("Setup Buttons as Input_Pullup ..."));

  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setFeature(ButtonConfig::kFeatureRepeatPress);

  // Setting the initial time Interval to check and send sensor data.
  timer.setInterval(1000L, sendSensor); // Sending Sensor Data to Blynk Cloud every 1 second for demo purposes

  // Initialize Blynk using the Wifi Connection
  // Initialize Wi-Fi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Wifi Connected");
  // Initialize Blynk using Wi-Fi Connection
  Blynk.config(BLYNK_AUTH_TOKEN);
  while (!Blynk.connect()) {
    delay(1000);
    Serial.println("Connecting to Blynk...");
  }
  Serial.println("Blynk Connected");
  delay(1000);
  // Syncronize the relay status with the BLYNK Dashboard  
  getRelayState();
  // Set the thresholds and relay status
  Blynk.virtualWrite(VPIN_Light, LightState);
  Blynk.virtualWrite(VPIN_Fan, FanState);
  Blynk.virtualWrite(VPIN_setTemp, setTemp);
  Blynk.virtualWrite(VPIN_setHumi, setHumi);
  Blynk.virtualWrite(VPIN_setLUX, setLUX);
  Serial.println(F("Setup READY"));
}

// Function to read the sensors and store in variables
void readSensor(){
  // Read the 4 variables
  currentLUX = light.readLight();
  currentPress = bme.readPressure() / 100.0F;
  currentTemp = bme.readTemperature();  
  currentHumi = bme.readHumidity();
  // According to the operation mode
  switch (modeState) {
    case 1:
      currentLUX = light.readLight();
      currentPress = bme.readPressure() / 100.0F;
      currentTemp = bme.readTemperature();  
      currentHumi = bme.readHumidity();

      // Check the temperature Thresholds
      if (currentTemp > setTemp) {
        // If is higher, check if the alertstate is not activated      
        if (!TempAlertState) {
          // Update the alertstate and the variables asociated
          FanState = 1;
          numberAlerts++;
          TempAlertState = 1;
          // Turn on the FAN and send the values to the Blynk Server
          digitalWrite(relay, !FanState);
          pref.putBool("Fan", FanState);
          Blynk.virtualWrite(VPIN_Fan, FanState);          
          Blynk.virtualWrite(VPIN_Temp, TempAlertState);
        }
        // Prevent measure: if someone turn off the fan by error, will turn be on
        if (FanState == 0) {
          FanState = 1;
          digitalWrite(relay, !FanState);
          pref.putBool("Fan", FanState);
          Blynk.virtualWrite(VPIN_Fan, FanState);        
        }
      } else {
        // If the value is below the threshold and the alertstate ON
        // The FAN is OFF mode, and the alert variables are updated      
        if (TempAlertState) {
          FanState = 0;
          TempAlertState = 0;
          numberAlerts--;
          digitalWrite(relay, !FanState);
          pref.putBool("Fan", FanState);
          Blynk.virtualWrite(VPIN_Fan, FanState);
          Blynk.virtualWrite(VPIN_Temp, TempAlertState);
        }
      }
      // Check the humidity threshold
      if (currentHumi > setHumi) {
        // If the value is below the min humidity and the alert state is OFF
        // Update the alert and send a message to show in the dashboard the problem
        // It is possible to replace the action to autmomate another control system (dehumifier)
        if (!HumiAlertState) {
          alerts = "Humidity: Alert";
          HumiState = 1;
          numberAlerts++;
          HumiAlertState = 1;
          pref.putBool("Humidity", HumiState);
          Blynk.virtualWrite(VPIN_Text, alerts);          
        }        
      } else {
        // iF the value is normal again and the alert is ON
        // Update the values and show the message of OK.
        if (HumiAlertState) {
          alerts = "Humidity: OK";
          HumiState = 0;
          pref.putBool("Humidity", HumiState);
          Blynk.virtualWrite(VPIN_Text, alerts);
          numberAlerts--;
          HumiAlertState = 0;
        }
      }
      // If the light is below the limit
      if (currentLUX < setLUX) {
        // If the AlertState is negative
        if (!LUXAlertState) {
          // Turn On The lights
          LightState = 1;
          digitalWrite(relaytwo, !LightState);
          pref.putBool("Light", LightState);
          Blynk.virtualWrite(VPIN_Light, LightState);
          // Change the AlertState and increase the number of alerts
          numberAlerts++;
          LUXAlertState = 1;
          Blynk.virtualWrite(VPIN_LUX, LUXAlertState);
          wrongLUX = currentLUX;
        }
      // If the light is above the limit
      } else {
        // If it the alertstate is positive
        if (LUXAlertState) {
          // Save the new light value considering the lights ON (lastLUX)
          if (flag ==0 ) {
            lastLUX = currentLUX - wrongLUX;
            flag++;
          }
          // After 2 readings, stability, if we want to deactivate the alert the value 
          // The Light intensity should increase enough to turn off the lights.
          if (flag == 1 && ((currentLUX - lastLUX) > setLUX)) {
            LightState = 0;
            numberAlerts--;
            LUXAlertState = 0;
            Blynk.virtualWrite(VPIN_LUX, LUXAlertState);
            lastLUX = 0;
            flag = 0;
            digitalWrite(relaytwo, !LightState);
            pref.putBool("Light", LightState);
            Blynk.virtualWrite(VPIN_Light, LightState);          
          }          
        }
      }
      break;
    // Manual Mode
    case 0:
    // We check the alerts, but not automate the relays
      currentLUX = light.readLight();
      currentPress = bme.readPressure() / 100.0F;
      currentTemp = bme.readTemperature();  
      currentHumi = bme.readHumidity();

      // Check the temperature Thresholds
      if (currentTemp > setTemp) {
        // If is higher, check if the alertstate is not activated      
        if (!TempAlertState) {
          // Update the alertstate and the variables asociated
          numberAlerts++;
          TempAlertState = 1;
          // Just send the alert     
          Blynk.virtualWrite(VPIN_Temp, TempAlertState);
        }
      } else {
        // If the value is below the threshold and the alertstate ON
        // The FAN is OFF mode, and the alert variables are updated      
        if (TempAlertState) {
          TempAlertState = 0;
          numberAlerts--;
          Blynk.virtualWrite(VPIN_Temp, TempAlertState);
        }
      }
      // Check the humidity threshold
      if (currentHumi > setHumi) {
        // If the value is below the min humidity and the alert state is OFF
        // Update the alert and send a message to show in the dashboard the problem
        // It is possible to replace the action to autmomate another control system (dehumifier)
        if (!HumiAlertState) {
          alerts = "Humidity: Alert";
          HumiState = 1;
          numberAlerts++;
          HumiAlertState = 1;
          pref.putBool("Humidity", HumiState);
          Blynk.virtualWrite(VPIN_Text, alerts);          
        }        
      } else {
        // iF the value is normal again and the alert is ON
        // Update the values and show the message of OK.
        if (HumiAlertState) {
          alerts = "Humidity: OK";
          HumiState = 0;
          pref.putBool("Humidity", HumiState);
          Blynk.virtualWrite(VPIN_Text, alerts);
          numberAlerts--;
          HumiAlertState = 0;
        }
      }
      // If the light is below the limit
      if (currentLUX < setLUX) {
        // If the AlertState is negative
        if (!LUXAlertState) {
          // Change the AlertState and increase the number of alerts
          numberAlerts++;
          LUXAlertState = 1;
          Blynk.virtualWrite(VPIN_LUX, LUXAlertState);
          wrongLUX = currentLUX;
        }
      // If the light is above the limit
      } else {
        // If it the alertstate is positive
        if (LUXAlertState) {
          // Save the new light value considering the lights ON (lastLUX)
          if (flag ==0 ) {
            lastLUX = currentLUX - wrongLUX;
            flag++;
          }
          // After 2 readings, stability, if we want to deactivate the alert the value 
          // The Light intensity should increase enough to turn off the lights.
          if (flag == 1 && ((currentLUX - lastLUX) > setLUX)) {
            numberAlerts--;
            LUXAlertState = 0;
            Blynk.virtualWrite(VPIN_LUX, LUXAlertState);
            lastLUX = 0;
            flag = 0;          
          }          
        }
      }
      break;

    case 2:
      // Reset the alerts value
      // Not read the sensors and force the realys to off.
        numberAlerts = 0;
        LUXAlertState = 0;
        HumiAlertState = 0;
        TempAlertState = 0;
        if (LightState == 1 || FanState == 1) {
          LightState = 0;
          digitalWrite(relaytwo, !LightState);
          pref.putBool("Light", LightState);
          Blynk.virtualWrite(VPIN_Light, LightState);
          FanState = 0;
          digitalWrite(relay, !FanState);
          pref.putBool("Fan", FanState);
          Blynk.virtualWrite(VPIN_Fan, FanState);
        }
      break;
  }
}
// Send the values to the Cloud Server
void sendSensor(){
  // Call the function
  readSensor();
  // Update the values in the Blynk Server
  Blynk.virtualWrite(VPIN_Text, alerts);
  Blynk.virtualWrite(VPIN_Alerts, "Sensors Alerted: " + String(numberAlerts));
  Blynk.virtualWrite(VPIN_currentTemp, currentTemp);
  Blynk.virtualWrite(VPIN_currentHumi, currentHumi);
  Blynk.virtualWrite(VPIN_currentLUX, currentLUX);
  Blynk.virtualWrite(VPIN_currentPress, currentPress);
}
// One time function during the setup process to syncronize some variables
void getRelayState(){
  // Operation Mode
  modeState = pref.getBool("Mode", 1);
  Blynk.virtualWrite(VPIN_Mode, modeState);
  delay(200);
  // Relay Status
  LightState = pref.getBool("Light", 0);
  digitalWrite(relaytwo, !LightState);
  Blynk.virtualWrite(VPIN_Light, LightState);
  delay(200);
  FanState = pref.getBool("Fan", 0);
  digitalWrite(relay, !FanState);
  Blynk.virtualWrite(VPIN_Fan, FanState);
  delay(200);
  // Thresholds
  setTemp = pref.getBool("setemp", 0);
  Blynk.virtualWrite(VPIN_setTemp, setTemp);
  delay(200);
  setHumi = pref.getBool("Humidity", 0);
  Blynk.virtualWrite(VPIN_setHumi, setHumi);
  delay(200);
  setLUX = pref.getBool("Light", 0);
  Blynk.virtualWrite(VPIN_setLUX, setLUX);
  delay(200);
}

// Function to display information in the LCD Screen
void DisplayData() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.print("ALR (Lux): ");display.println(currentLUX);
  display.print("Temp ");display.print((char)247); display.print("C: "); display.println(currentTemp);
  display.print("Pres (hPa): ");display.println(currentPress);
  display.print("Hum (%): ");display.println(currentHumi);
  display.display();
}

void loop() {
  // Blynk Connection
  Blynk.run();
  // Timer: each 1 second for demo purposes
  timer.run();
  // Display Data Function
  DisplayData();
  // Check if the button are pressed
  button1.check();
  button2.check();
}

// Function to handle the action of both buttons
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events, for both buttons.
  Serial.print(F("handleEvent(): pin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(AceButton::eventName(eventType));
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);

  // Control the LED only for the Pressed and Released events of Button 1.
  // Notice that if the MCU is rebooted while the button is pressed down, no
  // event is triggered and the LED remains off.
  switch (eventType) {
    case AceButton::kEventReleased:
      // If the button are used, automatically the modeState is MANUAL (0)
      modeState = 0;
      pref.putBool("Mode", modeState);
      Blynk.virtualWrite(VPIN_Mode, modeState);
      // Each time the button is pressed the state of the relay change in order to TURN ON and TURN OFF
      if (button->getPin() == BUTTON1_PIN) {
        digitalWrite(relay, FanState);
        FanState = !FanState;
        pref.putBool("Fan", FanState);
        Blynk.virtualWrite(VPIN_Fan, FanState);        
      } else if (button->getPin() == BUTTON2_PIN) {
        digitalWrite(relaytwo, LightState);
        LightState = !LightState;
        pref.putBool("Light", LightState);
        Blynk.virtualWrite(VPIN_Light, LightState);        
      }
      break;      
  }
}
