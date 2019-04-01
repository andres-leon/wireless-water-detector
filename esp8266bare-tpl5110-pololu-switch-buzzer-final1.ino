/*
 * Water detector: By andres leon
 * version tracking since ver 10
 * ver 10: updated strDateTime() function to request time if year is 1970.
 * ver 11: removed or reduced delay in sections to speed up the whole process.
 * ver 12: added funtion send_sensor_battery_data_mqtt. only sends to one mqtt topic all needed data
 * ver 13: adjusted map values in battery_level_percent function
 * With TPL5110: forget about reseting. let the TPL5110 take care of wakign up and shutting down.
 *                need to set a pin high to let the tpl5110 know when to shutdown. 
 *                see https://learn.adafruit.com/adafruit-tpl5110-power-timer-breakout/usage
 * With Buzzer: adds functionality to trigger buzzer when water is detected
 * Not checking bat voltage anymore
 * ver 4: added define for serila print
 * ver 5: reading voltage from battery. code from https://learn.adafruit.com/using-ifttt-with-adafruit-io/wiring#battery-tracking
 */
  
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h> 

// the current address in the EEPROM (i.e. which byte
// we're going to read from)
int addr_EEPROM = 0;

/* ----------- UPDATE SENSOR NAME ---------------------- */
#define mqtt_sensor_status_topic "watersensor/status"
const char* deviceName = "watersensor";
 /* ----------- SENSOR NAME ---------------------- */

/************************* WiFi Access Point *********************************/
const char* ssid     = "your SSID";
const char* password = "your password";
/************************* MQTT Connection *********************************/
#define mqtt_server "your mqtt server ip address"
#define mqtt_user "your mqtt username"
#define mqtt_password "your mqtt password"
 
//https://arduino.stackexchange.com/questions/9857/can-i-make-the-arduino-ignore-serial-print
//to deactivate serail prints, replace the defines with
//#define Sprintln(a) //nothing else
//#define Sprint(a)
#define Sprintln(a) (Serial.println(a))   
#define Sprint(a) (Serial.print(a))

#define WATER_IO_PIN 13
#define DONEPIN 5   //notifies the tpl5110 when it code is done and is time to shutdown.
#define BUZZER_PIN 15
#define mqtt_max_connect_count 5          //max tries to send mqtt message
#define wifi_max_connect_count 10         //max tries to send wifi message
#define rest_between_attempts_milli 750   //wait time between attempts
#define counter_limit_wakeup 1           //when this num is reached, then connect to wifi and send notification. 
#define BUZZER_BEEP_TIME 5000             //milliseconds buzzer should beep

WiFiClient espClient;
PubSubClient mqtt_client(espClient);

// function prototypes
void battery_level();
bool water_sensor();
bool send_sensor_battery_data_mqtt();
void shutdownMsg();
void setupWIFI();

int batt_level;
int batt_level_percent;
String ipaddress;
bool prev_water_sensor;
bool curr_water_sensor;
int wakeup_counter;

