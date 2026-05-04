#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
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
// LED pins
#define LED_DATA_PIN 8
#define NUM_LEDS 16

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

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

static void setAllRGB(uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t color = strip.Color(r, g, b);
    for (int i = 0; i < NUM_LEDS; ++i)
        strip.setPixelColor(i, color);
    strip.show();
}

// tiny HSV->RGB helper using 0..65535 hue like ColorHSV expects
static uint32_t colorFromHSV16(uint16_t hue16, uint8_t sat = 255, uint8_t val = 150)
{
    return strip.ColorHSV(hue16, sat, val);
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

    strip.begin();
    strip.setBrightness(8);
    strip.show();

    // Start as RED while trying networks
    setAllRGB(255, 0, 0);

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
        // show RED and message (stays red)
        setAllRGB(255, 0, 0);
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(4, 4);
        tft.print("No WiFi");
        // still continue; time won't initialize until connected
    }
    else
    {
        // brief GREEN pulse on successful connect
        setAllRGB(0, 255, 0);
        delay(700);
        // blackout briefly to transition to rainbow later
        setAllRGB(0, 0, 0);

        // init time (NTP)
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

    static uint16_t j = 0;
    for (int i = 0; i < NUM_LEDS; ++i)
    {
        strip.setPixelColor(i, strip.ColorHSV((j + i * 65536 / NUM_LEDS) & 0xFFFF));
    }
    strip.show();
    j += 256;
    delay(50);
}
