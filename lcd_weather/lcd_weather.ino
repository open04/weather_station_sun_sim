#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <SPI.h>
#include <DS3231.h>
#include <Wire.h>
#include "icons.h"

MCUFRIEND_kbv tft;
DS3231 rtc;

// Touch calibration values (adjust if needed)
#define MINPRESSURE 200
#define MAXPRESSURE 1000
const uint8_t chipSelect = 10;
const uint8_t XP = 6, XM = A2, YP = A1, YM = 7;
const uint16_t TS_LEFT = 907, TS_RT = 136, TS_TOP = 942, TS_BOT = 139;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button next_btn, sync_btn, on_btn, off_btn;
int pixel_x, pixel_y;

enum ScreenState { MAIN_SCREEN, SYNC_SCREEN };
ScreenState currentScreen = MAIN_SCREEN;
unsigned long lastActivityTime = 0;
const unsigned long timeoutDuration = 3000;
bool ledOn;
bool century = false;
bool h12Flag;
bool pmFlag;

struct WeatherData {
    char timeStr[6] = "00:00";
    char dateStr[11] = "00/00/0000";
    char dowStr[9] = "SUNDAY";
    char weather[10] = "SUNNY";
    char willRain[4] = "NO";
    char rainTime[6] = "--:--";
    char ipAdd[16] = "NOT CONNECTED";
    char sync[2] = "F";
};

WeatherData currentData;
WeatherData prevData;

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x8410
#define GRAYELLOW 0x7D0C
#define GRAYBLUE 0x8414

bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);
    digitalWrite(XM, HIGH);

    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        // For landscape (rotation 1)
        pixel_x = map(p.y, TS_TOP, TS_BOT, 0, tft.width());
        pixel_y = map(p.x, TS_LEFT, TS_RT, 0, tft.height());
    }
    return pressed;
}

