bool baselineEncoder(String urlProg, String *urlBase) {
    // This section is for using the following API for converting progressive
    // JPG that twitter provides into a baseline format that TJpg_Decoder
    // can interpret

    if (WiFi.status() == WL_CONNECTED) {
        // Starts up http client
        HTTPClient http;

        http.begin("http://api.rest7.com/v1/jpeg_optim.php?url=" + urlProg + "&encoding=baseline");
        int httpCode = http.GET();

        if (httpCode > 0) {
            Serial.println("Connected to rest7 API");

            // Highly recommend arduinoJson assistant for generating this section
            String payload = http.getString();
            StaticJsonDocument<256> doc;

            DeserializationError error = deserializeJson(doc, payload);

            if (error) {
                Serial.print("DeserializeJson() failed rest7: ");
                Serial.println(error.c_str());
                return 0;
            }

            const char* urlBaseChar = doc["file"];
            *urlBase = urlBaseChar;
            Serial.println("Successfully got baseline URL");

        } else {
            Serial.println("Error on rest7 HTTP request:");
            Serial.println(httpCode);
            return 0;
        }
        http.end();
    }
    return 1;
}
