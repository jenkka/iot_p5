/*************************************************************************************************
 * This Example sends  harcoded data to Ubidots and serves as example for users which 
 * desire build a HTTP request to the Ubidots API with any device. Please reference to the link below 
 * to find more information about it - Ubidots REST API Reference - https://ubidots.com/docs/api/
 *
 * You will find a help guide for this code on the link below: 
 * http://help.ubidots.com/connect-your-devices/connect-your-intel-edison-to-ubidots-using-arduino-over-http
 * 
 * This example is given AS IT IS without any warranty.
 *  
 * Made by María Carlina Hernández.
 *************************************************************************************************/

/********************************
 * Libraries included
 *******************************/
#include <WiFi.h>
#include <SPI.h>
#include "DHT.h"

#define pin_sensor 13 
#define pin_led 15

int led_on = 0;

char str_temp[10];
char str_hum[10];
DHT dht1(pin_sensor, DHT11);

/********************************
 * Constants and objects
 *******************************/
/* Assigns the network parameters */
char* WIFI_SSID = "IoT"; // your network WIFI_SSID (name)
char* WIFI_PASSWORD = "1t3s0IoT18"; // your network password

/* Assigns the Ubidots parameters */
char const * TOKEN = "BBFF-p7zAoGwCrnSgZ4Ts8Mn4gcsQYuqCUN"; // Assign your Ubidots TOKEN
char const * DEVICE_LABEL = "esp32"; // Assign the unique device label 
char const * VARIABLE_LABEL_1 = "temperatura"; // Assign the unique variable label to publish data to Ubidots (1) 
char const * VARIABLE_LABEL_2 = "humedad"; // Assign the unique variable label to publish data to Ubidots (2)

/* Parameters needed for the requests */
char const * USER_AGENT = "TEAM4";
char const * VERSION = "1.0";
//char const * SERVER = "things.ubidots.com";
char const * SERVER = "industrial.api.ubidots.com";
int PORT = 80;
char topic[700];
char payload[300];

/* initialize the library instance */
WiFiClient client;

/********************************
 * Auxiliar Functions
 *******************************/

void SendToUbidots(char* payload) {

  int i = strlen(payload); 
  /* Builds the request POST - Please reference this link to know all the request's structures https://ubidots.com/docs/api/ */
  sprintf(topic, "POST /api/v1.6/devices/%s/?force=true HTTP/1.1\r\n", DEVICE_LABEL);
  sprintf(topic, "%sHost: things.ubidots.com\r\n", topic);
  sprintf(topic, "%sUser-Agent: %s/%s\r\n", topic, USER_AGENT, VERSION);
  sprintf(topic, "%sX-Auth-Token: %s\r\n", topic, TOKEN);
  sprintf(topic, "%sConnection: close\r\n", topic);
  sprintf(topic, "%sContent-Type: application/json\r\n", topic);
  sprintf(topic, "%sContent-Length: %d\r\n\r\n", topic, i);
  sprintf(topic, "%s%s\r\n", topic, payload);
  
  /* Connecting the client */
  client.connect(SERVER, PORT); 

  if (client.connected()) {
    /* Sends the request to the client */
    client.print(topic);
    Serial.println("Connected to Ubidots - POST");
  } else {
    Serial.println("Connection Failed ubidots - Try Again"); 
  }  
    
  /* Reads the response from the server */
  while (client.available()) {
    char c = client.read();
    //Serial.print(c); // Uncomment this line to visualize the response on the Serial Monitor
  }

  /* Disconnecting the client */
  client.stop();
}

/********************************
 * Main Functions
 *******************************/
 
void setup() {
  Serial.begin(115200);
  pinMode(pin_led, OUTPUT);
  digitalWrite(pin_led, LOW);
  
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID); 
  /* Connects to AP */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
    
  Serial.println(F("WiFi connected"));
  Serial.println(F("IP address: "));
  Serial.println(WiFi.localIP());

  dht1.begin();
}


void loop() {
  /* Reads sensors values */
  float temperature = dht1.readTemperature();
  dtostrf(temperature, 4, 2, str_temp);
  
  float humidity = dht1.readHumidity();
  dtostrf(humidity, 4, 2, str_hum);

  /* Builds the payload - {"temperature":25.00,"humidity":50.00} */
  sprintf(payload, "{\"");
  sprintf(payload, "%s%s\":%f", payload, VARIABLE_LABEL_1, temperature);
  sprintf(payload, "%s,\"%s\":%f", payload, VARIABLE_LABEL_2, humidity);

  /* Verificamos temperatura para pender el pin_led */
  if (led_on == 0 && temperature > 28) {
    led_on = 1;  
    digitalWrite(pin_led, HIGH);
  } else if (led_on == 1 && temperature < 25) {
    led_on = 0;
    digitalWrite(pin_led, LOW);
  }
  
  sprintf(payload, "%s,\"%s\":%d", payload, "LED", led_on);
  sprintf(payload, "%s}", payload);
  
  /* Calls the Ubidots Function POST */
  SendToUbidots(payload);
  /* Prints the data posted on the Serial Monitor */
  Serial.println("Posting data to Ubidots");
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Humidity: ");
  Serial.println(humidity);

  delay(5000); 
}