void drawMainScreen() {
    if (strcmp(currentData.weather, prevData.weather) !=0)
    {
      if (String(currentData.weather) == "B"){
        tft.drawBitmap(0, 0,(const uint8_t*)b, 100, 100, YELLOW);
      }
      else if (String(currentData.weather) == "C"){
        tft.drawBitmap(0, 0,(const uint8_t*)c, 100, 100, BLUE);
      }
      else if (String(currentData.weather) == "H"){
        tft.drawBitmap(0, 0,(const uint8_t*)h, 100, 100, YELLOW);
      }
      else if (String(currentData.weather) == "I"){
        tft.drawBitmap(0, 0,(const uint8_t*)i, 100, 100, BLUE);
      }
      else if (String(currentData.weather) == "N"){
        tft.drawBitmap(0, 0,(const uint8_t*)n, 100, 100, BLUE);
      }
      else if (String(currentData.weather) == "R"){
        tft.drawBitmap(0, 0,(const uint8_t*)r, 100, 100, GRAY);
      }
      else if (String(currentData.weather) == "Q"){
        tft.drawBitmap(0, 0,(const uint8_t*)q, 100, 100, GRAY);
      }
      else if (String(currentData.weather) == "P"){
        tft.drawBitmap(0, 0,(const uint8_t*)p, 100, 100, GRAY);
      }
      else if (String(currentData.weather) == "J"){
        tft.drawBitmap(0, 0,(const uint8_t*)j, 100, 100, GRAYELLOW);
      }
      else if (String(currentData.weather) == "K"){
        tft.drawBitmap(0, 0,(const uint8_t*)k, 100, 100, GRAYBLUE);
      }
      else{
        tft.drawBitmap(0, 0,(const uint8_t*)na, 100, 100, RED);
      }

      strcpy(prevData.weather, currentData.weather);
    }

    // currentData.timeStr = String(rtc.getHour(h12Flag, pmFlag), DEC) + ":" + String(rtc.getMinute());
    snprintf(currentData.timeStr, sizeof(currentData.timeStr), "%2d:%2d", rtc.getHour(h12Flag, pmFlag), rtc.getMinute());
    if (strcmp(currentData.timeStr, prevData.timeStr) != 0)
    {
        tft.fillRect(105, 0, 180, 50, BLACK);
        tft.setTextColor(GREEN);
        tft.setTextSize(6);
        tft.setCursor(105, 0);
        tft.print(currentData.timeStr);
        strcpy(prevData.timeStr, currentData.timeStr);
    }

    // currentData.dateStr = String(rtc.getMonth(century)) + "/" + String(rtc.getDate()) + "/" + String(rtc.getYear())
    snprintf(currentData.dateStr, sizeof(currentData.dateStr), "%02d/%02d/%02d", rtc.getMonth(century), rtc.getDate(), rtc.getYear());
    if (strcmp(currentData.dateStr, prevData.dateStr) != 0)
    {
        tft.fillRect(120, 50, 180, 20, BLACK);
        tft.setTextColor(GREEN);
        tft.setTextSize(3);
        tft.setCursor(120, 50);
        tft.print(currentData.dateStr);
        strcpy(prevData.dateStr, currentData.dateStr);
    }
    if (strcmp(currentData.dowStr, prevData.dowStr) != 0)
    {
        tft.fillRect(120, 75, 180, 50, BLACK);
        tft.setTextColor(WHITE);
        tft.setTextSize(3);
        tft.setCursor(120, 75);
        tft.print(currentData.dowStr);
        strcpy(prevData.dowStr, currentData.dowStr);
    }
    if (strcmp(currentData.rainTime, prevData.rainTime) != 0)
    {
        tft.fillRect(0, 200, 120, 30, BLACK);
        // Time it will rain
        tft.setTextColor(MAGENTA);
        tft.setTextSize(4);
        tft.setCursor(0, 200);
        tft.print(currentData.rainTime);
        strcpy(prevData.rainTime, currentData.rainTime);
    }
    // if (strcmp(currentData.temperature, prevData.temperature) != 0)
    // {
    //     tft.fillRect(170, 105, 90, 60, BLACK);
    //     // Temperature
    //     tft.setTextColor(YELLOW);
    //     tft.setTextSize(7);
    //     tft.setCursor(170, 105);
    //     tft.print(currentData.temperature);
    //     tft.setCursor(258, 105);
    //     tft.drawCircle(258, 110, 5, YELLOW);
    //     tft.drawCircle(258, 110, 4, YELLOW);
    //     tft.setCursor(267, 105);
    //     tft.print("C");
    //     strcpy(prevData.temperature, currentData.temperature);
    // }
    // if (strcmp(currentData.humidity, prevData.humidity) != 0)
    // {
    //     tft.fillRect(170, 165, 70, 50, BLACK);  // Clear old time
    //     // Humidity
    //     tft.setTextColor(YELLOW);
    //     tft.setTextSize(6);
    //     tft.setCursor(170, 165);
    //     tft.print(currentData.humidity);
    //     tft.setTextSize(4);
    //     tft.setCursor(240, 168);
    //     tft.print("%");
    //     strcpy(prevData.humidity, currentData.humidity);
    // }

    next_btn.initButton(&tft, tft.width() - 40 / 2 - 10, tft.height() - 40 / 2 - 10,
                        40, 40, WHITE, GREEN, BLACK, ">>", 2);
    next_btn.drawButton(false);
}

