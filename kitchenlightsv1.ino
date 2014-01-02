#include <Adafruit_NeoPixel.h>

#define PIN 6
//#define micPIN A0
#define bPIN 5

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(6, PIN, NEO_GRB + NEO_KHZ800);
//String stringBuffer;

//bool toggle = false;
bool enableDemoMode = true;

//int micVal=0;
//int micBuffer[8];
//int micBufferPos=0;

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

//  Serial.begin(9600);
  //  pinMode(micPIN,INPUT);
  pinMode(bPIN, INPUT);


  // look for button held on boot
  if (digitalRead(bPIN))
  {
    // debounce
    delay(20);
    if (digitalRead(bPIN))
    {
      // solid press!
      enableDemoMode = false;
    }
  }

}

void loop() {

  if (enableDemoMode)
  {
    demoMode();
  } else {
    clearAllPixels();
    delay(1000);
    whiteFadeUp();
    delay(5000);
    clearAllPixels();
  }

}

//void pushButtonMode()
//{
//
//  //button push
//  if (digitalRead(bPIN))
//  {
//    // debounce
//    delay(20);
//    if (digitalRead(bPIN))
//    {
//      // solid press!
//      toggle = !toggle;
//      Serial.println("button!");
//      Serial.println("toggle!");
//      while (digitalRead(bPIN)) { };
//    }
//  }
//}

void demoMode()
{
  //Some example procedures showing how to display to the pixels:
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  colorWipe(strip.Color(255, 0, 0), 50); // Red
  colorWipe(strip.Color(0, 255, 0), 50); // Green
  colorWipe(strip.Color(0, 0, 255), 50); // Blue
  rainbow(10);
  rainbowCycle(10);
  whiteFadeUp();
  whiteFadeDown();
}

//void micRoutines() {
//
//micVal = analogRead(micPIN);
//micBuffer[micBufferPos] = micVal;
//micBufferPos++;
//if(micBufferPos>=8) micBufferPos=0;
//
//int micAvg = 0;
//int micMax = 512;
//int micMin = 512;
//for(int i = 0; i < 8; i++)
//{
//  micAvg = (micAvg + micBuffer[i])/2;
//  if (micBuffer[i] > micMax) micMax = micBuffer[i];
//  if (micBuffer[i] < micMin) micMin = micBuffer[i];
//}
//
//stringBuffer = String("Min: ");
//stringBuffer += micMin;
//stringBuffer += " Max:";
//stringBuffer += micMax;
//stringBuffer += " Avg:";
//stringBuffer += micAvg;
//
//Serial.println(stringBuffer);
//delay(10);
//
//}

void whiteFadeUp()
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();

  for (int b = 0; b <= 255; b++)
  {
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, b, b, b);
    }
    strip.show();
    delay(10);
  }
}

void whiteFadeDown()
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();

  for (int b = 255; b >= 0; b--)
  {
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
      strip.setPixelColor(i, b, b, b);
    }
    strip.show();
    delay(10);
  }
}

void clearAllPixels()
{
  colorWipe(strip.Color(0, 0, 0), 1);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) {
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}


