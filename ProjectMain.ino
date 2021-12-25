/*
    Benjamin Jones
    12/24/21

    Significant amounts of this code is based off the work by:
        https://github.com/Bodmer
    thank u bodmer

    Utilizations:
        * Twitter API v2
        * rest7 JPG encoder api
            https://github.com/rest7/api/wiki/Encode-JPEG-as-progressive-or-baseline-and-remove-metadata-in-PHP


    This code is for a christmas present, designed to present @colorschemez's
    latest tweet picture and text on a 2.4" TFT display. It is implemented
    with an HUZZAH32 ESP32 Feather, and the 2.4" TFT FeatherWing

    Necessary changes:
    * make code prettier

    Wanted changes:
    * run loadTweet() based on micros(), not delay. This will allow interaction
        with the display.
    * Button to save image and send to email or something
    * Tap display to randomize tweets
    * Change query to only past hour. Decreases query size from 1024 to 768
*/
// For TFT
#include <SPI.h>
#include <TFT_eSPI.h>

// For ESP32
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TJpg_Decoder.h> // Jpg decoder to TFT display
#include "SPIFFS.h"
#include "time.h"

// Custom headers
#include "tokens.h" //contains ssid, pass, and BEARER_TOKEN
#include "List_SPIFFS.h"
#include "Load_Tweet.h"
#include "Download_Image.h"
#include "Baseline_Encoder.h"

// Adafruit ESP32 feather pinouts for 2.4" TFT featherwing
#define STMPE_CS 32
#define SD_CS    14
#define LEDPIN   13

// Time values for EST no DST, -5:00 GMT
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -21600; // EST
const int   daylightOffset_sec = 0;
const long  refreshDelay = 300000; // Check for tweet update every 5 minutes
String prevID; // Previous tweet ID, for checking if it is new

// Initialize TFT library
TFT_eSPI tft = TFT_eSPI();

// From Bodmer's example code
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) {
    if ( y >= tft.height() ) return 0;

    // This function will clip the image block rendering automatically at the TFT boundaries
    tft.pushImage(x, y, w, h, bitmap);

    // Return 1 to decode next block
    return 1;
}

void setup() {
    Serial.begin(115200);
    pinMode(LEDPIN, OUTPUT);
    delay(500);

    // Initializing TFT screen
    tft.begin();
    tft.fillScreen(ILI9341_BLACK);

    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);

    // Start up file system
    SPIFFS.begin(true);
    if (!SPIFFS.begin()) {
        tft.setTextSize(2);
        tft.println("SPIFFS initialisation failed!");
        Serial.println("SPIFFS initialisation failed!");
        delay(15000);
        while (1) yield(); // Stay here twiddling thumbs waiting
    }

    // Image is 480x480, which is ridiculously lucky. TFT is 320x240, which
    // allows a perfect image scale of 2 to fit into a square on the screen
    TJpgDec.setJpgScale(2);
    TJpgDec.setSwapBytes(true);
    TJpgDec.setCallback(tft_output);

    // Connect to WiFi
    attemptConnection();
    Serial.println();

    // Setting time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}


void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        // If WiFi gets disconnected, attempt to connect
        attemptConnection();
    }

    // Image url from twitter, text from tweet, tweet ID for checking if new
    String imageUrlProg, text, id;
    if (loadTweet(&imageUrlProg, &text, &id)) {
        if (id == prevID) {
            // Want to limit requests to the rest7 api, as it is run for free
            // Waits 5 mins before checking again
            Serial.println("No update");
            delay(refreshDelay);
            return;
        }
        // Setting ID for next loop
        prevID = id;

        // Displaying info
        tft.println("Loaded New Tweet");
        Serial.println(imageUrlProg);
        Serial.println(text);

        // Gotta delete the previous image before downloading new one
        if (SPIFFS.exists("/image.jpg") == true) {
            Serial.println("Remove file after every check");
            tft.println("Remove file after every check");
            SPIFFS.remove("/image.jpg");
        }

        // Takes URL from twitter, sends that through rest7 api, and rest7 API
        // returns their own URL to a baseline version of the JPG. The TJpgDec
        // library does not accept progressive JPG's, only baseline.
        String imageUrlBase;
        bool loaded_baseline = baselineEncoder(imageUrlProg, &imageUrlBase);
        if (loaded_baseline) {
            Serial.println("Converted to Baseline");
            tft.println("Converted to Baseline");
        } else {
            tft.fillScreen(ILI9341_BLACK);
            tft.setTextSize(2);
            Serial.println("Failed Baseline conversion");
            tft.println("Failed Baseline conversion");
            delay(15000);
            return;
        }

        // Downloads image from baseline URL provided by rest7 api
        bool loaded = imageToMem(imageUrlBase, "/image.jpg");
        if (loaded) {
          tft.println("Downloaded Baseline JPG");
          Serial.println("Downloaded Baseline JPG");
        }

        // Listing file
        listSPIFFS();

        // Displaying tweet and image
        displayTweet(text);

        // Free memory for troubleshooting. I suspect this code will only run
        // for a few days before it runs out of memory, but it (should) just
        // restart safely so I don't really care that much.
        Serial.print("Free Memory: ");
        Serial.println(ESP.getFreeHeap());
        delay(refreshDelay);
    }
}


void attemptConnection() {
    Serial.print("Connecting to ");
    Serial.println(ssid);
    tft.print("Connecting to ");
    tft.println(ssid);

    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        digitalWrite(LEDPIN, HIGH);
        Serial.print(".");
        delay(250);
        digitalWrite(LEDPIN, LOW);
    }
    Serial.println("");
    Serial.println("Wifi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    tft.println("Wifi connected");
    tft.println("IP address: ");
    tft.println(WiFi.localIP());
    digitalWrite(LEDPIN, HIGH);

    delay(2000);
    Serial.println();
    Serial.println();
}

// Displaying the tweet and JPG
void displayTweet(String tweet) {
    tft.fillScreen(ILI9341_BLACK);

    tft.setCursor(0, 0);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);

    tft.println(tweet);
    TJpgDec.drawFsJpg(0, 80, "/image.jpg");
    Serial.println("Drawn JPG");

}