void drawPrevMainScreen() {
    tft.fillScreen(BLACK);
    if (String(currentData.weather) == "B"){
      tft.drawBitmap(0, 0,(const uint8_t*)b, 100, 100, YELLOW);
    }
    else if (String(currentData.weather) == "C"){
      tft.drawBitmap(0, 0,(const uint8_t*)c, 100, 100, BLUE);
    }
    else if (String(currentData.weather) == "H"){
      tft.drawBitmap(0, 0,(const uint8_t*)h, 100, 100, YELLOW);
    }
    else if (String(currentData.weather) == "I"){
      tft.drawBitmap(0, 0,(const uint8_t*)i, 100, 100, BLUE);
    }
    else if (String(currentData.weather) == "N"){
      tft.drawBitmap(0, 0,(const uint8_t*)n, 100, 100, BLUE);
    }
    else if (String(currentData.weather) == "R"){
      tft.drawBitmap(0, 0,(const uint8_t*)r, 100, 100, GRAY);
    }
    else if (String(currentData.weather) == "Q"){
      tft.drawBitmap(0, 0,(const uint8_t*)q, 100, 100, GRAY);
    }
    else if (String(currentData.weather) == "P"){
      tft.drawBitmap(0, 0,(const uint8_t*)p, 100, 100, GRAY);
    }
    else if (String(currentData.weather) == "J"){
      tft.drawBitmap(0, 0,(const uint8_t*)j, 100, 100, GRAYELLOW);
    }
    else if (String(currentData.weather) == "K"){
      tft.drawBitmap(0, 0,(const uint8_t*)k, 100, 100, GRAYBLUE);
    }
    else{
      tft.drawBitmap(0, 0,(const uint8_t*)na, 100, 100, RED);
    }

    if (String(currentData.willRain) == "NO") {
        tft.drawBitmap(15, 105, (const uint8_t*)no_rain, 90, 90, MAGENTA);
    }
    else {
        tft.drawBitmap(15, 105,(const uint8_t*)umbrella, 90, 90, MAGENTA);
    }

    tft.drawBitmap(130, 105, (const uint8_t*)temp_icon, 25, 50, CYAN);
    tft.drawBitmap(118, 165, (const uint8_t*)humd_icon, 50, 50, CYAN);

    tft.setTextColor(GREEN);
    tft.setTextSize(6);
    tft.setCursor(105, 0);
    
    tft.print(currentData.timeStr);

    tft.setTextSize(3);
    tft.setCursor(120, 50);    
    tft.print(currentData.dateStr);
    
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.setCursor(120, 75);
    tft.print(currentData.dowStr);

    tft.setTextColor(MAGENTA);
    tft.setTextSize(4);
    tft.setCursor(0, 200);
    tft.print(currentData.rainTime);
        
    tft.setTextColor(YELLOW);
    tft.setTextSize(7);
    tft.setCursor(170, 105);
    // tft.print(currentData.temperature);
    tft.setCursor(258, 105);
    tft.drawCircle(258, 110, 5, YELLOW);
    tft.drawCircle(258, 110, 4, YELLOW);
    tft.setCursor(267, 105);
    tft.print("C");
        
    tft.setTextColor(YELLOW);
    tft.setTextSize(6);
    tft.setCursor(170, 165);
    // tft.print(currentData.humidity);
    tft.setTextSize(4);
    tft.setCursor(240, 168);
    tft.print("%");
}

void drawSyncScreen() {
    tft.fillScreen(BLACK);
    if (String(currentData.ipAdd) == "NOT CONNECTED") {
        tft.setTextColor(RED);
        tft.setTextSize(3);
        tft.setCursor(40, 0);
        tft.print(currentData.ipAdd);
    }
    else {
        tft.setTextColor(BLUE);
        tft.setTextSize(3);
        tft.setCursor(40, 0);
        tft.print(currentData.ipAdd);
    }
    
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.setCursor(40, 40);
    tft.print("Last Sync:");
    tft.setCursor(160, 40);
    tft.print(currentData.dateStr);

    if (String(currentData.sync) == "F") {
        tft.drawBitmap(140, 60, (const uint8_t*)cross, 50, 50, RED);
    }
    else {
        tft.drawBitmap(140, 60, (const uint8_t*)check, 50, 50, GREEN);
        rtc.setYear(String(currentData.dateStr).substring(6, 10).toInt());
        rtc.setMonth(String(currentData.dateStr).substring(0, 2).toInt());
        rtc.setDate(String(currentData.dateStr).substring(3, 5).toInt());
        rtc.setDoW(currentData.dowStr);
        rtc.setHour(String(currentData.timeStr).substring(0, 2).toInt());
        rtc.setMinute(String(currentData.timeStr).substring(3, 5).toInt());
    }

    // Sync Now button
    sync_btn.initButton(&tft, tft.width() / 2, 140,
                        120, 40, WHITE, YELLOW, BLACK, "Sync Now", 2);
    sync_btn.drawButton(false);

    on_btn.initButton(&tft, 80, 190, 100, 40,
                         WHITE, GREEN, BLACK, "LED ON", 2);
    on_btn.drawButton(false);

    // LED OFF button
    off_btn.initButton(&tft, 240, 190, 100, 40,
                          WHITE, RED, BLACK, "LED OFF", 2);
    off_btn.drawButton(false);

    if(ledOn) {
        tft.fillCircle(tft.width() / 2, 190, 15, BLACK);  
        tft.fillCircle(tft.width() / 2, 190, 15, WHITE); 
    }
    else {
        tft.fillCircle(tft.width() / 2, 190, 15, BLACK); 
        tft.drawCircle(tft.width() / 2, 190, 15, RED); 
    }

    lastActivityTime = millis();
}

void setup(void)
{
    Serial1.begin(115200);
    Serial.begin(115200);
    Wire.begin();
    rtc.setClockMode(false); // 24 hour

    uint16_t ID = tft.readID();
    if (ID == 0xD3D3) ID = 0x9486; // write-only shield
    tft.begin(ID);
    tft.setRotation(1); // LANDSCAPE
    tft.fillScreen(BLACK);
    drawPrevMainScreen();
    drawMainScreen();
}

