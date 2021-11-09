//// [000.E662FE][001.E662FE][002.E662FE][003.E662FE][004.E662FE][005.E662FE][006.E662FE][007.E662FE]!

#include <Adafruit_NeoPixel.h>

#define PIN   48
#define N_LEDS 8

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

String srl_string = "";
String led_string = "";
String rgb_string = "";

uint32_t i   = 0;
uint32_t led = 0;
uint32_t rgb = 0;

uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;

char incoming_chr = "";

uint8_t command = 0;

// Initial setup
void setup()
{
  Serial.begin(115200); // Initialize the debug serial port
 /*
  * It is recommended to use the lower baud rate. The higher the baud rate,
  * the higher the bit error rate will be, and the control action might fail.
  */
  Serial3.begin(4800); // Initialize the ESP8266 <=> MEGA2560 serial port
  strip.begin();
  //// pinMode(LED_BUILTIN, OUTPUT); // Initialize the digital pin LED_BUILTIN as an output
}

// Read a command from ESP8266 over serial
void read_string()
{
  while (Serial3.available() > 0) {
    incoming_chr = Serial3.read();
    srl_string += incoming_chr;
    //// Serial.print(incoming_chr);
    switch (incoming_chr) {
      case '[':
        command = 1;
        led_string = "";
        rgb_string = "";
        break;
      case ']':
        command = 0;
        if ((led_string.length() == 3) && (rgb_string.length() == 6)) {
          rgb = (uint32_t) strtol(&rgb_string[0], NULL, 16);
          r = rgb >> 16;
          g = rgb >> 8 & 0xFF;
          b = rgb & 0xFF;
          if ((led_string[0] == "A") && (led_string[1] == "L") && (led_string[2] == "L")) {
            for (i=0; i<N_LEDS; i++) {
              strip.setPixelColor(i, r, g, b);
            }
          }
          else {
            led = (uint32_t) strtol( &led_string[0], NULL, 16);
            if (led < N_LEDS) {
              strip.setPixelColor(led, r, g, b);
            }
          }
        }
        led_string = "";
        rgb_string = "";
        srl_string = "";
        break;
      case '.':
        if (command == 1) {
          command = 2;
          rgb_string = "";
        }
        break;
      case '!':
        strip.show();
        break;
      default:
        if (command == 1) {
          led_string += incoming_chr;
        }
        if (command == 2) {
          rgb_string += incoming_chr;
        }
    }
  }
}

// The loop function runs over and over again forever
void loop()
{
  if (Serial3.available() > 0) {
    read_string();
  }
}