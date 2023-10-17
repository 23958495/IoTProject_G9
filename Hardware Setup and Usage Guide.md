# Smart ShelterGuard System Setup and Usage Guide

## Table of Contents
1. [Introduction and Problem Statement](#introduction-and-problem-statement)
2. [Hardware Setup](#hardware-setup)
   - [List of Materials](#list-of-materials)
   - [Sensors and Relay Connections](#sensors-and-relay-connections)
   - [Button Connections](#button-connections)
   - [Power Supply](#power-supply)
   - [Breadboard Connections](#breadboard-connections)
4. [Software Installation](#software-installation)
   - [Arduino IDE Configuration](#arduino-ide-configuration)
      - [Variables and Initialization](#variables-and-initialization)
      - [Setup Process](#setup-process)
         - [Serial Monitoring](#serial-monitoring)
         - [Sensor Initialization](#sensor-initialization)
         - [Display Initialization](#display-initialization)
         - [Relay and Button Initialization](#relay-and-button-initialization)
         - [Blynk Cloud Platform Integration](#blynk-cloud-platform-integration)
         - [secrets.h File](#secrets.h-file)
         - [Connecting to Wi-Fi and Blynk](#connecting-to-wi-fi-and-blynk)
      - [Loop Process](#loop-process) 
         - [Functions and Timers](#functions-and-timers)
         - [Timer Class](#timer-class)
         - [Data Synchronization](#data-synchronization)
         - [Button Event Handling](#button-event-handling)
   - [BLYNK IoT SERVER](#blynk-iot-server)
5. [System Usage](#system-usage)
   - [Functions and Timers (Continued)](#functions-and-timers-continued)
   - [Button Event Handling (Continued)](#button-event-handling-continued)
   - [BLYNK IoT SERVER (Continued)](#blynk-iot-server-continued)
   - [User Interface](#user-interface)
     - [Dashboard](#dashboard)
     - [Data](#data)

## Introduction and Problem Statement

The Smart ShelterGuard System has been developed to address the pressing issue of high euthanasia rates in animal shelters, particularly in Western Australia. The primary reasons for euthanasia are related to the health and behavior of animals. To mitigate these issues, it is crucial to enhance the physical and mental well-being of sheltered animals by improving their living conditions.

## Hardware Setup

### List of Materials
- LILYGO T-Beam v1.1 board
- BME280 sensor
- VEML6030 light sensor
- 2-Relay module
- Fan control button
- Light control button
- Micro USB cable (for board power)
- 12-volt power source (for fan and light systems)
- Jumper wires (various lengths)
- Breadboards (2)

![T-Beam Diagram](https://github.com/23958495/IoT_Project/blob/fcf4e92642d7b74c89cec5e4d5bd0c04affeaf04/images/tbeam-diagram.jpg)


### Sensors and Relay Connections
- Connect the BME280 sensor to the SDA (Pin 21) and SCL (Pin 22) pins of the LILYGO T-Beam v1.1 board.
- Connect the VEML6030 light sensor to the same SDA and SCL pins.
- Connect the 2-relay module to GPIO ports 4 and 25 for controlling the light and fan systems.

### Button Connections
- Connect the fan control button to pin 13.
- Connect the light control button to pin 14.

### Power Supply
- Power on the LILYGO T-Beam board by connecting it to a laptop through the micro USB port.
- The relay module is energized by 5 volts from the board, while the sensors use 3.3 volts from the board.
- Both the sensors and the relay module share the same ground.
- The fan and light systems require an external power source of 12 volts, connected to the 2-relay module to close the circuit.

### Breadboard Connections
- Use one breadboard to connect the relay and control systems with the board.
- Use another breadboard to interconnect the board with the buttons and sensors.

## Software Installation

The software plays a critical role in the Smart ShelterGuard system, enabling sensor data collection, control systems, and interaction with the BLYNK IoT Cloud platform.

### Arduino IDE Configuration

Use the Arduino IDE to configure the T-Beam board.
Deploy initial libraries to control the hardware and buttons, such as Acebutton.h.
Initialize libraries including SPI.h, Wire.h, Sparkhun.h, Adafruit.h to start reading the sensors.
Configure the Adafruit library for controlling the LCD screen.

### Variables and Initialization

Initialize variables for sensor measurements, button pins, relay pins, and boolean values for relay states, alert states, and messages.

### Setup Process

### Serial Monitoring

Start the serial at a 115200 baud rate to monitor the setup process in real-time.

### Sensor Initialization and Calibration

Begin by initializing the BME280 sensor. If the sensor is not detected initially, the board will attempt detection every 10 seconds until successful. The VEML6030 sensor follows the same process.
- For the VEML6030 light sensor, initialize the Sparkfun_ambient_light variable and specify the I2C address of the board.
- Calibrate the sensor by setting variables for gain, time, and initial luxVal value.
- For the BME280, initialize an Adafruit_BME280 object. Set the initial sea-level pressure to match the location where the sensor is deployed (e.g., Perth).

### Display Initialization

Initialize the screen display to showcase real-time sensor data.

### Sensor Calibration
- For the VEML6030 light sensor, initialize the Sparkfun_ambient_light variable and specify the I2C address of the board.
- Calibrate the sensor by setting variables for gain, time, and initial luxVal value.
- For the BME280, initialize an Adafruit_BME280 object. Set the initial sea-level pressure to match the location where the sensor is deployed (e.g., Perth).

### Blynk Cloud Platform Integration
- Import the necessary libraries, including Preferences.h to store values and BlynkSimpleEsp32 to handle events and interactions with the Blynk cloud platform.
- Connect the ESP32 board to the nearest Wi-Fi access point (AP) modem.
- Map Wi-Fi credentials, Blynk credentials, and virtual pins to facilitate the connection between the board and Blynk.

### secrets.h File
- Create a secrets.h file that uses the pgmspace.h library to optimize SRAM usage.
- Define global variables to handle Wi-Fi and Blynk authentication, as well as virtual pins mapped with Blynk.

### Connecting to Wi-Fi and Blynk
- In the setup function, after initializing other devices, connect to the nearest AP using the stored credentials.
- Establish a connection with the Blynk cloud platform using the global credentials.
- Call the function `getrelaystate()` and use `Blynk.virtualWrite` to synchronize the dashboard with the ESP32.
- Display "Setup Ready" in the serial monitor to confirm that the setup is complete.

By following these steps, you will have successfully set up the software for the Smart ShelterGuard system, connecting your ESP32 board to the Blynk cloud platform and initializing sensors, buttons, and relays for real-time monitoring and control.

### Loop Process

### Functions and Timers
- In the loop function, configure the timer to execute the following functions after a specified duration:
  - `readsensor`: Reads current sensor values, stores them in variables, and manages the operation modes (Automatic, Manual, Off). In Automatic mode, thresholds trigger automatic control system activation. In Manual mode, thresholds trigger alerts but do not control systems. In Off mode, sensors and relays are deactivated.
  - `sendsensor`: Reads sensor values using the `readsensor` function and sends the data to BLYNK.
  - `DisplayData`: Displays environmental variables on the LCD screen.
  - `BLYNK CONNECTED`: Updates the last state of the sensors and synchronizes virtual PINs with the dashboard.
  - `BLYNK WRITE per Virtual PIN`: Reads and writes variables interconnected between the server and the board, including relay statuses and thresholds.
  - `getrelayState`: Synchronizes the relay status between the BLYNK dashboard and the board.

### Timer Class
- Utilize the Timer class to specify the duration and frequency at which sensor data is read and transmitted to the BLYNK app.
- Define timer intervals to control how often sensor data is updated and sent to the BLYNK app.

### Data Synchronization
- Describe how the board uses functions and timers to periodically update sensor data on the BLYNK app in real-time.
- Explain the importance of frequent data synchronization for monitoring and maintaining optimal environmental conditions.

By implementing functions and timers, you ensure that sensor data is consistently and efficiently transmitted to the BLYNK app, providing real-time monitoring of environmental parameters in the shelter.

### Button Event Handling

Create a single function, `HandledEvent()`, to manage button events. The logic within this function switches the operation mode to Manual Mode when a button is pressed. Depending on the current relay states, it controls the lights and fans.

### BLYNK IoT SERVER

In order to deploy the system, create a BLYNK account and choose an appropriate plan. Premium plans offer increased datastreams, widgets, and visualizations, making it an ideal choice. The BLYNK IoT Cloud services provide essential features:

- Storage for up to 6 months of data.
- A User Interface Module for deploying a user-friendly interface accessible on the web and mobile.
- An Automation Module for sending automatic notifications based on sensor values.

Access to the user interface requires permission from the BLYNK administrator on both web and mobile platforms.

# System Usage

### User Interface
- The user interface is user-friendly, available on both mobile and web platforms, consisting of two main windows: "Dashboard" and "Data."

### "Dashboard"
- "Dashboard" provides control options, threshold inputs, and alerts notification.
  - Operation mode selection: Choose from Automatic, Manual, or Off.
  - Controls: Turn the control systems (fans and lights) on/off.
  - Threshold inputs: Modify the threshold values that trigger alerts.
  - Alerts module: Displays the current number of sensors in an alerted state (0 to a maximum of 3) for temperature, humidity, and pressure monitoring.
 
![DashBoard WebApp](https://github.com/23958495/IoT_Project/blob/fcf4e92642d7b74c89cec5e4d5bd0c04affeaf04/images/dashboardui.png)

![DashBoard MobileApp](https://github.com/23958495/IoT_Project/blob/fcf4e92642d7b74c89cec5e4d5bd0c04affeaf04/images/appui.jpg)

### "Data" (Available for Web Interface)
- "Data" offers real-time and historical visualizations of sensor data.
  - View current values and a timeline of sensor readings.
  - Select a time period (latest, last hour, last day, 6 hours, 1 day, 1 week, or 3 months) and zoom in on the timeline.
  - Download data in a customized CSV file for further analysis.
 
![Data WebApp](https://github.com/23958495/IoT_Project/blob/fcf4e92642d7b74c89cec5e4d5bd0c04affeaf04/images/dataui.png)
  
Note: The "Data" window is not available for viewing on mobile devices due to the app's size.

By following these steps, you'll have a comprehensive understanding of the software functions, timer settings, BLYNK IoT Server integration, and user interface usage for the Smart ShelterGuard system.

