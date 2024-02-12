#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include "UbidotsEsp32Mqtt.h"
#include "DHT.h"
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define AO_MQ2_PIN 35
#define DO_FLAME_PIN 34
#define DHTPIN 25
#define IR_EMITTER 5
#define BUZZER_PIN 18
#define ALARM_LED 19
#define DHTTYPE DHT11

/* Telegram credentials */
#define BOTtoken ":"  // your Bot Token (Get from Botfather)
#define CHAT_ID ""

/* Define the Token provided by the Ubidots platform to make the connection */
const char *UBIDOTS_TOKEN = "";
/* Define SSID and PASSWORD of your WiFi network */
const char *WIFI_SSID = "";
const char *WIFI_PASS = "";
/* Define the name of your device, which will appear on the Ubidots platform */
const char *DEVICE_LABEL = "";

/* Define the variables to be measured and published on the Ubidots platform */
const char *DO_MQ2 = "gas-state";
const char *AO_MQ2 = "gas-measure";
const char *DO_FLAME = "flame-state";
const char *TEMP_LABEL = "temperature";
const char *HUMIDITY_LABEL = "humidity";
const char *ALARMS_INACTIVITY_TIME = "alarms-inactivity-time";
const char *TOGGLE_ALARMS = "toggle-alarm";
const char *TEST_ALARMS = "test-alarms";

const int PUBLISH_FREQUENCY = 1000;
const int gasThreshold = 2000;

const uint8_t NUMBER_OF_VARIABLES = 3;  // Number of variables to subscribe to
char *variable_labels[NUMBER_OF_VARIABLES] = {"alarms-inactivity-time", "toggle-alarm", "test-alarms"}; // Labels of variables to subscribe to
float value;                // To store incoming value
uint8_t variable;           // To keep track of the state machine

int alarmsActive = 1;
int manualAlarm = 0;
static unsigned long CURRENT_INACTIVITY_TIME = 0;
static unsigned long INACTIVITY_TIME = 0;      // Variable to store inactivity time
int TOGGLE_ALARMS_VAR;              // Variable to toggle alarms
int TEST_ALARMS_VAR;
String colorLight = "";
const int ERROR_VALUE = 65535;   // Error value

Ubidots ubidots(UBIDOTS_TOKEN);
DHT dht(DHTPIN, DHTTYPE);
IRsend irsend(IR_EMITTER);  // Set the GPIO to be used to sending the message.
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

void callback(char *topic, byte *payload, unsigned int length)
{
    char *variable_label = (char *)malloc(sizeof(char) * 30);
    get_variable_label_topic(topic, variable_label);
    // Serial.println(variable_label);
    value = btof(payload, length);
    set_state(variable_label);
    execute_cases();
    free(variable_label);
}

// Parse topic to extract the variable label which changed value
void get_variable_label_topic(char *topic, char *variable_label)
{
    Serial.print("topic:");
    Serial.println(topic);
    sprintf(variable_label, "");
    for (int i = 0; i < NUMBER_OF_VARIABLES; i++)
    {
        char *result_lv = strstr(topic, variable_labels[i]);
        if (result_lv != NULL)
        {
            uint8_t len = strlen(result_lv);
            char result[100];
            uint8_t i = 0;
            for (i = 0; i < len - 3; i++)
            {
                result[i] = result_lv[i];
            }
            result[i] = '\0';
            // Serial.print("Label is: ");
            // Serial.println(result);
            sprintf(variable_label, "%s", result);
            break;
        }
    }
}

// Cast from an array of chars to float value
float btof(byte *payload, unsigned int length)
{
    char *demo_ = (char *)malloc(sizeof(char) * 10);
    for (int i = 0; i < length; i++)
    {
        demo_[i] = payload[i];
    }
    return atof(demo_);
}
// State machine to use switch case
void set_state(char *variable_label)
{
    variable = 0;
    for (uint8_t i = 0; i < NUMBER_OF_VARIABLES; i++)
    {
        if (strcmp(variable_label, variable_labels[i]) == 0)
        {
            break;
        }
        variable++;
    }
    if (variable >= NUMBER_OF_VARIABLES)
        variable = ERROR_VALUE; // Not valid
}

// Function with switch case to determine which variable changed and assign the value accordingly to the code variable
void execute_cases()
{
    switch (variable)
    {
    case 0:
        // Serial.println("Tiempo de inactividad recibido:" + String(value));
        INACTIVITY_TIME = value * 60 * 1000;
        CURRENT_INACTIVITY_TIME = millis();
        Serial.println("Inactivity time started:" + String(CURRENT_INACTIVITY_TIME));
        alarmsActive = 0;
        break;
    case 1:
        // Serial.println("value:" + String(value));
        TOGGLE_ALARMS_VAR = value;
        if (TOGGLE_ALARMS_VAR == 1.0) // User activates alarms
        {
          alarmsActive = 1;
        }
        else // User deactivates alarms
        {
          alarmsActive = 0;
        }
        break;
    case 2:
        // Serial.println("value:" + String(value));
        TEST_ALARMS_VAR = value;
        if (TEST_ALARMS_VAR == 1.0) // User activates alarms
        {
          manualAlarm = 1;
        }
        else // User deactivates alarms
        {
          manualAlarm = 0;
        }
        break;        
    case ERROR_VALUE:
        Serial.println("Error");
        Serial.println();
        break;
    default:
        Serial.println("Default");
        Serial.println();
    }
}

