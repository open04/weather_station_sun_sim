#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h> 
#include <WiFiClientSecure.h>
#include <AutoConnect.h>
#include <time.h>
#include <coredecls.h>
#include "OpenMeteoOneCall.h"

#include "page.h"

ESP8266WebServer Server;
AutoConnect Portal(Server);
AutoConnectConfig Config;
OpenWeatherMapOneCallData weatherData;
OpenWeatherMapOneCall oneCallClient;

const char* host = "api.open-meteo.com";
const int httpsPort = 443;
const float latitude = 14.2638;
const float longitude = 120.9130;
String language = "en";
const bool isMetric = true;

bool synced = false;
String weather[7];
String dowFc[7];
int rainProb[7]; 
String willRain[7];

const char* daysOfWeek[] = {
    "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY",
    "THURSDAY", "FRIDAY", "SATURDAY"
  };

String data = "";

void handleRoot() {
  Server.send(200, "text/html; charset=utf-8", html);
}

void handleSync() {
  syncTime();
  bool forecastSuccess = syncForecast();

  String message = forecastSuccess ? "✅ Forecast synced successfully." : "⚠️ Forecast sync failed.";
  
  String html = "<html><head><meta http-equiv='refresh' content='3; url=/'/>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; background-color: #f9f9f9; }";
  html += "h2 { font-size: 32px; color: #333; }";
  html += "p { font-size: 20px; color: #666; }";
  html += "</style></head><body>";
  html += "<h2>" + message + "</h2>";
  html += "<p>Redirecting back...</p>";
  html += "</body></html>";

  Server.send(200, "text/html; charset=utf-8", html);
}

bool syncForecast() {
  oneCallClient.update(&weatherData, "", latitude, longitude); // API key not needed

  for (int i = 0; i < 7; i++) {
    time_t t = weatherData.daily[i].dt;
    struct tm* tm_fc = localtime(&t);

    dowFc[i] = daysOfWeek[tm_fc->tm_wday];
    weather[i] = weatherData.daily[i].weatherIconMeteoCon;
    rainProb[i] = weatherData.daily[i].rain_prob;

    if (rainProb[i] > 50) {
      willRain[i] = "YES";
    } else {
      willRain[i] = "NO";
    }
  }

  bool rainPeriodActive = false;
  time_t rainStart = 0;
  time_t rainEnd = 0;

  time_t now = time(nullptr);
  struct tm* now_tm = localtime(&now);
  for (int i = 0; i < 24; i++) {
    time_t t = weatherData.hourly[i].dt;
    struct tm* forecast_tm = localtime(&t);

    if (forecast_tm->tm_mday != now_tm->tm_mday ||
        forecast_tm->tm_mon  != now_tm->tm_mon ||
        forecast_tm->tm_year != now_tm->tm_year) {
      break;
    }
    
    float rainProb_hrly = weatherData.hourly[i].rain_prob;

    if (rainProb_hrly > 50) {
      if (!rainPeriodActive) {
        rainStart = t;
        rainPeriodActive = true;
      }
      rainEnd = t;
    } else {
      if (rainPeriodActive) {
        // Period ended
        break;
      }
    }
  }

  data += "," + weather[0] + "," + willRain[0];

  if (rainPeriodActive) {
    struct tm* start_tm = localtime(&rainStart);
    struct tm* end_tm = localtime(&rainEnd);

    data += "," + String(start_tm->tm_hour) + "-" + String(end_tm->tm_hour);
  }
  else
  {
    data += ",--:--";
  }

  return true;
}

void syncTime() {
  data = "";
  configTime(28800, 0, "pool.ntp.org", "time.nist.gov");

  while (time(nullptr) < 100000) {
    delay(500);
  }

  time_t now = time(nullptr);
  struct tm *t = localtime(&now); 
  Serial.print("Last Update: "); Serial.println(ctime(&now));

  char buf[64];
  snprintf(buf, sizeof(buf), "%02d:%02d,%02d/%02d/%d,%s", 
           t->tm_hour, t->tm_min, 
           t->tm_mon + 1, t->tm_mday, 
           t->tm_year + 1900,
           daysOfWeek[t->tm_wday]);

  data = String(buf);
}

void setup() {
  delay(1000);
  Serial.begin(115200);

  Config.apid = "my ESP";
  Config.psk = "polikoli";
  Config.autoReconnect = true;
  Config.autoRise = true;  // Keep portal if disconnected
  Config.portalTimeout = 0;  // Stay in portal indefinitely if no connection
  Portal.config(Config);

  Server.on("/", handleRoot);
  Server.on("/sync", handleSync);  // ✅ Add sync handler

  // Establish a connection with an autoReconnect option.
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }

  oneCallClient.setMetric(isMetric);
  oneCallClient.setLanguage(language);
}

void loop() {

  bool b_online = WiFi.status() == WL_CONNECTED;

  if(b_online && !synced){
    syncTime();
    bool b_sync = syncForecast(); 
    data += "," + WiFi.localIP().toString() + "," + (b_sync ? "T" : "F");
    Serial.println(data);
    synced = true;
  }
  else if (!b_online){
    synced = false;
  }

  if (Serial.available() > 0) {
    String receivedData = Serial.readStringUntil('\n'); // Read data until newline
    receivedData.trim();
    
    if (b_online) { 
        syncTime();
        bool b_sync = syncForecast(); 
        data += "," + WiFi.localIP().toString() + "," + (b_sync ? "T" : "F");
        Serial.println(data);
    }
    else{
      data = "";

      for (int i = 0; i < 7; i++) {
        if (receivedData == dowFc[i]) {
          data = " , , ," + weather[i] + "," + willRain[i] + ",--:--,OFFLINE,F"; 
          break;
        }
      }
      synced = false;
    }
  }

  Portal.handleClient();
}