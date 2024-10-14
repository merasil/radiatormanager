#include <Arduino.h>
#include <FanManager.h>

FanManager fm;
uint64_t t1;
uint64_t t2;
uint16_t speed = 100;

void setup(void) {
  Serial.begin(115200);

  fm.addFan(0, 5, true, true);
  fm.addFan(1, 6, true, true);
  fm.addFan(3, 7, true, true);
  fm.addFan(4, 10, true, true);
  t1 = millis();
}

void loop(void) {
  fm.handle();
  if(millis() - t1 > 1000)
  {
      Serial.print("Fan1: "); Serial.println(fm.getPWM(0));
      Serial.print("Fan1 RPM: "); Serial.println(fm.getRPM(0));

      Serial.print("Fan2: "); Serial.println(fm.getPWM(1));
      Serial.print("Fan2 RPM: "); Serial.println(fm.getRPM(1));

      Serial.print("Fan3: "); Serial.println(fm.getPWM(2));
      Serial.print("Fan3 RPM: "); Serial.println(fm.getRPM(2));

      Serial.print("Fan4: "); Serial.println(fm.getPWM(3));
      Serial.print("Fan4 RPM: "); Serial.println(fm.getRPM(3));
      Serial.println();
      t1 = millis();
  }

  if(millis() - t2 > 20000)
  {
    if(speed == 100)
      speed = 0;
    else if(speed == 0)
      speed = 50;
    else
      speed = 100;
    Serial.printf("Changing Speed to %d\n", speed);
    fm.setPWM(0, speed); 
    fm.setPWM(1, speed);
    fm.setPWM(2, speed);
    fm.setPWM(3, speed);

    t2 = millis();
  }
}