/*
Example query, uses ISO 8601 format
https://api.twitter.com/2/tweets/search/recent?query=from%3Acolorschemez%20&start_time=YYYY-MM-DDTHH%3A00%3A00-05%3A00&expansions=attachments.media_keys&media.fields=url";
*/

bool loadTweet(String *url, String *tweet, String *id) {

    // Getting time from time.h struct
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      Serial.println("Failed to obtain time, trying again...");
      delay(1000);
      return 0;
    }

    // Setting custom time. Queries the past hour for tweets.
    char queryMidChar[50];
    strftime(queryMidChar, 50, "%Y-%m-%dT%H", &timeinfo);
    String queryMid = String(queryMidChar);

    // Rest of Query
    String queryStart = "https://api.twitter.com/2/tweets/search/recent?query=from%3Acolorschemez%20&start_time=";
    String queryEnd = "%3A00%3A00-05%3A00&expansions=attachments.media_keys&media.fields=url";

    // Sending HTTP request with authorization
    HTTPClient http;
    http.setAuthorizationType("Bearer");
    http.begin(queryStart + queryMid + queryEnd);
    http.setAuthorization(BEARER_TOKEN);
    int httpCode = http.GET();

    if (httpCode > 0) { //Check for the returning code
        // Get HTTP response payload
        String payload = http.getString();

        // Setting up JSON deserialization
        // Uses ArduinoJson, tutorials online
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print(F("deserializeJson() tweet failed with code "));
            Serial.println(error.c_str());
            return 0;
        }

        // Code provided by ArduinoJson assistant
        JsonObject data_0 = doc["data"][0];
        const char* tweetChar = data_0["text"];
        const char* tweetID = data_0["id"];
        JsonObject includes_media_0 = doc["includes"]["media"][0];
        const char* urlChar = includes_media_0["url"];

        // This section is for removing the extraneous URL that exists at
        // the end of the tweet for some reason.
        int textLen = strlen(tweetChar) - 23; //23 is length of url at end of tweet with null character
        char tweetStr[textLen];
        for (int i = 0; i < textLen; i++) {
            tweetStr[i] = tweetChar[i];
        }

        // This section is for changing the PNG to JPG. Only JPG can be handled
        // by arduino processors due to necessary computational capacities.
        // All URLs are the same # of characters, which makes this easy.
        char urlStr[48];
        for (int i = 0; i < 48; i++) {
            urlStr[i] = urlChar[i];
        }
        // png to jpg
        urlStr[44] = 'j';
        urlStr[45] = 'p';
        urlStr[46] = 'g';
        // extra null terminator since it was removed during the separation
        tweetStr[textLen] = '\0';

        // Assigning pointers to be "returned"
        *url = urlStr;
        *tweet = tweetStr;
        *id = tweetID;
    } else {
        Serial.println("Error on Tweet HTTP request:");
        Serial.println(httpCode);
        return 0;
    }
    http.end(); //Free the resources
    return 1;
}
