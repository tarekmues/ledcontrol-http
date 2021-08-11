#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>

//217
#define NUM_LEDS 217
#define COLOR_ORDER GRB
#define LED_PIN 0
#define LED_TYPE WS2812

CRGB leds[NUM_LEDS];

ESP8266WebServer server(80);

int state = 0;            //Strip On/Off

double currentred = 0;    //Ist
double currentgreen = 0;
double currentblue = 0;
double newred = 0;        //Soll
double newgreen = 0;
double newblue = 0;
int lastred = 100;        //Letzter Wert, welcher im Hex Code nicht "000000" war
int lastgreen = 100;
int lastblue = 100;

String hexcolor;          // Hex-Code (Kommunikation mit Plugin)
String lastclientip;

const char* ssid = "#";
const char* password = "#";

void setup() {

  //Start Up Sequence
  Serial.begin(74880);
  Serial.println("ESP gestartet");
  delay(100);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  for (int i = 0; i <= 2; i++) {
    leds[0] = CHSV (0, 0, 255);
    delay(400);
    leds[0] = CHSV (0, 0, 0);
    delay(200);
  }
  //start WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Verbindung wird hergestellt...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Verbindung wurde erfolgreich hergestellt");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  if (!MDNS.begin("bedlights")) {
    Serial.println("Couldn't start mDNS Responder!");
    for(int i = 0; i <=NUM_LEDS+1; i++) {
    leds[i].red = 255;
    leds[i].green = 0;
    leds[i].blue = 0;
    FastLED.show();
  }
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  server.begin();
  Serial.println("TCP Server started");
  
  for(int i = 0; i <=NUM_LEDS+1; i++) {
    leds[i].red = 255;
    leds[i].green = 0;
    leds[i].blue = 0;
    FastLED.show();
    delay(1);
  }
  for(int i = 0; i <=NUM_LEDS+1; i++) {
    leds[i].red = 0;
    leds[i].green = 255;
    leds[i].blue = 0;
    FastLED.show();
    delay(1);
  }
  for(int i = 0; i <=NUM_LEDS+1; i++) {
    leds[i].red = 0;
    leds[i].green = 0;
    leds[i].blue = 255;
    FastLED.show();
    delay(1);
  }
  for(int i = 0; i <=NUM_LEDS+1; i++) {
    leds[i].red = 0;
    leds[i].green = 0;
    leds[i].blue = 0;
    FastLED.show();
    delay(1);
  }
  delay(1000);
  for (int t = 0; t < 2; t++) {
    for(int i = 0; i <=NUM_LEDS+1; i++) {
      leds[i].red = 255;
      leds[i].green = 255;
      leds[i].blue = 255;
    }
    FastLED.show();
    delay (20);
    for(int i = 0; i <=NUM_LEDS+1; i++) {
      leds[i].red = 0;
      leds[i].green = 0;
      leds[i].blue = 0;
    }
    FastLED.show();
    delay(150);
  }

  server.onNotFound(handlerequest);
}


void handlerequest() {
  
  String url = server.uri();

  if (currentred != 0 || currentgreen != 0 || currentblue != 0) {
    lastred = int(currentred);
    lastgreen = int(currentgreen);
    lastblue = int(currentblue);
    }

  int slashpos = url.indexOf('/');
  url = url.substring(slashpos+1);

  if (url == "status") {
    server.send(200, "text/plain", String(state));
  }
  else if (url == "on") {
    server.send(200, "text/plain", "");
    if (currentred == 0 && currentgreen == 0 && currentblue == 0) {
      state = 1;
      newred = lastred;
      newgreen = lastgreen;
      newblue = lastblue;
    }
    else {
      state = 1;
      newred = currentred;
      newgreen = currentgreen;
      newblue = currentblue;
    }
    manageTransition();
  }
  else if (url == "off") {
    server.send(200, "text/plain", "");
    state = 0;
    newred = 0;
    newgreen = 0;
    newblue = 0;
    manageTransition();
  }
  else if (url == "rgbstatus") {
    if (state == 0) {
      server.send(200, "text/plain", "000000");
    } else {
      server.send(200, "text/plain", rgbToHex());
    }
  }
  else if (url.length() == 6) {
    server.send(200, "text/plain", "");
    state = 1;
    hexcolor = url;
    newred = hexToRgb(String(hexcolor[0]) + String(hexcolor[1]));
    newgreen = hexToRgb(String(hexcolor[2]) + String(hexcolor[3]));
    newblue = hexToRgb(String(hexcolor[4]) + String(hexcolor[5]));
    manageTransition();
  } else {
    server.send(404, "text/plain", "Not found!");
  }
}


String rgbToHex () {
  byte R = int(currentred);
  byte G = int(currentgreen);
  byte B = int(currentblue);

  char hex[7] = {0};
  sprintf(hex,"%02X%02X%02X",R,G,B);
  return hex;
}

int hexToRgb (String str) {
  int x = str.length();
  int y = 0;
  int z = 0;
  int summe = 0;
  while(y<x) {
           if(str[y] == '0') z = 0;
      else if(str[y] == '1') z = 1;
      else if(str[y] == '2') z = 2;
      else if(str[y] == '3') z = 3;
      else if(str[y] == '4') z = 4;
      else if(str[y] == '5') z = 5;
      else if(str[y] == '6') z = 6;
      else if(str[y] == '7') z = 7;
      else if(str[y] == '8') z = 8;
      else if(str[y] == '9') z = 9;
      else if(str[y] == 'a' || str[y] == 'A') z = 10;
      else if(str[y] == 'b' || str[y] == 'B') z = 11;
      else if(str[y] == 'c' || str[y] == 'C') z = 12;
      else if(str[y] == 'd' || str[y] == 'D') z = 13;
      else if(str[y] == 'e' || str[y] == 'E') z = 14;
      else if(str[y] == 'f' || str[y] == 'F') z = 15;

      z = z * pow(16,x - y - 1);
      summe = summe + z;
      z = 0;
      y++;
  }
  return summe;
}

void manageTransition() {
  
  double averagediff = ((abs(newred - currentred)) + abs((newgreen - currentgreen)) + abs((newblue - currentblue))) / 3;
  int steps = averagediff*0.9;
  if (steps < 30) steps = 30;

  double reddiff   = (newred - currentred) / steps;
  double greendiff = (newgreen - currentgreen) / steps;
  double bluediff  = (newblue - currentblue) / steps;


  for(int k = 1; k <= steps; k++) {
    for(int i = 0; i <= NUM_LEDS; i++) {
      leds[i].red = currentred + reddiff;
      leds[i].green = currentgreen + greendiff;
      leds[i].blue = currentblue + bluediff;
    }
    FastLED.show();
    
    currentred = currentred + reddiff;
    currentgreen = currentgreen + greendiff;
    currentblue = currentblue + bluediff;
  }

  delay(4);
  for(int i = 0; i <= NUM_LEDS; i++) {
      leds[i].red = newred;
      leds[i].green = newgreen;
      leds[i].blue = newblue;
  }
  FastLED.show();
  
  currentred = newred;
  currentgreen = newgreen;
  currentblue = newblue;  
}




void loop () {
  MDNS.update();
  server.handleClient();
  delay(5);
}
