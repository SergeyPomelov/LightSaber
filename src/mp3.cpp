#include <Arduino.h>

#define DEBUG_SERIAL 1

#define SERIAL_RX D5  // Software Serial RX
#define SERIAL_TX D2  // Software Serial TX
#define SERIAL2_RX D3 // Software Serial2 RX
#define SERIAL2_TX D4 // Software Serial2 TX

#include <SoftwareSerial.h> // связь с mp3 плеером по последовательному порту
#include <MD_YX5300.h>      // mp3 плеер

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
MD_YX5300 mp3effect(swSerial);
MD_YX5300 mp3bg(sw2Serial);

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

void mp3setup()
{
  swSerial.begin(MD_YX5300::SERIAL_BPS);
  sw2Serial.begin(MD_YX5300::SERIAL_BPS);
  Serial.println("mp3");
  mp3effect.begin();
  Serial.println("mp3effect");
  mp3bg.begin();
  Serial.println("mp3bg");
  mp3effect.setSynchronous(false);
  mp3bg.setSynchronous(false);
  mp3effect.volume(100U);
  mp3bg.volume(250U);
}

void mp3tick()
{
  mp3effect.check();
  mp3bg.check();
  const uint16_t code = mp3effect.getStsCode();
  const uint16_t data = mp3effect.getStsData();

  // Serial.printf("%d %d \n", code, data);
  if (code == MD_YX5300::STS_FILE_END || code == MD_YX5300::STS_ERR_FILE)
  {
    // Serial.println(mp3effect.getStatus()->code);
    if (playing != 0)
    {
      Serial.println("STS_FILE_END");
      playing = 0;
    }
  }

  if (code == MD_YX5300::STS_STATUS)
  {
    // Serial.println(mp3effect.getStatus()->code);
    byte lower = (byte)(data & 0xff);
    if (lower != (byte)0x01 && playing != 0)
    {
      Serial.println("STS_STATUS NOT PLAY");
      playing = 0;
    }
  }
  if (millis() % 1000 == 0)
  {
    Serial.print("mp3effect.getStsCode() ");
    Serial.println(mp3effect.getStsCode());
    Serial.print("mp3bg.getStsCode() ");
    Serial.println(mp3bg.getStsCode());
  }
}

void startBgSound()
{
  Serial.print("startBgSound ");
  mp3bg.playSpecific(1U, 1U);
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
    mp3effect.playSpecific(folder, sound);
    playing = playingEffectType;
  }
}

void playOnEffect()
{
  playEffect(FLD_SOUND_ON, random(1U, numSoundOn), SOUND_ON, true);
}

void playOffEffect()
{
  mp3bg.playStop();
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
