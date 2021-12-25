# colorschemez display
This code takes the most recent tweet from the twitter account @colorschemez, and displays it onto a 2.4" TFT display. 

If you use this with different hardwar, you must edit the TJpg_Decoder library Info for that is within the library's documentation, its pretty straightforward. Feel free to message me (bexben) if you have any questions.

Some parts of this code is used from Bodmer's examples (https://github.com/Bodmer/TJpg_Decoder). I marked what was used within the code. Slight edits were made as necessary to meet my goals.

# Hardware
Adafruit HUZZAH32 – ESP32 Feather Board: https://www.adafruit.com/product/3405

Adafruit 2.4" TFT LCD with Touchscreen Breakout w/MicroSD Socket - ILI9341: https://www.adafruit.com/product/2478

# API's used
* Twitter API v2
* rest7 JPG progressive baseline converter: https://github.com/rest7/api/wiki/Encode-JPEG-as-progressive-or-baseline-and-remove-metadata-in-PHP


