#include <Arduino.h>

#define CYCLE_INC(x, count) \
  x++;                      \
  x %= count
#define CYCLE_DEC(x, count) \
  x += count - 1;           \
  x %= count

// ---------------------------- НАСТРОЙКИ -------------------------------
#define NUM_LEDS 50         // число МИКРОСХЕМ на ленте
#define BRIGHTNESS 255      // максимальная яркость ленты (0 - 255)

#define FLASH_DELAY 80      // время вспышки при ударе (миллисекунды)

#define BLINK_ALLOW 1       // разрешить мерцание (1 - разрешить, 0 - запретить)
#define BLINK_AMPL 20       // амплитуда мерцания клинка
#define BLINK_DELAY 30      // задержка между мерцаниями

#define LED_PIN D8          // DIN ленты

#include "Wire.h"           // вспомогательная библиотека для работы с акселерометром
#include "FastLED.h"        // библиотека для работы с адресной лентой

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

CRGB COLORS[8] = {0xE00000, 0xB03000, 0x805000, 0x00E000, 0x005060, 0x0000E0, 0x400090, 0xB0B0B0};

void blink(CRGB color);
void randomBlink();
void setColor(byte color);
void setPixel(int pixel, CRGB rgb);
void setAll(CRGB rgb) ;

void ledsetup()
{

  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.show();

  setAll(0);
  setColor(5);
  FastLED.show();
}

void ledtick(bool isOn)
{
  nowLed = millis();
  if (isOn)
  {
    randomBlink();
  } else {
    blink(0x000050);
  }
}

void blink(CRGB color)
{
  static bool ledOn = 0;
  bool toOn = nowLed % 1000 < 300;
  if (ledOn != toOn) {
    ledOn = toOn;
    setAll(0);
    if (ledOn) {
      setPixel(0, color);
    }
    FastLED.show();
  }
}

void randomBlink()
{
  if (BLINK_ALLOW && (millis() - blinkTimer > BLINK_DELAY))
  {
    blinkTimer = millis();
    blinkOffset = blinkOffset * k + random(-BLINK_AMPL, BLINK_AMPL) * (1 - k);
    if (nowSaberColor == 0)
      blinkOffset = constrain(blinkOffset, -15, 5);
    redOffset = constrain(nowRGB.red + blinkOffset, 0, 255);
    greenOffset = constrain(nowRGB.green + blinkOffset, 0, 255);
    blueOffset = constrain(nowRGB.blue + blinkOffset, 0, 255);
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
  for (int i = 0; i < NUM_LEDS; i++)
  {
    setPixel(i, nowRGB);
    FastLED.show();
    delay(10);
  }
}

void lightDown()
{
  for (int i = NUM_LEDS - 1; i >= 0; i--)
  {
    setPixel(i, CRGB::Black);
    FastLED.show();
    delay(10);
  }
}

void strikeFlash()
{
  setAll(CRGB::White); // цвет клинка белым
  delay(FLASH_DELAY);  // ждать
  setAll(nowRGB);      // цвет клинка старым цветом
}

void setColor(byte color)
{
  nowRGB = COLORS[color];
}
