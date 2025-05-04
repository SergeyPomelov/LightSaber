#include <Arduino.h>

#define NUM_LEDS    47
#define BRIGHTNESS  255

#define FLASH_DELAY 80

#define BLINK_ALLOW   1
#define BLINK_AMPL    10
#define BLINK_DELAY   50

#define EFFECT_STANDBY  1
#define EFFECT_ON       2
#define EFFECT_OFF      3
#define EFFECT_BLINKING 4
#define EFFECT_STRIKE   5

byte currentColor = 5;
byte currentEffect = 0;
unsigned long effectStart = 0;

#define LED_PIN D0

#include "Wire.h"
#include "FastLED.h"

CRGB leds[NUM_LEDS];

unsigned long
    nowLed,
    blinkTimer,
    batteryTimer,
    remindToOff = 0,
    ledTime = 0,
    offTimer;

byte
    redOffset,
    greenOffset,
    blueOffset;

int blinkOffset;
CRGB nowRGB;
float k = 0.2;
byte nowSaberColor = 5;

CRGB COLORS[8] = {0xE00000, 0xB03000, 0x805000, 0x00E000, 0x005060, 0x000016, 0x400090, 0xB0B0B0};

void blink();
void randomBlink();
void setColor(byte color);
void setPixel(int pixel, CRGB rgb);
void setAll(CRGB rgb) ;
void standbyBlinkTick();
void randomBlinkTick();
void lightUpTick();
void lightDownTick();
void strikeFlashTick();

void effect(byte newEffect)
{
  currentEffect = newEffect;
  effectStart = millis();
  FastLED.show();
}

void effectEnd()
{
  effectStart = 0;
  currentEffect = EFFECT_BLINKING;
  FastLED.show();
}

void ledsetup()
{

  pinMode(LED_PIN, OUTPUT);
  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.show();

  setAll(0);
  setColor(currentColor);
  FastLED.show();
}

void cycleColor() {
  if (currentColor > 7) {
    currentColor  = 0;
    setColor(currentColor);
  } else {
    setColor(currentColor++);
  }
}

void ledtick()
{
  nowLed = millis();
  standbyBlinkTick();
  randomBlinkTick();
  lightUpTick();
  lightDownTick();
  strikeFlashTick();
  if (nowLed % 100 == 0) {
    //Serial.print("currentEffect ");
    //Serial.println(currentEffect);
  }
}

void standbyBlink() {
  if (currentEffect != EFFECT_OFF)
  {
    currentEffect = EFFECT_STANDBY;
  }
}

void standbyBlinkTick()
{
  if (currentEffect != EFFECT_STANDBY)
  { 
    return;
  }
  static bool ledOn = 0;
  bool toOn = nowLed % 1000 < 300;
  if (ledOn != toOn)
  {
    ledOn = toOn;
    setAll(0);
    if (ledOn)
    {
      setPixel(0, 0x000050);
    }
    FastLED.show();
  }
}

void randomBlink()
{
  if (currentEffect == EFFECT_STANDBY) {
    currentEffect = EFFECT_BLINKING;
  }
}

void randomBlinkTick()
{
  if (currentEffect != EFFECT_BLINKING)
  {
    return;
  }
  if (BLINK_ALLOW && (millis() - blinkTimer > BLINK_DELAY))
  {
    blinkTimer = millis();
    blinkOffset = blinkOffset * k + random(-BLINK_AMPL, BLINK_AMPL) * (1 - k);
    if (nowSaberColor == 0)
      blinkOffset = constrain(blinkOffset, -15, 5);
    redOffset = constrain(nowRGB.red + blinkOffset, 0, BRIGHTNESS);
    greenOffset = constrain(nowRGB.green + blinkOffset, 0, BRIGHTNESS);
    blueOffset = constrain(nowRGB.blue + blinkOffset, 0, BRIGHTNESS);
    setAll(CRGB(redOffset, greenOffset, blueOffset));
  }
}

void setPixel(int pixel, CRGB rgb)
{
  leds[pixel] = rgb;
}

void setAll(CRGB rgb)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = rgb;
  }
  FastLED.show();
}

void lightUp()
{
  effect(EFFECT_ON);
}

void lightUpTick()
{
  if (currentEffect != EFFECT_ON)
  {
    return;
  }
  int ledsOn = (nowLed - effectStart) / 10;
  for (int i = 0; i < ledsOn; i++)
  {
    setPixel(i, nowRGB);
  }
  FastLED.show();
  if (ledsOn >= NUM_LEDS)
  {
    effectEnd();
  }
}

void lightDown()
{
  effect(EFFECT_OFF);
}

void lightDownTick()
{
  if (currentEffect != EFFECT_OFF)
  {
    return;
  }
  int ledsOff = (nowLed - effectStart) / 10;
  for (int i = NUM_LEDS - 1; i >= NUM_LEDS - ledsOff; i--)
  {
    setPixel(i, CRGB::Black);
  }
  // Serial.println(ledsOff);
  FastLED.show();
  if (ledsOff >= NUM_LEDS) {
    effectEnd();
    currentEffect = EFFECT_STANDBY;
  }
}

void strikeFlash()
{
  effect(EFFECT_STRIKE);
  Serial.print("nowLed ");
  Serial.println(nowLed);
  Serial.print("effectStart ");
  Serial.println(effectStart);
}

void strikeFlashTick()
{
  if (currentEffect != EFFECT_STRIKE)
  {
    return;
  }
  if (nowLed - effectStart < FLASH_DELAY)
  {
    setAll(0x111111);
    FastLED.show();
  }
  else
  {
    effectEnd();
    setAll(nowRGB);
  }
}

void setColor(byte color)
{
  nowRGB = COLORS[color];
}
