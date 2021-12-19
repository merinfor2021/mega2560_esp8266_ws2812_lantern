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
    " <meta name='viewport' content='width=device-width, height=device-height, initial-scale=1.0, user-scalable=yes' />"
   " </head>"
   " <body style='width:100%; height:100%;'>"
    " <div style='width:100%; height:100%;'>"
     " <script>"
      " function toggleDetails() {"
       " var elem = document.getElementById('my_details');"
       " if (elem.open == true) {"
        " elem.open = false;"
       " }"
       " else {"
        " elem.open = true;"
       " }"
      " }"
          " var stripLocked = false;"
          " function updateStrip(val_r, val_g, val_b) {"
           " if (stripLocked) {"
            " return;"
           " }"
           " function colourCorrect(v) {"
            " return Math.round(1023-(v*v)/64);"
           " }"
           " var params = ["
            " 'r=' + colourCorrect(val_r),"
            " 'g=' + colourCorrect(val_g),"
            " 'b=' + colourCorrect(val_b)"
            " ].join('&');"
           " var req = new XMLHttpRequest();"
           " req.open('POST', '?' + params, true);"
           " req.send();"
           " stripLocked = true;"
           " req.onreadystatechange = function() {"
            " if (req.readyState == 4) {"
             " stripLocked = false;"
            " }"
           " }"
          " }"
      " function updateSummaryColor() {"
       " var val_r = document.getElementById('range_r').value;"
       " my_r =            (parseInt(val_r, 10 )).toString(16).padStart(2, '0');"
       " my_r_inv = (255 - (parseInt(val_r, 10))).toString(16).padStart(2, '0');"
       " document.getElementById('text_r').value = my_r;"
       " var val_g = document.getElementById('range_g').value;"
       " my_g =            (parseInt(val_g, 10 )).toString(16).padStart(2, '0');"
       " my_g_inv = (255 - (parseInt(val_g, 10))).toString(16).padStart(2, '0');"
       " document.getElementById('text_g').value = my_g;"
       " var val_b = document.getElementById('range_b').value;"
       " my_b =            (parseInt(val_b, 10 )).toString(16).padStart(2, '0');"
       " my_b_inv = (255 - (parseInt(val_b, 10))).toString(16).padStart(2, '0');"
       " document.getElementById('text_b').value = my_b;"
       " var my_details = document.getElementById('my_details');"
       " my_details.style.background = '#' + my_r + my_g + my_b;"
       " my_details.style.color      = '#' + my_r_inv + my_g_inv + my_b_inv;"
         " var my_debug_background = document.getElementById('my_debug_background');"
         " my_debug_background.textContent = '#' + my_r + my_g + my_b;"
         " var my_debug_color =      document.getElementById('my_debug_color');"
         " my_debug_color.textContent      = '#' + my_r_inv + my_g_inv + my_b_inv;"
       " updateStrip(parseInt(val_r, 10 ), parseInt(val_g, 10 ), parseInt(val_b, 10 ));"  
      " }"
      " function changedText_r() {"
       " var range_r = document.getElementById('range_r');"
       " var text_r  = document.getElementById('text_r');"
       " range_r.value = parseInt(text_r.value, 16);"
       " updateSummaryColor();"
      " }"
      " function changedText_g() {"
       " var range_g = document.getElementById('range_g');"
       " var text_g  = document.getElementById('text_g');"
       " range_g.value = parseInt(text_g.value, 16);"
       " updateSummaryColor();"
      " }"
      " function changedText_b() {"
       " var range_b = document.getElementById('range_b');"
       " var text_b  = document.getElementById('text_b');"
       " range_b.value = parseInt(text_b.value, 16);"
       " updateSummaryColor();"
      " }"
      " // window.onload = updateSummaryColor;"
     " </script>"
     " <button id='my_toggler' onclick='toggleDetails()' style='width:95%; height:100%;'>"
      " <details id='my_details' onclick='toggleDetails()' style='width:100%; height:100%; background:#ffffff; color:#000000; font-family: \"Courier New\", Courier, monospace; font-weight:bold;'>"
       " <summary id='my_summary'>Palette</summary>"
       " <iframe style='width:100%; height:100%;' srcdoc=\""
        " <body style='margin: 0px; padding: 0px;' style='width:100%; height:100%;'>"
         " <canvas id='colorspace' style='width:100%;'></canvas>"
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
           " my_r =            (parseInt(data[0], 10 )).toString(16).padStart(2, '0');"
           " my_r_inv = (255 - (parseInt(data[0], 10))).toString(16).padStart(2, '0');"
           " my_g =            (parseInt(data[1], 10 )).toString(16).padStart(2, '0');"
           " my_g_inv = (255 - (parseInt(data[1], 10))).toString(16).padStart(2, '0');"
           " my_b =            (parseInt(data[2], 10 )).toString(16).padStart(2, '0');"
           " my_b_inv = (255 - (parseInt(data[2], 10))).toString(16).padStart(2, '0');"
           " function drawCoordinates(x,y) {"
            " var ctx = document.getElementById('colorspace').getContext('2d');"
            " var pointSize = 3;"
            " ctx.fillStyle = '#' + my_r_inv + my_g_inv + my_b_inv;"
            " ctx.beginPath();"
            " ctx.arc(x, y, pointSize, 0, Math.PI * 2, true);"
            " ctx.fill();"
           " }"
           " drawCanvas();"
           " drawCoordinates(clientX, clientY);"
           " var my_details = parent.document.getElementById('my_details');"
           " my_details.style.background = '#' + my_r + my_g + my_b;"
           " my_details.style.color      = '#' + my_r_inv + my_g_inv + my_b_inv;"
           " function updateSliders(val_r, val_g, val_b) {"
            " parent.document.getElementById('range_r').value = val_r;"
            " parent.document.getElementById('text_r').value = (parseInt(val_r, 10 )).toString(16).padStart(2, '0');"
            " parent.document.getElementById('range_g').value = val_g;"
            " parent.document.getElementById('text_g').value = (parseInt(val_g, 10 )).toString(16).padStart(2, '0');"
            " parent.document.getElementById('range_b').value = val_b;"
            " parent.document.getElementById('text_b').value = (parseInt(val_b, 10 )).toString(16).padStart(2, '0');"
           " }"
           " updateSliders(parseInt(data[0], 10 ), parseInt(data[1], 10 ), parseInt(data[2], 10 ));"
             " var my_debug_background = parent.document.getElementById('my_debug_background');"
             " my_debug_background.textContent = '#' + my_r + my_g + my_b;"
             " var my_debug_color = parent.document.getElementById('my_debug_color');"
             " my_debug_color.textContent      = '#' + my_r_inv + my_g_inv + my_b_inv;"
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
           " var rect = canvas.getBoundingClientRect();"
           " var my_x = event.clientX - rect.left;"
           " var my_y = event.clientY - rect.top;"
           " handleEvent(my_x, my_y, true);"
           " }, false);"
          " canvas.addEventListener('touchmove', function(event) {"
           " var rect = canvas.getBoundingClientRect();"
           " var my_x = event.touches[0].clientX - rect.left;"
           " var my_y = event.touches[0].clientY - rect.top;"
           " handleEvent(my_x, my_y);"
           " }, false);"
          " function resizeCanvas() {"
           " var rect = canvas.getBoundingClientRect();"
           " canvas.width  = window.innerWidth - rect.left;"
           " canvas.height = window.innerHeight - rect.top - 4;"
           " drawCanvas();"
          " }"
          " window.addEventListener('resize', resizeCanvas, false);"
          " resizeCanvas();"
          " drawCanvas();"
          " document.ontouchmove = function(e) {e.preventDefault()};"
         " })();"
        " </script>"
       "\">"
       " </iframe>"
      " </details>"
     " </button>"
     " <br>"
     " <div style='background:#ff0000; color:#ff0000;'>"
      " <input type='range' id='range_r' name='range_r' min='0' max='255'     value='255' style='width:80%; height:100%;'                                      onchange='updateSummaryColor();'>"
      " <input type='text'  id='text_r'  name='text_r' size='2' maxlength='2' value='ff'  style='font-family: \"Courier New\", Courier, monospace; font-weight:bold;' onchange='changedText_r();'>"
      " <br>"
     " </div>"
     " <div style='background:#00ff00; color:#00ff00;'>"
      " <input type='range' id='range_g' name='range_g' min='0' max='255'   value='255' style='width:80%; height:100%;'                                        onchange='updateSummaryColor();'>"
      " <input type='text'  id='text_g'  name='text_g' size='2' maxlength='2' value='ff'  style='font-family: \"Courier New\", Courier, monospace; font-weight:bold;' onchange='changedText_g();'>"
      " <br>"
     " </div>"
     " <div style='background:#0000ff; color:#0000ff;'>"
      " <input type='range' id='range_b' name='range_b' min='0' max='255'     value='255' style='width:80%; height:100%;'                                      onchange='updateSummaryColor();'>"
      " <input type='text'  id='text_b'  name='text_b' size='2' maxlength='2' value='ff'  style='font-family: \"Courier New\", Courier, monospace; font-weight:bold;' onchange='changedText_b();'>"
      " <br>"
     " </div>"
     " <div>"
      " <button id='my_debug_background' style='font-family: \"Courier New\", Courier, monospace; font-weight:bold;'>my_debug_background</button>"
      " <br>"
      " <button id='my_debug_color'      style='font-family: \"Courier New\", Courier, monospace; font-weight:bold;'>my_debug_color</button>"
     " </div>"
    " </div>"
   " </body>"
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
