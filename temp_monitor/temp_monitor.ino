#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include "DHT.h"       

#define DHTTYPE DHT11  

#define dht_dpin D3
DHT dht(dht_dpin, DHTTYPE); 

//Global Variables
const char *ssid     = "SSID_HERE";
const char *password = "PASSWORD_HERE";
int TIMES = 3600; //3600 secs = 1hr
//int EPOCH;

int hours;
int mins;

float avg_temp=-10000;
float avg_humid=-10000;

//Thingspeak Config
WiFiClient client;
int thingspeak_channel_no = 947332;
const char * thingspeak_api_key = "5T17L054I5GRQHC0";
int temp_field = 1;
int humid_field = 2;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800); //19800 for timezone GMT +05:30 (IST)

void setup() {
  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  Serial.println("Welcome to NodeMCU Weather Monitor! Connecting to Wifi");
  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay ( 1500 );
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print ( "." );
  }
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Connected to Wifi! Initializing Thingspeak and NTP Client");
  ThingSpeak.begin(client);
  timeClient.begin();

  //ThingSpeak.setField(2, 88);
  //ThingSpeak.setCreatedAt("2019-30-30T16:00:13"); 
  //ThingSpeak.writeFields(thingspeak_channel_no, thingspeak_api_key);
}

void get_temperature() {
  int counter = 0;
  avg_temp = 0;
  avg_humid = 0;
  while (counter < TIMES) {
    float temp_humid = dht.readHumidity();
    avg_humid = avg_humid + temp_humid;
    float temp_tmp = dht.readTemperature();
    avg_temp = avg_temp + temp_tmp;
    delay(900);
    counter++;
  }

  avg_temp = avg_temp/TIMES;
  avg_humid = avg_humid/TIMES;
  
 
  Serial.print("Average Temperature:");
  Serial.println(avg_temp);

  Serial.print("Average Humidity:");
  Serial.println(avg_humid);

  //Serial.println("Counter:");
  //Serial.print(counter);

  timeClient.update();
  mins = timeClient.getMinutes();
  hours = timeClient.getHours();

  Serial.print(hours);
  Serial.print(":");
  Serial.println(mins);
}

void upload_to_thingspeak(){
  Serial.println("Uploading data to Thingspeak");
  ThingSpeak.setField(temp_field, avg_temp);
  ThingSpeak.setField(humid_field, avg_humid);
  //ThingSpeak.setCreatedAt(String(EPOCH));
  ThingSpeak.writeFields(thingspeak_channel_no, thingspeak_api_key);
  Serial.println("Uploaded data to Thingspeak");
}

void check_time() {
  Serial.println("Checking Time");
  timeClient.update();
  mins = timeClient.getMinutes();
  hours = timeClient.getHours();

  Serial.println(mins); 
  if ((mins == 0) || (mins == 1)){
    if (avg_temp != -10000){
      upload_to_thingspeak();   
    }
    Serial.println("Time starts now. Going to start measuring temperature");
    //EPOCH = timeClient.getEpochTime();
    //EPOCH = EPOCH + TIMES;
    //Serial.print("EPOCH is: ");
    //Serial.println(String(EPOCH));
    get_temperature();
  }
  else {
    Serial.println("Waiting for some more time to start measuring");
    delay(59000);
  }
}

void loop() {
  check_time();
}
