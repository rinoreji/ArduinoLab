/*
 * This sketch
 * Reads humidity and temparature from DHT22 sensor,
 * Uses ST7735 display to show it
 * User Ethernet to POST values to remote server
 *
 * Author Rino Reji Cheriyan.
 *
 * Intial source, hardware, requirment from Palnar Transmedia
 * References: Arduino Examples
 */

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include "DHT.h"
#include <Ethernet.h>
#include <Adafruit_NeoPixel.h>

#define sclk 5//5 //13 /5
#define mosi 6//4  //11 /4
#define cs  4 //7
#define dc  7 //9
#define rt  0  // you can also connect this to the Arduino reset
#define DHTPIN 2
#define DHTTYPE DHT22
#define PIN 3
#define NUMPIXELS 6

DHT dht(DHTPIN, DHTTYPE);
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rt); //rst ist schon belegt
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//float humidity; float temperature;
unsigned long nextSensorRead;
unsigned long nextPagePost;
char hum[6]; char temp[6];
int mxh = 0, mxt = 0, mnh = 0, mnt = 0;

// initialize the library instance:
EthernetClient client;
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//byte mac[] = { 0xBA, 0xDD, 0xCA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 224);
char server[] = "192.168.1.3";
//char server[] = "192.168.1.172";
//unsigned int port = 8282;
unsigned int port = 80;
char pageName[] = "/temperature/web/device/upload/";

String serverResponse;
bool hasNewSensorValue = false;

/*
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
*/

void setup() {
  Serial.begin(9600); Serial.println(F("Booting ..."));

  Serial.println(F("Initializing display ..."));
  pixels.begin();
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  tft.setTextWrap(false); // Allow text to run off right edge
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);

  // start the Ethernet connection:
  Serial.println(F("Starting ethernet ..."));
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP, trying to use pre configured IP"));
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }
  // print the Ethernet board/shield's IP address:
  Serial.print(F("localIP address: "));
  Serial.println(Ethernet.localIP());

  nextSensorRead = millis() + 500;
  nextPagePost = millis() + 500;
}

void loop() {
  if (((signed long)(millis() - nextSensorRead)) > 0) {
    //Serial.println(freeRam());

    hasNewSensorValue = true;

    dtostrf(dht.readHumidity(), 4, 1, hum);  //4 is mininum width, 3 is precision;
    dtostrf(dht.readTemperature(), 4, 1, temp);

    char sensorValueBuffer[40];
    sprintf(sensorValueBuffer, "Sensor values: %s - %s", hum, temp);
    Serial.println(sensorValueBuffer);

    //LED control
    if (int(hum) >= mxh) {
      showalarm (3, 100, 100, 0, 1, 1, 1, 20, 5);
    }
    if (int(hum) <= mnh) {
      showalarm (3, 0, 200, 0, 1, 1, 1, 80, 5);
    }
    if (int(temp) >= mxh) {
      showalarm (0, 100, 0, 0, 1, 1, 1, 20, 5);
    }
    if (int(temp) <= mxh) {
      showalarm (0, 0, 0, 200, 1, 1, 1, 80, 5);
    }

    //Display
    tft.fillRect(0,17,tft.width(),tft.height()-17,ST7735_BLACK);
      tft.setRotation(1);
      tft.setCursor(10, 18);
      tft.setTextColor(ST7735_YELLOW);
      tft.setTextSize(6);
      tft.println(hum);
      tft.setTextColor(ST7735_RED);
      tft.setTextSize(6);
      tft.setCursor(10, 69);
      tft.println(temp);
/*      
    tft.fillRect(0, 14, tft.width(), tft.height() - 17, ST7735_BLACK);

    tft.setCursor(10, 15);
    tft.setTextColor(ST7735_YELLOW);
    tft.setTextSize(3);
    tft.println(hum);

    tft.setCursor(10, 40);
    tft.setTextColor(ST7735_RED);
    tft.setTextSize(3);
    tft.println(temp);

    tft.setCursor(0, 68);
    tft.setTextColor(ST7735_BLUE);
    tft.setTextSize(1);
    char diplayBuffer[80];
    sprintf(diplayBuffer, " Server: %s\n\n Mac: %s\n\n LocalIP: %s", server, getMacAddress().c_str(), getIpAddress().c_str());
    tft.println(diplayBuffer);
*/
    nextSensorRead = millis() + 5000;
  }

  if (((signed long)(millis() - nextPagePost)) > 0 && hasNewSensorValue) {
    char postBuffer[80];
    // postBuffer must be url encoded.
    sprintf(postBuffer, "mac=%s&Temperature=%s&Humidity=%s", getMacAddress().c_str(), temp, hum);
    Serial.println(postBuffer);
    if (!postPage(postBuffer)) {
      Serial.print(F("POST failed "));
    }
    else
    {
      hasNewSensorValue = false;
    }
    nextPagePost = millis() + nextPagePost;
  }
}

