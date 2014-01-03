// Doug Barry 20140103203200

#include <Adafruit_NeoPixel.h>

//#define DEBUG

// from http://forum.arduino.cc/index.php?topic=46900.0
#ifdef DEBUG
#define DEBUG_PRINT(x)     Serial.print (x)
#define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTDEC(x)
#define DEBUG_PRINTLN(x)
#endif

#define NOOP() __asm__("nop\n\t");

// Number of pixels in your string
#define PIXEL_COUNT 6

// PIN connected to the DI/DO of the pixel string
#define PIN_PIXEL_DO 6
// PIN connected to the push button for mode selection
#define PIN_PUSH_BUTTON 5


#define PIXEL_MODE_COUNT 4

#define WHITE_LEVEL_MAX 255

#define UPDATE_DELAY 10


// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIN_PIXEL_DO, NEO_GRB + NEO_KHZ800);

typedef void (* PixelModeServiceFuncPtr) ();
// holdwhiteservice must be position 1;
PixelModeServiceFuncPtr pixelModeServiceFunctions[PIXEL_MODE_COUNT] = { fadeUpToWhiteService, holdWhiteService, rainbowService, rainbowCycleService };
PixelModeServiceFuncPtr pixelModeService;

/*mode ideas, reversible in direction, and invertable in colours
hourglass drop out
hourglass full up
colour flow from ends
colour flow from center
colours sway
ping pong (single coloured led moving, or non-illuminated led moving)
flash colour
pulse colour (fade up, fade down)
*/

uint16_t currentPixelMode = 0;

// always a 16 bit number
// need to make sure this is acconted for in routines using it that may expect an 8 bit uint
uint16_t pixelModeCycleIndex = 0;

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  pinMode(PIN_PUSH_BUTTON, INPUT);

  setModeServiceRoutine();

#ifdef DEBUG
  Serial.begin(9600);
#endif

}

void loop() {

  // check for stimulous
  if (stimulousInput())
  {
    DEBUG_PRINTLN("button press");
    DEBUG_PRINT("Current mode: ");
    DEBUG_PRINTLN(currentPixelMode);
    // change pixel mode if necessary
    modeCycle();
    DEBUG_PRINT("New mode: ");
    DEBUG_PRINTLN(currentPixelMode);
  }

  // service current pixel mode
  modeService();

}

void modeService()
{
  // handle general purpose counter and run current pixel mode service routine
  // should probably take care of some timing here too at some point (to handle debouncing race condition)

  pixelModeCycleIndex += 2;

  //  if((pixelModeCycleIndex % 64) == 0)
  //  {
  //    Serial.println(pixelModeCycleIndex);
  //  }

  pixelModeService();

  // wait an appropriate amount of time
  delay(UPDATE_DELAY);
}

bool stimulousInput()
{
  if (digitalRead(PIN_PUSH_BUTTON))
  {
    // debounce
    modeService();

    if (digitalRead(PIN_PUSH_BUTTON))
    {
      // solid press, need tp wait for button release
      while (digitalRead(PIN_PUSH_BUTTON))
      {
        modeService();
      }
      return true;
    }
  }
}

// could probably macro this
void setModeServiceRoutine()
{
  pixelModeService = pixelModeServiceFunctions[currentPixelMode];
}

void modeCycle()
{
  currentPixelMode++;
  if (currentPixelMode >= PIXEL_MODE_COUNT)
  {
    currentPixelMode = 0;
  }
  setModeServiceRoutine();
}

void holdWhiteService()
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, WHITE_LEVEL_MAX, WHITE_LEVEL_MAX, WHITE_LEVEL_MAX);
  }
  strip.show();
}

void fadeUpToWhiteService()
{
  uint16_t c = pixelModeCycleIndex;

  if (c > WHITE_LEVEL_MAX)
  {
    c = WHITE_LEVEL_MAX;
  }

  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, c, c, c);
  }
  strip.show();

  if (c >= WHITE_LEVEL_MAX)
  {
    // special case, move to holdWhiteService
    currentPixelMode = 1;
    setModeServiceRoutine();
  }

}

void setAllPixelsOff()
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}

void rainbowService() {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i + pixelModeCycleIndex) & 255));
  }
  strip.show();
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycleService() {
  uint16_t i, j;

  //  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + pixelModeCycleIndex) & 255));
  }
  strip.show();
  //}
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


