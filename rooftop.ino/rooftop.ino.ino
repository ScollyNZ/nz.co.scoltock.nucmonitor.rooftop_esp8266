#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <inttypes.h>
#include <Wire.h>

const char *ssid = "scoltock";
const char *password = "nowireshere";
const int led = D0;
const char *influxURL = "http://bees.scoltock.co.nz:8086/write?db=bees";
int loopCount = 0;

void setup ( void ) {

  pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );
  Serial.begin ( 115200 );
  WiFi.begin ( ssid, password );
  Serial.println ( "" );

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
    digitalWrite(led, !digitalRead (led));  //Flash LED while WiFi is not established
  }

  digitalWrite(led, 0); //Ensure LED is off, now that WiFi is established

  printWiFiInfo();
}

void loop ( void ) {
  String postData="temp,hive=1 ambient=";
  float temp;
  
  for(int address = 0x48;address < 0x51; address++) {
    temp=getTemp(address);
    if (address>0x48) {
      postData+=",";
      postData+=address;
      postData+="=";
    }

    postData+=temp;
  }

  Serial.print(postData);

  insertData(postData);

  if (loopCount=4){
    loopCount = 0;
    reportRSSI();
    reportBatteryLevel();
  }

  delay(15*60*1000);
}

float getTemp(int address) {
  const int LM75A_BASE_ADDRESS = 0x48;
  const float LM75A_DEGREES_RESOLUTION = 0.125;
  float real_value=0;
  
  Wire.begin(D1, D2);

  const int LM75A_REG_ADDR_TEMP = 0;
  // Go to temperature data register
  Wire.beginTransmission(address);
  Wire.write(LM75A_REG_ADDR_TEMP);
  if(!Wire.endTransmission()) {
    uint16_t i2c_received = 0;
  
    // Get content
    if (Wire.requestFrom(address, 2)) {
      Wire.readBytes((uint8_t*)&i2c_received, 2);
      // Modify the value (only 11 MSB are relevant if swapped)
      int16_t refactored_value;
      uint8_t* ptr = (uint8_t*)&refactored_value;

      // Swap bytes
      *ptr = *((uint8_t*)&i2c_received + 1);
      *(ptr + 1) = *(uint8_t*)&i2c_received;

      // Shift data (left-aligned)
      refactored_value >>= 5;
      if (refactored_value & 0x0400) {
        // When sign bit is set, set upper unused bits, then 2's complement
        refactored_value |= 0xf800;
        refactored_value = ~refactored_value + 1;
        real_value = (float)refactored_value * (-1) * LM75A_DEGREES_RESOLUTION;
      }
      else {
        real_value = (float)refactored_value *  LM75A_DEGREES_RESOLUTION;
      }
    }
  }

  Serial.print(address);
  Serial.print(":");
  Serial.print(real_value);
  Serial.println("oC");

  return real_value;
    
}

void reportRSSI()
{
  String postData="rssi,hive=1 value=";
  postData+=WiFi.RSSI();
 
  insertData(postData);
}

void reportBatteryLevel()
{
  String postData="battery_voltage,hive=1 value=";
  postData+=analogRead(A0);
 
  insertData(postData); 
}

void insertData(String postData)
{
  HTTPClient http;
  http.begin(influxURL);
  int httpCode = http.POST(postData);
  String payload = http.getString();
  http.end();
  Serial.print(payload);
}

void printWiFiInfo(){
  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );
}

