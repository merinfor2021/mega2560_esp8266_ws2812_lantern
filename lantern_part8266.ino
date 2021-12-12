//// [000.E662FE][001.E662FE][002.E662FE][003.E662FE][004.E662FE][005.E662FE][006.E662FE][007.E662FE]!

/* Neopixel web server with ESP8266-01
 * with switch to rainbow animation
 *
 * Tom Tobback Sep 2016
 * BuffaloLabs www.cassiopeia.hk/kids
LICENSE:
Copyright (c) 2016 Tom Tobback

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 * web page esp/rgb returns POST request with 3 RGB parameters
 * web page inspired by https://github.com/dimsumlabs/nodemcu-httpd
 *
 * voltage regulator from 5V to 3.3V
 * neopixel strip 8 LEDs on GPIO2, and Vcc on 3.3V otherwise the data goes wrong sometimes
 *
 * ESP8266 ESP-01 connections
 * Vcc 3.3V
 * GND GND
 * GPIO2 neopixel data pin with 430ohm resistor
 * GPIO3 switch between animation/webserver (only at startup!) with 4K3 pull-down resistor and IN5819 diode to Vcc
 * CH_PD 3.3V (actually Vcc)
 * RST 3.3V (actually Vcc)
 *
 * 3 way switch on 3.3V to choose between webserver and animation
 * GPIO3=RX is connected to switch: HIGH= animation LOW= webserver
 * in HIGH state the power goes from GPIO3 to Vcc via a diode IN5819
 * in LOW state the Vcc power cannot reach GPIO3
 */
 
#define STRIP_NUMPIXELS 8

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

/* Set up your SSID and Password */
const char*     ssid =  "NodeMCU"; // SSID
const char* password = "19216811"; // Password

/* Set up your IP addresses */
IPAddress local_ip(192,168,1,1);
IPAddress  gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

const byte DNS_PORT = 53;
DNSServer dnsServer;
ESP8266WebServer webServer(80);

boolean animation;

String webpageRGB = ""
 " <!DOCTYPE html>"
  " <html>"
   " <head>"
    " <title>Neopixel control</title>"
     " <meta name='mobile-web-app-capable' content='yes' />"
     " <meta name='viewport' content='width=device-width' />"
   " </head>"
   " <button>"
    " <details>"
     " <iframe srcdoc=\""
       " <body style='margin: 0px; padding: 0px;'>"
        " <canvas id='colorspace'></canvas>"
       " </body>"
       " <script type='text/javascript'>"
        " (function () {"
         " var canvas = document.getElementById('colorspace');"
         " var ctx = canvas.getContext('2d');"
         " function drawCanvas() {"
          " var colours = ctx.createLinearGradient(0, 0, window.innerWidth, 0);"
          " for (var i=0; i <= 360; i+=10) {"
           " colours.addColorStop(i/360, 'hsl(' + i + ', 100%, 50%)');"
          " }"
          " ctx.fillStyle = colours;"
          " ctx.fillRect(0, 0, window.innerWidth, window.innerHeight);"
          " var luminance = ctx.createLinearGradient(0, 0, 0, ctx.canvas.height);"
          " luminance.addColorStop(0,    '#ffffff');"
          " luminance.addColorStop(0.05, '#ffffff');"
          " luminance.addColorStop(0.5,  'rgba(0,0,0,0)');"
          " luminance.addColorStop(0.95, '#000000');"
          " luminance.addColorStop(1,    '#000000');"
          " ctx.fillStyle = luminance;"
          " ctx.fillRect(0, 0, ctx.canvas.width, ctx.canvas.height);"
         " }"
         " var eventLocked = false;"
         " function handleEvent(clientX, clientY) {"
          " if (eventLocked) {"
           " return;"
          " }"
         " function colourCorrect(v) {"
          " return Math.round(1023-(v*v)/64);"
         " }"
         " var data = ctx.getImageData(clientX, clientY, 1, 1).data;"
         " var params = ["
          " 'r=' + colourCorrect(data[0]),"
          " 'g=' + colourCorrect(data[1]),"
          " 'b=' + colourCorrect(data[2])"
          " ].join('&');"
         " var req = new XMLHttpRequest();"
         " req.open('POST', '?' + params, true);"
         " req.send();"
         " eventLocked = true;"
         " req.onreadystatechange = function() {"
          " if (req.readyState == 4) {"
           " eventLocked = false;"
          " }"
         " }"
        " }"
        " canvas.addEventListener('click', function(event) {"
         " handleEvent(event.clientX, event.clientY, true);"
         " }, false);"
        " canvas.addEventListener('touchmove', function(event) {"
         " handleEvent(event.touches[0].clientX, event.touches[0].clientY);"
         " }, false);"
        " function resizeCanvas() {"
         " canvas.width  = window.innerWidth;"
         " canvas.height = window.innerHeight;"
         " drawCanvas();"
        " }"
        " window.addEventListener('resize', resizeCanvas, false);"
        " resizeCanvas();"
        " drawCanvas();"
        " document.ontouchmove = function(e) {e.preventDefault()};"
       " })();"
      " </script>"
     " \">"
    " </iframe>"
   " </details>"
  " </button>"
 " </html>";
 //////////////////////////////////////////////////////////////////////////////////////////////////