//Mac address to string (For display purpose)
String getMacAddress() {
  String macAddress;
  for (int i = 0; i < sizeof(mac); i++) {
    macAddress += String(mac[i], HEX);
  }
  return macAddress;
}
/*
//LocalIP address to string (For display purpose)
String getIpAddress() {
  IPAddress address = Ethernet.localIP();
  return String(address[0]) + "." + String(address[1]) + "." + String(address[2]) + "." + String(address[3]);
}
*/
//Connect to a server and do a POST
byte postPage(char* thisData)
{
  Serial.println(F("connecting to remote server..."));

  if (client.connect(server, port) == 1)
  {
    Serial.println(F("connected"));
    char outBuffer[80];
    // send the header
    sprintf(outBuffer, "POST %s HTTP/1.1", pageName);
    client.println(outBuffer);
    sprintf(outBuffer, "Host: %s", server);
    client.println(outBuffer);
    client.println(F("Connection: close\r\nContent-Type: application/x-www-form-urlencoded"));
    sprintf(outBuffer, "Content-Length: %u\r\n", strlen(thisData));
    client.println(outBuffer);
    client.print(thisData);
  }
  else
  {
    Serial.println(F("remote server connection failed"));
    return 0;
  }

  int connectLoop = 0;

  while (client.connected())
  {
    while (client.available())
    {
      char inChar = client.read();
      serverResponse += inChar;
      //Serial.write(inChar);
      connectLoop = 0;
    }

    delay(1);
    connectLoop++;
    if (connectLoop > 10000)
    {
      Serial.println();
      Serial.println(F("Timeout"));
      client.stop();
    }
  }

  nextPagePost = getValue("i", serverResponse);
  Serial.print(nextPagePost);
  /*int startIndex = serverResponse.indexOf('#interval=');
  if (startIndex > 0) {
    int lastIndex = serverResponse.lastIndexOf('#');
    Serial.println(serverResponse.substring(startIndex + 1, lastIndex));
    nextPagePost = serverResponse.substring(startIndex + 1, lastIndex).toInt();
  }*/
  serverResponse = "";
  Serial.println(F("disconnecting."));
  client.stop();
  return 1;
}

int getValue(String key, String data) {
  String indexKey = key + "=";
  int startIndex = data.indexOf(indexKey);
  if (startIndex > -1)
  {
    startIndex = startIndex + indexKey.length();
    String remaningData = data.substring(startIndex);
    int lastIndex = remaningData.indexOf(';');
    if (lastIndex > -1) {
      String value = remaningData.substring(0, lastIndex);
      if (value.length() > 0) {
        return value.toInt();
      }
    }
  }

  Serial.println("Parse error, using fallback value");
  //default fallback values
  if (key == "i")return 5000;
  if (key == "mnt")return 20;
  if (key == "mxt")return 35;
  if (key == "mnh")return 40;
  if (key == "mxh")return 70;
  return 0;
}

int showalarm(int up, int r, int g, int b, int r_off, int g_off, int b_off , int del, int times) {
  //Serial.println();Serial.println("alarm: ");
  // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
  for (int k = 0; k < times; k++) {
    for (int i = 0; i < 3; i++) {
      pixels.setPixelColor(i + up, pixels.Color(r, g, b));
    }
    pixels.show(); delay(del);
    for (int i = 3; i >= 0; i--) {
      pixels.setPixelColor(i + up, pixels.Color(r_off, g_off, b_off));
    }
    pixels.show(); delay(del);
  }
}
