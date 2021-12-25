bool imageToMem(String url, String filename) {
    // This section was mostly copied from:
    // https://github.com/Bodmer/TJpg_Decoder/blob/master/examples/SPIFFS/SPIFFS_Web_Jpg/Web_Fetch.h
    // Creator: Bodmer on github. Shoutout to Bodmer
    if (SPIFFS.exists(filename) == true) {
        Serial.println("Found " + filename);
        return 0;
    }

    Serial.println("Downloading " + filename + " from " + url);

    if ((WiFi.status() == WL_CONNECTED)) {
        // Start connection
        HTTPClient http;
        http.begin(url);

        int httpCode = http.GET();
        if (httpCode > 0) {
            fs::File f = SPIFFS.open(filename, "w+");
            if (!f) {
                Serial.println("file open failed");
                delay(3000);
                return 0;
            }

            Serial.printf("HTTP GET... CODE: %d\n", httpCode);

            // File found
            if (httpCode == HTTP_CODE_OK) {

                // Get length of document
                int len = http.getSize();

                // Create buffer for read
                uint8_t buff[128] = { 0 };

                // Get TCP stream
                WiFiClient * stream = http.getStreamPtr();

                // read all data from server
                while (http.connected() && (len > 0 || len == -1)) {
                    // Get available data size
                    size_t size = stream->available();

                    if (size) {
                        // Read up to 128 bytes
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // Write it to file
                        f.write(buff, c);

                        // Calculate remaining bytes
                        if (len > 0) {
                            len -= c;
                        }
                    }
                    yield();
                }
                Serial.println();
                Serial.println("HTTP connection closed or file end");
            }
            f.close();
        } else {
            Serial.printf("HTTP GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    return 1;
}