static uint32_t strip_color(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void serial_printhex(uint32_t num, uint16_t digits)
{
  if ((digits >= 3) && (num < 0x100)) {
    Serial.print("0");
  }
  if ((digits >= 2) && (num < 0x10)) {
    Serial.print("0");
  }
  Serial.print(num, HEX);
}

void strip_setPixelColor(uint32_t n, uint8_t r, uint8_t g, uint8_t b)
{
  Serial.print("[");
  serial_printhex(n, 3);
  Serial.print(".");
  serial_printhex(r, 2);
  serial_printhex(g, 2);
  serial_printhex(b, 2);
  Serial.print("]");
  return;
}

void strip_show()
{
  Serial.print("!");
  return;
}

void strip_setPixelColor(uint32_t n, uint32_t c)
{
  uint8_t r = (uint8_t)(c >> 16);
  uint8_t g = (uint8_t)(c >> 8);
  uint8_t b = (uint8_t)c;
  strip_setPixelColor(n, r, g, b);
}

void handleRGB()
{
  // Serial.println("handle RGB..");

  webServer.send(200, "text/html", webpageRGB);

  String red   = webServer.arg(0); // read RGB arguments
  String green = webServer.arg(1);
  String blue  = webServer.arg(2);

  allRGB(255 - red.toInt() / 4, 255 - green.toInt() / 4, 255 - blue.toInt() / 4);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
 /*
  * It is recommended to use the lower baud rate. The higher the baud rate,
  * the higher the bit error rate will be, and the control action might fail.
  */
  Serial.begin(4800); // Initialize the ESP8266 <=> MEGA2560 serial port

  /* STRIP */// strip.begin();
  strip_show(); // Initialize all pixels to 'off'
  /* STRIP */// strip.setBrightness(250); // 0-255 range

  animation = false;

  if (!animation) { // start webserver
    allRGB(0, 0, 0);
    delay(1000);
    // Serial.println("Starting webserver...");

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ssid, password);

    // if DNSServer is started with "*" for domain name, it will reply with provided IP to all DNS request
    dnsServer.start(DNS_PORT, "esp", local_ip);
    webServer.on("/", handleRGB);

    webServer.begin();

    introRed(); // knightrider
    // colorWipe(0x859d12, 5);
    // rainbow(5);
    // rainbowCycle(5);
    // theaterChaseRainbow(5);
    // theaterChase(0x7e1ae1, 5);
    // colorWipe(0x123456, 5);
    
    //// strip_setPixelColor(0, 255, 0, 0);
    //// strip_setPixelColor(3, 255, 0, 0);
    //// strip_setPixelColor(4, 255, 0, 0);
    //// strip_setPixelColor(7, 255, 0, 0);
    strip_show();
/*
 * CURRENT LOG:
 * CLEAR_ALL, CLEAR_ALL + 0 LED RED, CLEAR_ALL + 1 LED RED, ..., CLEAR_ALL + 7 LED RED,
 * CLEAR_ALL + 6 LED RED, ..., CLEAR_ALL + 0 LED RED, ..., CLEAR_ALL + 7 LED RED, ...,
 * CLEAR_ALL + 1 LED RED, CLEAR_ALL, 8 LED RED (???), 0 + 3 + 4 + 7 LED RED
 */
  }
  else {
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_STA);
    // Serial.println("Starting animation...");
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  if (animation) {
    rainbow(20);
    rainbowCycle(20);
    theaterChaseRainbow(50);
  }
  else {
    dnsServer.processNextRequest();
    webServer.handleClient();
  }
}
 //////////////////////////////////////////////////////////////////////////////////////////////////
 /////////////////////// NEOPIXEL FUNCTIONS //////////////////////////////////////////////////
 //////////////////////////////////////////////////////////////////////////////////////////////////

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  uint32_t i;

  for (i = 0; i < STRIP_NUMPIXELS; i++) {
    strip_setPixelColor(i, c);
    strip_show();
    delay(wait);
  }
}

