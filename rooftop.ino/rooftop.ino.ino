#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <inttypes.h>
#include <Wire.h>
 
#include <lm75.h>
 
TempI2C_LM75 termo = TempI2C_LM75(0x50,TempI2C_LM75::nine_bits);
 
 

const char *ssid = "scoltock";
const char *password = "nowireshere";

ESP8266WebServer server ( 80 );

const int led = 13;

void handleRoot() {
  digitalWrite ( led, 1 );
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf ( temp, 400,

"<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

    hr, min % 60, sec % 60
  );
  server.send ( 200, "text/html", temp );
  digitalWrite ( led, 0 );
}

void handleNotFound() {
  digitalWrite ( led, 1 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  digitalWrite ( led, 0 );
}





void setup ( void ) {

 pinMode ( led, OUTPUT );
  digitalWrite ( led, 0 );
  Serial.begin ( 115200 );
  WiFi.begin ( ssid, password );
  Serial.println ( "" );
  
  for(int address = 0x48;address < 0x50; address++)
    getTemp(address);
return;
  
 

  // Wait for connection
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  Serial.println ( "" );
  Serial.print ( "Connected to " );
  Serial.println ( ssid );
  Serial.print ( "IP address: " );
  Serial.println ( WiFi.localIP() );




  server.on ( "/", handleRoot );
  server.on ( "/test.svg", drawGraph );
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );
  server.begin();
  Serial.println ( "HTTP server started" );
}

void loop ( void ) {
  server.handleClient();
}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x+= 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send ( 200, "image/svg+xml", out);
}

void getTemp(int address) {
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

const int LM75A_BASE_ADDRESS = 0x48;

const float LM75A_DEGREES_RESOLUTION = 0.125;


  if (refactored_value & 0x0400) {
    // When sign bit is set, set upper unused bits, then 2's complement
    refactored_value |= 0xf800;
    refactored_value = ~refactored_value + 1;
    real_value = (float)refactored_value * (-1) * LM75A_DEGREES_RESOLUTION;
  }
  else {
    real_value = (float)refactored_value *  LM75A_DEGREES_RESOLUTION;
  }}}

  Serial.print(address);
  Serial.print(":");
  Serial.print(real_value);
  Serial.println("oC");
    
}