void loop(void)
{
    if (Serial1.available() > 0) {
        String receivedData = Serial1.readStringUntil('\n'); // Read data until newline
        Serial.println (receivedData);

        // Parse the incoming data (time, dateStr, dowStr, weather, willRain, rainTime, ip add, done sync)
        int firstComma = receivedData.indexOf(',');
        int secondComma = receivedData.indexOf(',', firstComma + 1);
        int thirdComma = receivedData.indexOf(',', secondComma + 1);
        int fourthComma = receivedData.indexOf(',', thirdComma + 1);
        int fifthComma = receivedData.indexOf(',', fourthComma + 1);
        int sixthComma = receivedData.indexOf(',', fifthComma + 1);
        int lastComma = receivedData.indexOf(',', sixthComma + 1);

        if (firstComma != -1 && 
            secondComma != -1 && 
            thirdComma != -1 && 
            fourthComma != -1 && 
            fifthComma != -1 && 
            sixthComma != -1 &&  
            lastComma != -1) {
            receivedData.substring(0, firstComma).toCharArray(currentData.timeStr, sizeof(currentData.timeStr));
            receivedData.substring(firstComma + 1, secondComma).toCharArray(currentData.dateStr, sizeof(currentData.dateStr));
            receivedData.substring(secondComma + 1, thirdComma).toCharArray(currentData.dowStr, sizeof(currentData.dowStr));
            receivedData.substring(thirdComma + 1, fourthComma).toCharArray(currentData.weather, sizeof(currentData.weather));
            receivedData.substring(fourthComma + 1, fifthComma).toCharArray(currentData.willRain, sizeof(currentData.willRain));
            receivedData.substring(fifthComma + 1, sixthComma).toCharArray(currentData.rainTime, sizeof(currentData.rainTime));
            receivedData.substring(sixthComma + 1, lastComma).toCharArray(currentData.ipAdd, sizeof(currentData.ipAdd));
            receivedData.substring(lastComma + 1).toCharArray(currentData.sync, sizeof(currentData.sync));

            // Update the main screen with the new data
            Serial.print(currentData.timeStr); Serial.print("       ");
            Serial.print(currentData.dateStr); Serial.print("       ");
            Serial.print(currentData.dowStr); Serial.print("       ");
            Serial.print(currentData.weather); Serial.print("       ");
            Serial.print(currentData.willRain); Serial.print("       ");
            Serial.println(currentData.rainTime); 

            drawMainScreen();
        }
    }

    bool down = Touch_getXY();

    switch (currentScreen) {
        case MAIN_SCREEN:
            next_btn.press(down && next_btn.contains(pixel_x, pixel_y));

            if (next_btn.justReleased()) next_btn.drawButton();
            if (next_btn.justPressed()) {
                drawSyncScreen();
                currentScreen = SYNC_SCREEN;
            }
            break;

        case SYNC_SCREEN:
            if (millis() - lastActivityTime > timeoutDuration) {
                drawPrevMainScreen();
                drawMainScreen();
                currentScreen = MAIN_SCREEN;
                break;
            }

            sync_btn.press(down && sync_btn.contains(pixel_x, pixel_y));
            if (sync_btn.justReleased()) sync_btn.drawButton();
            if (sync_btn.justPressed()) {
                Serial1.println("sync");
                sync_btn.drawButton(true);
                lastActivityTime = millis(); // Reset inactivity timer
            }

            // LED ON button
            on_btn.press(down && on_btn.contains(pixel_x, pixel_y));
            if (on_btn.justReleased()) on_btn.drawButton();
            if (on_btn.justPressed()) {
                ledOn = true;
                tft.fillCircle(tft.width() / 2, 190, 15, BLACK);  
                tft.fillCircle(tft.width() / 2, 190, 15, WHITE); 
                on_btn.drawButton(true);
                lastActivityTime = millis();
            }

            // LED OFF button
            off_btn.press(down && off_btn.contains(pixel_x, pixel_y));
            if (off_btn.justReleased()) off_btn.drawButton(); 
            if (off_btn.justPressed()) {
                ledOn = false;
                tft.fillCircle(tft.width() / 2, 190, 15, BLACK); 
                tft.drawCircle(tft.width() / 2, 190, 15, RED); 
                off_btn.drawButton(true);
                lastActivityTime = millis();
            }

            if (down) {
                lastActivityTime = millis(); // Reset timer on any touch
            }
            break;
    }
}