void rainbow(uint8_t wait)
{
  uint32_t i;
  uint16_t j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < STRIP_NUMPIXELS; i++) {
      strip_setPixelColor(i, Wheel((i + j) & 255));
    }
    strip_show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait)
{
  uint32_t i;
  uint16_t j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < STRIP_NUMPIXELS; i++) {
      strip_setPixelColor(i, Wheel(((i * 256 / STRIP_NUMPIXELS) + j) & 255));
    }
    strip_show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait)
{
  uint32_t i;
  uint16_t j, q;

  for (j = 0; j < 10; j++) { //do 10 cycles of chasing
    for (q = 0; q < 3; q++) {
      for (i = 0; i < STRIP_NUMPIXELS; i = i + 3) {
        strip_setPixelColor(i + q, c); //turn every third pixel on
      }
      strip_show();

      delay(wait);

      for (i = 0; i < STRIP_NUMPIXELS; i = i + 3) {
        strip_setPixelColor(i + q, 0); //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait)
{
  uint32_t i;
  uint16_t j, q;

  for (j = 0; j < 256; j++) { // cycle all 256 colors in the wheel
    for (q = 0; q < 3; q++) {
      for (i = 0; i < STRIP_NUMPIXELS; i = i + 3) {
        strip_setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      }
      strip_show();

      delay(wait);

      for (i = 0; i < STRIP_NUMPIXELS; i = i + 3) {
        strip_setPixelColor(i + q, 0); //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip_color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip_color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip_color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

//////////////////////////////// TOM NEOPIXEL //////////////////////////////////////////////////

void introRed()
{
  uint32_t i;
  uint16_t u;

  for (u = 0; u < 3; u++) {
    for (i = 0; i < STRIP_NUMPIXELS; i++) {
      oneRed(i);
      delay(100);
    }
    for (i = STRIP_NUMPIXELS - 2; i > 0 ; i--) {
      oneRed(i);
      delay(100);
    }
  }
  /* STRIP */// oneRed(STRIP_NUMPIXELS); // switch all off (switch on only 8, but beyond range)
  for (i = 0; i < STRIP_NUMPIXELS; i++) {
    strip_setPixelColor(i, 0, 0, 0);
  }
}

void oneRed(uint32_t u)
{
  uint32_t i;

  for (i = 0; i < STRIP_NUMPIXELS; i++) {
    strip_setPixelColor(i, 0, 0, 0);
  }
  strip_setPixelColor(u, 255, 0, 0);
  strip_show();
}

void allRGB(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t i;

  for (i = 0; i < STRIP_NUMPIXELS; i++) {
    strip_setPixelColor(i, r, g, b);
  }
  strip_show();
}
