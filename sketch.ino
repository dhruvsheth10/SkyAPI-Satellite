#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define TFT_CS     5
#define TFT_DC     4
#define TFT_RST    2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
const char* ssid = "Wokwi-GUEST";
const char* password = "";

bool firstscan = true;

void setup() {
  Serial.begin(115200);
  tft.begin();           
  tft.setRotation(1); 
  tft.fillScreen(ILI9341_BLACK);
  
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(1);
  tft.setCursor(10, 10);
  tft.println("Initiating Satellite Connection");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  tft.fillScreen(ILI9341_BLACK);
}
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    if (firstscan) {
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(10, 10);
      tft.setTextColor(ILI9341_CYAN);
      tft.setTextSize(2);
      tft.println("scanning the sky...");
      delay(500);
    }

    String apiurl = "https://dhruvsheth.hackclub.app/planesabove?lat=37.51656&lon=-121.920812&miles=15"; //<- put the coords above u
    http.begin(apiurl);
    int httpresp = http.GET();
    
    if (httpresp > 0) {
      String payload = http.getString();
      Serial.println(payload); 
      
      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, payload);

      
      if (!error) {
        tft.fillScreen(ILI9341_BLACK);
        
        JsonArray planes = doc["planes"];
        if (planes.size() > 0) {
          int yoffset = 10; 
          
         
          for (int i = 0; i < min((int)planes.size(), 4); i++) { //<- capped at 4 for now
            JsonObject plane = planes[i];
            
            const char* aircraft = plane["Aircraft"];
            int altitude = plane["altitude (feet)"];
            float distance = plane["distance from you (miles)"];
            const char* headingStr = plane["heading"].as<const char*>();
            
            const char* origin = plane["origin"].as<const char*>();
            if (!origin) origin = "???";

            const char* dest = plane["destination"].as<const char*>();
            if (!dest) dest = "???";

            // const char* arrow = "->"; 
            // if (headingStr != NULL) {
            //   String h = String(headingStr);
            //   h.trim();
            //   arrow = h
            //   // if (h == "N")   arrow = "↑";
            //   // if (h == "NE")  arrow = "↗";
            //   // if (h == "E")   arrow = "→";
            //   // if (h == "SE")  arrow = "↘";
            //   // if (h == "S")   arrow = "↓";
            //   // if (h == "SW")  arrow = "↙";
            //   // if (h == "W")   arrow = "←";
            //   // if (h == "NW")  arrow = "↖";
            // }

            tft.setCursor(10, yoffset);
            tft.setTextColor(ILI9341_WHITE);
            tft.setTextSize(2);
            tft.printf("%s", aircraft);
            
            tft.setCursor(10, yoffset + 18); 
            tft.setTextColor(ILI9341_MAGENTA);
            tft.setTextSize(2); 
            tft.printf("%s -> %s", origin, dest);
            
            tft.setCursor(10, yoffset + 36); 
            tft.setTextColor(ILI9341_YELLOW);
            tft.setTextSize(1.2);
            tft.printf("ALT: %d ft | DIST: %.1f mi | DIR: %s", altitude, distance, headingStr);
        
            yoffset += 58; 
          }
        } else {
          tft.setCursor(10, 60);
          tft.setTextColor(ILI9341_GREEN);
          tft.setTextSize(2);
          tft.println("clear skies above");
        }
        
        firstscan = false;
        
      } else {
        tft.setCursor(10, 10);
        tft.setTextSize(1);
        tft.println("json parse error");
      }
    } else {
      tft.setCursor(10, 10);
      tft.setTextColor(ILI9341_RED);
      tft.setTextSize(1);
      tft.printf("http connection error: %d", httpresp);
    }
    http.end();
  }
  
  delay(30000); //refresh rate for pinging api
}