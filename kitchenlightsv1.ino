// Doug Barry 20140104111600

#include <Adafruit_NeoPixel.h>

#define DEBUG

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

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

#define NOOP() __asm__("nop\n\t")

// Number of pixels in your string
#define PIXEL_COUNT 6

// PIN connected to the DI/DO of the pixel string
#define PIN_PIXEL_DO 6
// PIN connected to the push button for mode selection
#define PIN_PUSH_BUTTON 5

// PIN connected to Brightness pot
#define PIN_BRIGHTNESS_POT 1
// calibration for pot min/max
#define BRIGHTNESS_POT_READING_MAX 1022
#define BRIGHTNESS_POT_READING_MIN 1
#define BRIGHTNESS_READINGS_BUFFER_SIZE 4
// account for log/lin?

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

//PixelModeServiceFuncPtr pixelModeServiceFunctions[] = { fadeUpToWhiteService, holdWhiteService, rainbowService, rainbowCycleService, rainbowCycleService2 }; //, hourGlassDropOutService, colourFlowFromEndService };
PixelModeServiceFuncPtr pixelModeServiceFunctions[] = { knightRiderService };
PixelModeServiceFuncPtr pixelModeService;

#define PIXEL_MODE_COUNT (ARRAY_SIZE(pixelModeServiceFunctions))

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
uint8_t pixelModeCycleIndex = 0;

uint32_t preDefinedPixelColours[] = {
  strip.Color(255, 0, 0),
  strip.Color(0, 255, 0),
  strip.Color(0, 0, 255),
  strip.Color(255, 255, 0),
  strip.Color(255, 128, 0),
  strip.Color(255, 0, 255),
  strip.Color(0, 255, 255),
  strip.Color(0, 128, 255)
};

#define PREDEFINED_PIXELCOLOURS_COUNT (ARRAY_SIZE(preDefinedPixelColours))

//{
//  RED,
//  BLUE,
//  GREEN,
//  YELLOW,
//  ORANGE,
//  PURPLE,
//  PINK
//};

uint8_t currentBrightnessLevel = WHITE_LEVEL_MAX;

int brightnessReadings[BRIGHTNESS_READINGS_BUFFER_SIZE];
uint8_t brightnessReadingsIndex = 0;

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

  updateBrightness();

}

int getBrightnessPot() {
  int brtPot = analogRead(PIN_BRIGHTNESS_POT);
//  DEBUG_PRINT("Brightness pot: ");
//  DEBUG_PRINTLN(brtPot);
  return brtPot;
}

void updateBrightness() {
  int reading = getBrightnessPot();
  float brt = (float)reading / (float)(BRIGHTNESS_POT_READING_MAX - BRIGHTNESS_POT_READING_MIN);

//  DEBUG_PRINT("brt: ");
//  DEBUG_PRINTLN(brt);

  uint8_t brightness = floor((float)WHITE_LEVEL_MAX * brt);

  brightnessReadings[brightnessReadingsIndex] = brightness;
  brightnessReadingsIndex++;

  if (brightnessReadingsIndex > ARRAY_SIZE(brightnessReadings)) brightnessReadingsIndex = 0;

  int brightnessAvg = 0;
  for (int i = 0; i < ARRAY_SIZE(brightnessReadings); i++)
  {
    brightnessAvg = (int)((float)(brightnessAvg + brightnessReadings[i]) / 2);
  }

  currentBrightnessLevel = brightnessAvg; //floor((float)WHITE_LEVEL_MAX * brt);

//  DEBUG_PRINT("currentBrightnessLevel: ");
//  DEBUG_PRINTLN(currentBrightnessLevel);

}

// takes old brightness, converts to dimmed brightness
uint8_t colourBrightness(uint8_t inputBrightness)
{
  return floor(( (float)( (float) currentBrightnessLevel / (float)WHITE_LEVEL_MAX) * (float)inputBrightness));
}

void modeService()
{
  // handle general purpose counter and run current pixel mode service routine
  // should probably take care of some timing here too at some point (to handle debouncing race condition)

  pixelModeCycleIndex++;

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
  pixelModeCycleIndex = 0;
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

void knightRiderService()
{
  setAllPixelsOff(false);

  // max value of pixelModeCycleIndex is 256  
    
    uint16_t i = pixelModeCycleIndex;
    
    //DEBUG_PRINTLN(i);

    int pixelCenterPos = (float)(sin(i/8)+1) * ((float)PIXEL_COUNT/2);
//    DEBUG_PRINTLN(sin(i));
    DEBUG_PRINTLN(pixelCenterPos);
    
    strip.setPixelColor(pixelCenterPos, colourBrightness(255),0,0);
    strip.show();
    
    return;
    
    for(int fadeIndex = 0; fadeIndex <= PIXEL_COUNT/6; fadeIndex ++)
    {
      int pixelNum = 0;
      
      
      pixelNum = fadeIndex + pixelCenterPos;
      
//      DEBUG_PRINTLN(pixelNum);
      
      continue;
      
      if((pixelNum >= 0) || (pixelNum < PIXEL_COUNT))
      {
        int pixelBrightness = 255 * cos((float) (PIXEL_COUNT/6) * 90) * (float)fadeIndex;
        
//        DEBUG_PRINTLN(pixelBrightness);
        
        strip.setPixelColor(pixelNum, colourBrightness(pixelBrightness),0,0);
      }
    }
  
  strip.show();
  
}

void colourFlowFromEndService()
{
  // may be less pixels than there are colours!
  int pColourIndex = (pixelModeCycleIndex >> 4)   % PREDEFINED_PIXELCOLOURS_COUNT;

  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, colourBrightness(preDefinedPixelColours[pColourIndex]));
    pColourIndex++;
    if (pColourIndex > PREDEFINED_PIXELCOLOURS_COUNT)
    {
      pColourIndex = 0;
    }
  }
  strip.show();
}


void hourGlassDropOutService()
{
  setAllPixelsOff(false);
  for (uint16_t i = 0; i < strip.numPixels() - (pixelModeCycleIndex >> 5); i++)
  {
    strip.setPixelColor(i, colourBrightness(WHITE_LEVEL_MAX), colourBrightness(WHITE_LEVEL_MAX), colourBrightness(WHITE_LEVEL_MAX));
  }
  strip.show();
}



void holdWhiteService()
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, colourBrightness(WHITE_LEVEL_MAX), colourBrightness(WHITE_LEVEL_MAX), colourBrightness(WHITE_LEVEL_MAX));
  }
  strip.show();
}

void fadeUpToWhiteService()
{
  uint16_t c = pixelModeCycleIndex;

  if (c > 255)
  {
    c = 255;
  }

  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, colourBrightness(c), colourBrightness(c), colourBrightness(c));
  }
  strip.show();

  if (c >= 255)
  {
    // special case, move to holdWhiteService
    currentPixelMode = 1;
    setModeServiceRoutine();
  }

}

void setAllPixelsOff(bool flushToStrip)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, 0, 0, 0);
  }
  if (flushToStrip) strip.show();
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

// variant
void rainbowCycleService2() {
  uint16_t i, j;

  //  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel2(((i * 256 / strip.numPixels()) + pixelModeCycleIndex) & 255));
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

uint32_t Wheel2(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