/* <<<<<<<<<<<<<<<<<< BEGIN SETUP >>>>>>>>>>>>>>>>>  */
void setup() {  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(WATER_IO_PIN, INPUT_PULLUP);
  pinMode(DONEPIN, OUTPUT);  
  //make sure shutdown signal is off!  
  digitalWrite(DONEPIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
    
  Serial.begin(115200);  

  Sprintln("");
  //Sprintln("=========================================");
  Sprintln("STARTING esp8266_water-detector-tpl5111-06-voltageread-buzzer");  
  Sprintln(mqtt_sensor_status_topic);
  //Sprintln("=========================================");
  
  EEPROM.begin(512);  //Initialize EEPROM

  int prev_water_sensor_EEPROM_addr = addr_EEPROM;
  int wakeup_counter_EEPROM_addr = addr_EEPROM + 1;
  
  prev_water_sensor = (bool)EEPROM.read( prev_water_sensor_EEPROM_addr );
  curr_water_sensor = water_sensor();
  wakeup_counter = (int)EEPROM.read( wakeup_counter_EEPROM_addr );

  Sprint("prev_water_sensor = ");
  Sprintln(prev_water_sensor);
  Sprint("curr_water_sensor = ");
  Sprintln(curr_water_sensor);
  Sprint("wakeup_counter = ");
  Sprintln(wakeup_counter);
  
  if (  (curr_water_sensor) ||                        /* if WET */
        (curr_water_sensor != prev_water_sensor) ||   /* or status has changed */
        (wakeup_counter >= counter_limit_wakeup)  )   /* or timeout reached */
  {
    if (curr_water_sensor)
      digitalWrite(BUZZER_PIN, HIGH);
      
    setupWIFI();
    if (send_sensor_battery_data_mqtt()) //only update if message sent successfully.
    {
      if (curr_water_sensor != prev_water_sensor)   /* status has changed */  
        EEPROM.write(prev_water_sensor_EEPROM_addr, (bool)curr_water_sensor);
    }
    //WiFi.disconnect(); 
    wakeup_counter = 0;   //reset counter. 
    
  }
  
  wakeup_counter ++;
  EEPROM.write(wakeup_counter_EEPROM_addr, wakeup_counter);
    
  Sprint("saving to EEPROM new water sensor and wakeup counter: "); 
  EEPROM.commit();

  delay(500); //added delay to ensure the mqtt message is sent.
}
/* <<<<<<<<<<<<<<<<<< END SETUP >>>>>>>>>>>>>>>>>  */
void loop() 
{
  if (water_sensor())
  {
    digitalWrite(BUZZER_PIN, HIGH);
    //delay( BUZZER_BEEP_TIME );
  }
  else
    shutdownMsg();
}

void shutdownMsg()
{
  Sprint("Shutting down!");
  digitalWrite(LED_BUILTIN, HIGH);
  while (1) {
    digitalWrite(DONEPIN, HIGH);
    delay(1);
    pinMode(DONEPIN, INPUT);  
//    digitalWrite(DONEPIN, LOW);
//    delay(1);
  }  
}

void setupWIFI()
{
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  
  int wifi_connect_attempt_count = 0;
  Sprint("connecting to ");
  Sprintln( ssid );  
  /*   setting static IP address   */
  /*
  // config static IP
  IPAddress ip(192, 168, 1, 205); // where xx is the desired IP Address
  IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);
  IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
  WiFi.config(ip, gateway, subnet);
  */
  /*   setting static IP address   */
  //WiFi.hostname(deviceName);      // DHCP Hostname (useful for finding device for static lease)
  //WiFi.config(staticIP, subnet, gateway, dns);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && ( wifi_connect_attempt_count < wifi_max_connect_count ) ) 
  {
    delay(rest_between_attempts_milli);      
    Sprint(wifi_connect_attempt_count);
    Sprint(".. ");
    wifi_connect_attempt_count++;      
  }
  Sprintln("");
  if ( wifi_connect_attempt_count >= wifi_max_connect_count )
  {
    Sprint("Failed to connect to wifi. Shutting down.");
    shutdownMsg();
  }  
  Sprint("Connected to ");
  Sprintln( ssid );
  Sprint("IP address: ");
  ipaddress = (String)WiFi.localIP().toString();
  Sprintln(ipaddress);
  Sprintln("");
  //WiFi.printDiag(Serial);   //wifi diagnostics...
}

bool send_sensor_battery_data_mqtt()
{
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  bool retValue = 0;
  if (curr_water_sensor){
    root["status"] = "WET";
  }
  else {
    root["status"] = "DRY";
  }

  if (prev_water_sensor){
    root["previous_status"] = "WET";
  }
  else {
    root["previous_status"] = "DRY";
  }
  
  root["IP"] = (String)WiFi.localIP().toString();

  //root["status_date"] = strAcquiredDateTime;
  battery_level_percent(); 
  //float vc1 = (ESP.getVcc()/10000.0)/2;  //get voltage
  root["batt_per"] = (String)batt_level_percent;
  root["batt_vol"] = (String)batt_level;
  //root["batt_per"] = "0";
  //root["batt_vol"] = "0";

  String payload;
  root.printTo(payload);  
  Sprint("sending ");
  Sprintln(payload);
  return send_mqtt_msg(mqtt_sensor_status_topic, payload); 
  
}

bool send_mqtt_msg(String topic, String payload) 
{
  //Serial.println(topic);
  int mqtt_connect_count = 1; 
  int mqtt_pub_count = 0;
  int mqtt_pub_count_limit = 3;
    
  mqtt_client.setServer(mqtt_server, 1883);
  Sprintln("Attempting MQTT connection... ");
  // Loop until we're reconnected or attempt limit has been reached
  while (!mqtt_client.connected() && (mqtt_connect_count <= mqtt_max_connect_count)) 
  {
    Sprint(mqtt_connect_count); Sprint(". ");
    mqtt_client.connect(deviceName, mqtt_user, mqtt_password);
    mqtt_connect_count++;
    delay(rest_between_attempts_milli);
  }
  Sprintln("");
  if ( mqtt_connect_count >= mqtt_max_connect_count ) 
  { //failed to connect to MQTT server
    Sprint("failed to connect to MQTT server. rc= ");
    Sprintln(mqtt_client.state());
    return false;
  } 
  else 
  {
    Sprintln("connected to MQTT server.");
    //mqtt_client.loop();
    Sprint("sending message to topic: ");
    Sprintln(topic);
    Sprint("Payload: ");
    Sprintln(payload);
    bool mqttMsgSent = 0;
    
    while (!mqttMsgSent && ( mqtt_pub_count < mqtt_pub_count_limit ) ) 
    {
      Sprint(mqtt_pub_count);
      Sprintln("..!.. ");
      mqttMsgSent = mqtt_client.publish(topic.c_str(), payload.c_str(), true);
      if ( mqttMsgSent )
        break;
      else
        delay(rest_between_attempts_milli);
    } 
    if (mqtt_pub_count <= mqtt_pub_count_limit )
    {
      Sprintln(F("MQTT SENT!"));
      return true;
    }
  }
  return true;
}


bool water_sensor(){
  return digitalRead(WATER_IO_PIN) == LOW;
}

void battery_level_percent() {
  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 1M & 220K voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  
  batt_level = analogRead(A0);
  //Sprint("analog raw value of voltage: ");
  //Sprintln(level);

  // convert battery level to percent
  batt_level_percent = map(batt_level, 580, 774, 0, 100);
  //Serial.print("mapped Battery level: "); Serial.println(level); 
  
}
