# mega2560_esp8266_ws2812_lantern
WS2812 lantern: Robotdyn "mega2560 + esp8266" "WiFi R3" AliExpress board. Over a serial, ESP8266 tells MEGA2560 to blink a WS2812.

This project is based on https://cassiopeia.hk/lantern/ with the changes for the architecture of this board
(more information about it - https://iotdiary.blogspot.com/2017/11/esp-link-on-atmega2560esp8266-board.html ).
Instead of controlling the WS2812 board directly (which it couldn't do because no direct access), the code
had to be changed - so that ESP8266 generates a LED control command and sends it over serial to MEGA2560 board.

Usage and flashing settings:
1) lantern_part2560.ino for MEGA2560. DIP switches: only 3 and 4 are enabled, and RXD3/TXD3.
2) lantern_part8266.ino for ESP8266. DIP switches: only 5 and 6 and 7 are enabled, and RXD3/TXD3.
https://arduino-esp8266.readthedocs.io/en/latest/installing.html - downgrade the ESP8266 library to 2.7.4 
3) DIP switches: to connect MEGA2560 with ESP8266, only 1 and 2 are enabled (and RXD3/TXD3).

Now ESP8266 tells MEGA2560 to blink a WS2812 according to the chosen algorithm at ESP8266 code.