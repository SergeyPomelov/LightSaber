#include <Arduino.h>

#define DEBUG_SERIAL 1

#define SERIAL_RX D5  // Software Serial RX
#define SERIAL_TX D2  // Software Serial TX
#define SERIAL2_RX D3 // Software Serial2 RX
#define SERIAL2_TX D4 // Software Serial2 TX

#include <SoftwareSerial.h> // связь с mp3 плеером по последовательному порту
#include <DFRobotDFPlayerMini.h> // mp3 плеер

#define SOUND_ON 1
#define SOUND_OFF 2
#define SOUND_SETUP 3
#define SOUND_STRIKE 4
#define SOUND_SHORT_SWING 5
#define SOUND_SWING 6
#define SOUND_GENERIC 7

#define FLD_BG_SABER 1 // sd:/01/

#define FLD_SOUND_ON 1    // sd:/01/
#define FLD_SOUND_OFF 2   // sd:/02/
#define FLD_STRIKE_1 3    // sd:/03/
#define FLD_LONG_SWING 6  // sd:/06/
#define FLD_SHORT_SWING 7 // sd:/07/

SoftwareSerial swSerial(SERIAL_RX, SERIAL_TX);
SoftwareSerial sw2Serial(SERIAL2_RX, SERIAL2_TX);
DFRobotDFPlayerMini mp3effect;
DFRobotDFPlayerMini mp3bg;

byte
    numBgSaber = 7,
    numSoundOn = 1,
    numSoundOff = 1,
    numStrike1 = 16,
    numStrike2 = 13,
    numStrike3 = 8,
    numLongSwing = 4,
    numShortSwing = 5;

byte playing = 0;

void startBgSound()
{
  Serial.print("startBgSound ");
  mp3bg.playFolder(1U, 1U);
}

void mp3setup()
{
  swSerial.begin(9600U);
  sw2Serial.begin(9600U);
  Serial.println("mp3");
  mp3effect.begin(swSerial, false, false);
  mp3effect.volume(10U);
  Serial.println("mp3effect");
  mp3bg.begin(sw2Serial, false, false);
  mp3bg.volume(30U);
  Serial.println("mp3bg");
}

void mp3tick()
{
  uint8_t effectCode = 0;
  uint16_t effectData = 0;
  if (mp3effect.available())
  {
    effectCode = mp3effect.readType();
    effectData = mp3effect.read();
  }

  uint8_t bgCode = 0;
  uint16_t bgData = 0;
  if (mp3bg.available())
  {
    bgCode = mp3effect.readType();
    bgData = mp3effect.read();
  }

  // Serial.printf("%d %d \n", code, data);
  if (bgCode == DFPlayerPlayFinished || bgCode == DFPlayerError)
  {
    startBgSound();
  }

  if (bgCode == -1)
  {
    // Serial.println(mp3effect.getStatus()->code);
    byte lower = (byte)(bgData & 0xff);
    if (lower != (byte)0x010)
    {
      startBgSound();
    }
  }

  if (effectCode == DFPlayerPlayFinished || effectCode == DFPlayerError)
  {
    // Serial.println(mp3effect.getStatus()->code);
    if (playing != 0)
    {
      Serial.println("STS_FILE_END");
      playing = 0;
    }
  }

  if (effectCode == -1)
  {
    // Serial.println(mp3effect.getStatus()->code);
    byte lower = (byte)(effectData & 0xff);
    if (lower != (byte)0x01 && playing != 0)
    {
      Serial.println("STS_STATUS NOT PLAY");
      playing = 0;
    }
  }
  if (millis() % 1000 == 0 || true)
  {
    Serial.print("mp3effect.getStsCode() ");
    Serial.println(effectCode);
    Serial.print("mp3bg.getStsCode() ");
    Serial.println(bgCode);
  }
}

void playEffect(uint folder, uint sound, uint playingEffectType, boolean condition = false)
{
  if (playing == 0 || condition)
  {
    Serial.print("mp3effect.playSpecific ");
    Serial.print(folder);
    Serial.print(" ");
    Serial.println(sound);
    Serial.print(" ");
    Serial.println(playingEffectType);
    mp3effect.playFolder(folder, sound);
    playing = playingEffectType;
  }
}

void playOnEffect()
{
  playEffect(FLD_SOUND_ON, random(1U, numSoundOn), SOUND_ON, true);
}

void playOffEffect()
{
  mp3bg.stop();
  playEffect(FLD_SOUND_OFF, random(1U, numSoundOff), SOUND_OFF, true);
}

void playStrikeEffect()
{
  playEffect(FLD_STRIKE_1, random(1U, numStrike1), SOUND_STRIKE, true);
}

void playSwingEffect()
{
  playEffect(FLD_LONG_SWING, random(1U, numLongSwing), SOUND_SWING, playing == SOUND_SHORT_SWING);
}

void playShortSwingEffect()
{
  playEffect(FLD_SHORT_SWING, random(1U, numShortSwing), SOUND_SHORT_SWING);
}
