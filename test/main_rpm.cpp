#include <Arduino.h>
#include <FanManager.h>

FanManager fm;
uint64_t t1;
uint64_t t2;
volatile uint16_t cnt;
volatile uint16_t cnt2;
uint16_t speed = 255;

void tick()
{
  ++cnt;
}

void tick2()
{
  ++cnt2;
}

void setup(void) {
  Serial.begin(115200);

  pinMode(0, OUTPUT);
  pinMode(5, INPUT_PULLUP);

  pinMode(1, OUTPUT);
  pinMode(6, INPUT_PULLUP);

  attachInterrupt(5, tick, FALLING);
  attachInterrupt(6, tick2, FALLING);
  t1 = millis();
}

void loop(void) {
  if(millis() - t1 > 1000)
  {
      detachInterrupt(5);
      detachInterrupt(6);

      Serial.print("Fan1 Cnt: "); Serial.println(cnt);
      Serial.print("Fan1 RPM: "); Serial.println(cnt * 30);

      Serial.print("Fan2 Cnt: "); Serial.println(cnt2);
      Serial.print("Fan2 RPM: "); Serial.println(cnt2 * 30);
      Serial.println();

      cnt = 0;
      cnt2 = 0;
      t1 = millis();
      attachInterrupt(5, tick, FALLING);
      attachInterrupt(6, tick2, FALLING);
  }
  if(millis() - t2 > 10000)
  {
    if(speed == 255)
      speed = 0;
    else if(speed == 0)
      speed = 128;
    else
      speed = 255;
    Serial.printf("Changing Speed to %d\n", speed);
    analogWrite(0, speed);
    analogWrite(1, speed);
    t2 = millis();
  }
}