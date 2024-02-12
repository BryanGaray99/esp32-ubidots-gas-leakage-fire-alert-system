# Gas and Fire Leak Detection, Monitoring, and Alert System with ESP32 and IoT Integration with Ubidots and Telegram

## 1. Introduction
This project aims to develop a comprehensive system for detecting and alerting gas leaks and fires using ESP32 microcontroller. The system integrates visual and auditory alarms, communication with Telegram for real-time notifications, and data visualization through Ubidots.

## 2. Materials and Resources
### Sensors, output and connections:
- ESP32 microcontroller
- Gas sensor MQ2 (300 - 10000 ppm)
- Flame sensor LM393
- Active Buzzer Module
- High Brightness Led
- Temperature sensor DHT11
- IR transmitter / receiver
- Color Changing Light Buld (With IR control)
- Breadboard and jumper wires
- Power source (e.g., USB cable)

### Digital:
- Arduino IDE
- Libraries (listed in section 4)
- Ubidots Account
- Telegram Bot

## 3. Circuit Diagram, Architecture and Image of the system
![Wiring Diagram](https://github.com/BryanGaray99/esp32-ubidots-gas-leakage-fire-alert-system/blob/main/CircuitDiagram.png?raw=true)

Circuit Diagram

![Wiring Diagram](https://github.com/BryanGaray99/esp32-ubidots-gas-leakage-fire-alert-system/blob/main/Architecture.png?raw=true)

Architecture

![Wiring Diagram](https://github.com/BryanGaray99/esp32-ubidots-gas-leakage-fire-alert-system/blob/main/PhysicalCircuit.png?raw=true)

Image of the system

## 4. Libraries
The project utilizes the following libraries:
```
<Arduino.h>
<WiFi.h>
<WiFiClientSecure.h>
<UniversalTelegramBot.h>
<ArduinoJson.h>
"UbidotsEsp32Mqtt.h"
"DHT.h"
<IRremoteESP8266.h>
<IRsend.h>
```
### Ubidots for ESP32, PubSubClient and Telegram Bot
All the libraries you can install from the Arduino IDE libraries section, except for Ubidots and Telegram libraries. 
- **Installation:**
    1. Download the [esp32-mqtt-main.zip](esp32-mqtt-main.zip) file from the repository.
    2. Download the [pubsubclient-master.zip](pubsubclient-master.zip) file from the repository.
    3. Download the [Universal-Arduino-Telegram-Bot.zip](Universal-Arduino-Telegram-Bot.zip) file from the repository.
    4. In the Arduino IDE, go to File > Preferences, and add the path to the folder containing the downloaded ZIP file to the "Sketchbook location."
    5. Go to Sketch > Include Library > Add .ZIP Library and select the downloaded file.

## 5. Code

The code provided is for an embedded system project utilizing an ESP32 microcontroller for detecting, monitoring, and alerting gas leaks and fires. It integrates with IoT platforms such as Ubidots for data visualization and Telegram for real-time notifications.

### Setup

The setup function initializes various components and establishes connections with WiFi, Telegram, and Ubidots. Here's a breakdown of key points in the setup:

- **WiFi Connection**: The ESP32 connects to the local WiFi network using provided credentials.

- **Telegram Connection**: The system connects to the Telegram API using the provided BOTtoken. It sends a startup message upon successful connection.

- **Ubidots Connection**: Connection to Ubidots is established, and variables for subscribing and publishing data are configured. The system also subscribes to variables related to alarm toggling and inactivity time.

### Main Loop

The loop function continuously monitors sensor data, publishes it to Ubidots, and checks for incoming messages from Telegram. Here's what happens in the main loop:

- **Sensor Data Acquisition**: Temperature, humidity, gas, and flame sensor readings are collected.

- **Publishing Data**: Collected sensor data is published to Ubidots for visualization and monitoring.

- **Inactivity Timer**: An inactivity timer is started upon receiving a value for the "alarms-inactivity-time" variable from Ubidots. If no activity occurs within the specified time, automatic alarms are activated.

- **Alarm Logic**: Depending on sensor readings and user inputs, alarms are turned on or off. If gas or flame is detected, or if manual testing is initiated from Telegram, alarms are activated. Alarms include visual (LED) and auditory (buzzer) alerts, along with Telegram notifications. The color of an IR-controlled light changes to indicate alarm status.

### Alarm Activation Logic

#### Turning On Alarms

When gas or flame is detected, or manual testing is initiated, the alarms are activated. Here's what happens when alarms are turned on:

- **Telegram Notification**: A message is sent to Telegram to alert the user about the detected gas leak or fire.

- **Visual Indicator**: An infrared-controlled light changes color to indicate the alarm status. For example, the color red indicates a gas leak or fire.

- **Auditory Alert**: A buzzer sound is activated to provide an auditory alarm signal.

#### Turning Off Alarms

When the gas or flame is no longer detected, or the user deactivates the alarms manually, the alarms are turned off. Here's what happens when alarms are turned off:

- **Telegram Notification**: A message is sent to Telegram to inform the user that the situation is under control and alarms have been deactivated.

- **Visual Indicator**: The infrared-controlled light returns to its default color (e.g., white) to indicate that the alarm has been cleared.

- **Auditory Alert**: The buzzer sound is turned off.

### Switch Statement in execute_cases Function

The execute_cases function uses a switch statement to determine the action based on the variable received from Ubidots. Here's how the switch statement works:

- **Variable Label Extraction**: The received topic is parsed to extract the variable label.

- **State Assignment**: The variable label is used to determine the state of the system (e.g., inactivity time, toggle alarms, test alarms).

- **Action Execution**: Based on the state, appropriate actions are executed. For example, setting the inactivity timer, toggling alarms, or initiating manual testing.

This switch statement provides a modular and organized approach to handle different variables and their corresponding actions.

## 6. System Operation Modes

The system operates in two main modes:

- **Manual Mode**: Alarms can be manually tested or activated/deactivated through a Unidots Dashboard button.
  
- **Automatic Mode**: Alarms are triggered automatically based on sensor readings. The system adjusts to changes in gas and flame levels and activates alarms as needed.

This system provides a comprehensive solution for gas leak and fire detection, with flexible operation modes and integration with IoT platforms for monitoring and notifications.

## 7. Ubidots Dashboard
![Wiring Diagram](https://github.com/BryanGaray99/esp32-ubidots-gas-leakage-fire-alert-system/blob/main/UbidotsDashboard.png?raw=true)
- Test button for alarms
- Toggle switch for automatic alarms
- Input for disabling alarms for a certain duration (max 60 min)
- Gas presence (MQ2) and flame indicators (On/Off)
- Gauge displaying gas concentration in parts per million (PPM)
- Thermometer
- Historical line chart of temperature and gas concentration

## 8. Project Setup
1. Install all necessary libraries from the Arduino IDE.
2. Install the zip libraries `esp32-mqtt-main.zip`, `pubsubclient-master.zip`, and `Universal-Arduino-Telegram-Bot-master.zip` from the root of this repository.
3. Enter all required credentials of Ubidots and Telegram.
4. Connect the ESP32 according to the circuit diagram.
5. Upload the code to the ESP32 microcontroller.

## Contributions

Contributions are welcome! If you find issues or have suggestions, feel free to open an issue or submit a pull request.

## Contact

For any inquiries, please contact the project maintainer: 
- Bryan Enrique Garay Benavidez
- [bryangarayacademico@gmail.com]