/****************************************
Main Functions
****************************************/

void setup() {
  Serial.begin(115200);
  Serial.println("Measurement initiated");
  Serial.println("DHT11 and MQ-2 Turned On");
  irsend.begin();
  dht.begin();
  pinMode(DO_FLAME_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);      
  pinMode(ALARM_LED, OUTPUT);      

  // Telegram connection
  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot started up", "");

  // Ubidots connection
  ubidots.setDebug(true);  // Uncomment this to make debug messages available
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();
  ubidots.add(TOGGLE_ALARMS, alarmsActive);    
  ubidots.publish(DEVICE_LABEL);
  for (uint8_t i = 0; i < NUMBER_OF_VARIABLES; i++)
  {
      ubidots.subscribeLastValue(DEVICE_LABEL, variable_labels[i]); // Insert the device and Variable's Labels
      delay(100);
  }
}

void loop() {
  static unsigned long timer = millis();  // Declare 'timer' as static
  if (!ubidots.connected())
  {
      ubidots.reconnect();
      for (uint8_t i = 0; i < NUMBER_OF_VARIABLES; i++)
      {
          ubidots.subscribeLastValue(DEVICE_LABEL, variable_labels[i]); // Insert the device and Variable's Labels
          delay(100);
      }
  }

  if (abs(static_cast<long>(millis() - timer)) > PUBLISH_FREQUENCY) {
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int analogValue  = analogRead(AO_MQ2_PIN);
    float gasMeasure = (analogValue / 4095.0) * (10000 - 300) + 300;
    int flame_stateSignal = digitalRead(DO_FLAME_PIN);
    int gasState;
    int flameState;

    Serial.print("MQ2 sensor AO value: ");
    Serial.print(gasMeasure);
    Serial.println("\t");

    if (gasMeasure > gasThreshold) {
      gasState = 1;
      Serial.println("The gas is present");
    } else {
      gasState = 0;
      Serial.println("The gas is NOT present");
    }

    if (flame_stateSignal == HIGH) {
      flameState = 0;
      Serial.println("No flame detected => The fire is NOT detected");
    } else if (flame_stateSignal == LOW) {
      flameState = 1;
      Serial.println("Flame detected => The fire is detected");
    }

    ubidots.add(TEMP_LABEL, temperature);
    ubidots.add(HUMIDITY_LABEL, humidity);
    ubidots.add(DO_FLAME, flameState);
    ubidots.add(DO_MQ2, gasState);
    ubidots.add(AO_MQ2, gasMeasure);    

    ubidots.publish(DEVICE_LABEL);

    Serial.println("Sending data to Ubidots: ");
    Serial.println("Temperature: " + String(temperature) + " °C");
    Serial.println("Humidity: " + String(humidity) + " %");
    Serial.println("DO_FLAME_STATE: " + String(flameState));
    Serial.println("DO_MQ2_STATE: " + String(gasState));
    Serial.println("AO_MQ2_MEASURE: " + String(gasMeasure));
    Serial.println("-----------------------------------------");

    // Check if it's time to start the inactivity timer
    if (INACTIVITY_TIME !=0 && abs(static_cast<long>(millis() - CURRENT_INACTIVITY_TIME)) > INACTIVITY_TIME) {
      // If the inactivity timer has been started and the specified time has passed
      alarmsActive = 1;
      INACTIVITY_TIME = 0;
      Serial.println("Activating alarms");
    } 

    if (alarmsActive == 1 && (gasMeasure > gasThreshold || flameState == 1))
    {
      turnOnAlarms();
    }
    else if (manualAlarm == 1) {
      turnOnAlarms();
    }
    else 
    {
      turnOffAlarms();
    }

    timer = millis(); 
  }

  delay(500);
  ubidots.loop();
}

void turnOnAlarms() {
  Serial.println("Turning on alarms");
  if (colorLight != "red")
  {
    bot.sendMessage(CHAT_ID,"\xF0\x9F\x9A\xA8", "");
    bot.sendMessage(CHAT_ID, "Attention! Gas leak or fire detected, please check your place!!!", "");
    colorLight = "red";
    irsend.sendNEC(0xFF6897, 32);  // Send the hex code for the color red
  }
  digitalWrite(BUZZER_PIN, LOW);  // Turn on the buzzer
  digitalWrite(ALARM_LED, HIGH);  // Turn on the LED
  delay(250);
  digitalWrite(BUZZER_PIN, HIGH);  // Turn off the buzzer
  digitalWrite(ALARM_LED, LOW);  // Turn off the LED
  delay(250);
  digitalWrite(BUZZER_PIN, LOW);  // Turn on the buzzer
  digitalWrite(ALARM_LED, HIGH);  // Turn on the LED
}

void turnOffAlarms() {
  Serial.println("Turning off alarms");
  if (colorLight != "white")
  {
    bot.sendMessage(CHAT_ID,"\xE2\x9C\x85", "");
    bot.sendMessage(CHAT_ID, "Situation under control, no gas leak or fire detected. Be more careful!", "");
    colorLight = "white";
    irsend.sendNEC(0xFF52AD, 32);  // Send the hex code for the color white
  }
  digitalWrite(BUZZER_PIN, HIGH);   // Turn off the buzzer
  digitalWrite(ALARM_LED, LOW);  // Turn off the LED
}
