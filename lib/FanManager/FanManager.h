#pragma once
#include <Arduino.h>

static volatile uint16_t fan0_cnt = 0;
static volatile uint16_t fan1_cnt = 0;
static volatile uint16_t fan2_cnt = 0;
static volatile uint16_t fan3_cnt = 0;
static void intFan0()
{
    fan0_cnt += 1;
}
static void intFan1()
{
    fan1_cnt += 1;
}
static void intFan2()
{
    fan2_cnt += 1;
}
static void intFan3()
{
    fan3_cnt += 1;
}

class FanManager
{
private:
    struct Fan
    {
        uint8_t id;
        bool meassureRPM;
        bool reversed = false;
        uint8_t pin_pwm;
        uint8_t pin_rpm;
        uint8_t curr_pwm;
        uint16_t curr_rpm;
        uint16_t max_rpm;
    };
    Fan m_fans[10];
    uint8_t m_fans_count = 0;
    typeof(millis()) m_fans_rpmtimer = 0;

    uint16_t calcRPM(uint8_t id, typeof(millis()) t);
    void attachFanInterrupt(uint8_t id);
    void detachFanInterrupt(uint8_t id);

public:
    FanManager(){}
    void handle();
    bool addFan(uint8_t pin_pwm, uint8_t pin_rpm, bool meassureRPM = true, bool reversed = false);
    bool setRPM(uint8_t id, int16_t rpm = -1);
    bool setPWM(uint8_t id, uint8_t pwm_perc = 100);
    uint16_t getRPM(uint8_t id);
    uint8_t getPWM(uint8_t id);
    uint8_t getFanCount();
};