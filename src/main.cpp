#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include "time.h"
#include "secrets.h"

// NTP settings
const char *ntpServer = "bh.pool.ntp.org";
const long gmtOffset_sec = 3 * 60 * 60; // Bahrain GMT+3
const int daylightOffset_sec = 0;

// TFT pins
#define TFT_CS 1
#define TFT_DC 0
#define TFT_RST -1
#define TFT_SCK 6
#define TFT_MOSI 7

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

bool tryConnect(const char *ssid, const char *pass, unsigned long timeoutMs)
{
    WiFi.begin(ssid, pass);
    unsigned long start = millis();

    // show connecting indicator
    tft.fillRect(0, 24, 240, 24, ST77XX_BLACK);
    tft.setCursor(4, 24);
    tft.print("Connecting...");
    int dot = 0;

    while (millis() - start < timeoutMs)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            tft.fillRect(0, 24, 240, 24, ST77XX_BLACK);
            tft.setCursor(4, 24);
            tft.print("Connected");
            return true;
        }
        // update simple dot animation
        tft.fillRect(120, 24, 100, 16, ST77XX_BLACK);
        tft.setCursor(120, 24);
        for (int i = 0; i <= dot; ++i)
            tft.print('.');
        dot = (dot + 1) % 4;
        delay(300);
    }

    tft.fillRect(0, 24, 240, 24, ST77XX_BLACK);
    tft.setCursor(4, 24);
    tft.print("Connect failed");
    return false;
}

void setup()
{
    // TFT init
    SPI.begin(TFT_SCK, -1, TFT_MOSI, TFT_CS);
    tft.init(240, 320);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_WHITE);

    // Try saved networks and show progress on TFT
    const unsigned long perNetworkTimeout = 5000; // ms
    bool connected = false;
    for (size_t i = 0; i < wifiCredsCount; ++i)
    {
        tft.fillRect(0, 0, 240, 24, ST77XX_BLACK);
        tft.setCursor(4, 0);
        tft.print("Trying:");
        tft.setCursor(4, 28);
        tft.print(wifiCreds[i].ssid);

        if (tryConnect(wifiCreds[i].ssid, wifiCreds[i].pass, perNetworkTimeout))
        {
            connected = true;
            break;
        }
        delay(300);
    }

    if (!connected)
    {
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(4, 4);
        tft.print("No WiFi");
        // still continue; time won't initialize until connected
    }
    else
    {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        tft.fillRect(0, 0, 240, 56, ST77XX_BLACK);
        tft.setCursor(4, 4);
        tft.print("WiFi:");
        tft.setCursor(4, 28);
        tft.print(WiFi.SSID());
    }
}

void loop()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        tft.fillRect(0, 56, 240, 32, ST77XX_BLACK); // clear time area
        tft.setCursor(4, 56);
        tft.print("No time");
        delay(1000);
        return;
    }

    char timebuf[32];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // clear only the rectangle where the time is drawn to avoid flicker
    tft.fillRect(0, 56, 240, 32, ST77XX_BLACK);
    tft.setCursor(4, 56);
    tft.print(timebuf);

    delay(1000);
}
