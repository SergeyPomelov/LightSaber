#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <GyverButton.h>
#include <mp3.h>
#include <LED.h>

#define DEBUG_SERIAL 1

#ifdef DEBUG_SERIAL
#define LOG(x) Serial.println(x)
#define LOG2(x, y) \
  Serial.print(x); \
  Serial.println(y)
#else
#define LOG(x)
#define LOG2(x, y)
#endif

#define CYCLE_INC(x, count) \
  x++;                      \
  x %= count
#define CYCLE_DEC(x, count) \
  x += count - 1;           \
  x %= count

// ---------------------------- НАСТРОЙКИ -------------------------------
#define NUM_LEDS 50    // число МИКРОСХЕМ на ленте
#define BRIGHTNESS 255 // максимальная яркость ленты (0 - 255)

#define SWING_THR 300   // порог угловой скорости для взмаха
#define SWING_S_THR 400 // порог угловой скорости для сильного взмаха
#define STRIKE_THR 80  // порог ускорения для распознавания удара
#define FLASH_DELAY 80  // время вспышки при ударе (миллисекунды)

#define BLINK_ALLOW 1  // разрешить мерцание (1 - разрешить, 0 - запретить)
#define BLINK_AMPL 20  // амплитуда мерцания клинка
#define BLINK_DELAY 30 // задержка между мерцаниями

#define R1 470         // сопротивление резистора делителя (kOm)
#define BATTERY_THR 10 // порог заряда АКБ в %, если 0, то не выключаться до конца

// ---------------------------- ПИНЫ -------------------------------
#define BTN1_PIN D5 // пин кнопки 1

#define VOLT_PIN A0 // пин вольтметра
#define LED_PIN D8  // DIN ленты
#define SDA_PIN D6
#define SCL_PIN D7

// ----------------------------- ОБЪЕКТЫ -------------------------------------
MPU6050 accelgyro;
GButton btn1(BTN1_PIN, HIGH_PULL, NORM_OPEN);

int16_t
    ax,
    ay,
    az,
    gx,
    gy,
    gz;
unsigned long
    ACC,
    GYR,
    COMPL;
int
    gyroX,
    gyroY,
    gyroZ,
    accelX,
    accelY,
    accelZ;

bool
    isOn = 0;

unsigned long
    now,
    mpuTimer;

unsigned long
    userLastActivity = 0;

void readAccel();
void btnTick();
void strikeTick();
void swingTick();
void on();
void off();

void setup()
{
#ifdef DEBUG_SERIAL
  Serial.begin(76800);
  Serial.flush();
#else
#endif

  Wire.begin(SDA_PIN, SCL_PIN);
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_250);

  LOG("MPU6050...");
  if (accelgyro.testConnection())
    LOG("MPU6050 ok");
  else
    LOG("MPU6050 fail");

  mp3setup();
  ledsetup();

  now = millis();
}

void loop()
{
  delay(1);
  now = millis();

  mp3tick();
  ledtick(isOn);
  btnTick();

  if (isOn)
  {
    readAccel();
    strikeTick();
    swingTick();
  }
}

void btnTick()
{
  btn1.tick();
  if (btn1.isClick())
  {
    if (isOn)
    {
      off();
    }
    else
    {
      on();
    }
    return;
  }
}

void on()
{
  userLastActivity = now;
  LOG("SABER ON");
  playOnEffect();
  startBgSound();
  lightUp();
  isOn = 1;
}

void off()
{
  userLastActivity = now;
  playOffEffect();
  lightDown();
  LOG("SABER OFF");
  isOn = 0;
}

void strikeTick()
{
  if (ACC > STRIKE_THR)
  {
    userLastActivity = now;
    playStrikeEffect();
    strikeFlash();
    LOG2("STRIKE: ", ACC);
  }
}

void swingTick()
{
  if (GYR > SWING_THR)
  {
    userLastActivity = now;
    if (GYR > SWING_S_THR)
    {
      playSwingEffect();
    }
    else
    {
      playShortSwingEffect();
    }
    LOG2("SWING: ", GYR);
  }
}

void readAccel()
{
  if (micros() - mpuTimer > 500)
  {
    mpuTimer = micros();
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    ACC = (long)sqrt(sq((double)ax) + sq((double)ay) + sq((double)az)) / 100;
    GYR = (long)sqrt(sq((double)gx) + sq((double)gy) + sq((double)gz)) / 100;
    COMPL = ACC + GYR;
  }
}
