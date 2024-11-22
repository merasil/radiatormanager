#include "FanManager.h"

uint16_t FanManager::calcRPM(uint8_t id, typeof(millis()) t)
{
    uint16_t cnt;
    switch(id)
    {
    case 0:
        cnt = fan0_cnt;
        fan0_cnt = 0;
        break;    
    case 1:
        cnt = fan1_cnt;
        fan1_cnt = 0;
        break;    
    case 2:
        cnt = fan2_cnt;
        fan2_cnt = 0;
        break;   
    case 3:
        cnt = fan3_cnt;
        fan3_cnt = 0;
        break;
    }
    return ((60000 / t) * cnt) / 4;
}

void FanManager::attachFanInterrupt(uint8_t id) {
    switch (id) {
        case 0: attachInterrupt(m_fans[id].pin_rpm, intFan0, FALLING); break;
        case 1: attachInterrupt(m_fans[id].pin_rpm, intFan1, FALLING); break;
        case 2: attachInterrupt(m_fans[id].pin_rpm, intFan2, FALLING); break;
        case 3: attachInterrupt(m_fans[id].pin_rpm, intFan3, FALLING); break;
    }
}

void FanManager::detachFanInterrupt(uint8_t id) {
    detachInterrupt(m_fans[id].pin_rpm);
}

void FanManager::handle()
{
    if(millis() - m_fans_rpmtimer > 2000)
    {
        for(uint8_t i = 0; i < m_fans_count; ++i)
        {
            if(m_fans[i].meassureRPM)
                m_fans[i].curr_rpm = calcRPM(m_fans[i].id, millis()-m_fans_rpmtimer);
        }
        m_fans_rpmtimer = millis();
    }
}

bool FanManager::addFan(uint8_t pin_pwm, uint8_t pin_rpm, bool meassureRPM, bool reversed)
{
    if(m_fans_count+1 >= (sizeof(m_fans) / sizeof(Fan)))
        return false;
    m_fans[m_fans_count].id = m_fans_count;
    m_fans[m_fans_count].reversed = reversed;
    m_fans[m_fans_count].pin_pwm = pin_pwm;
    m_fans[m_fans_count].pin_rpm = pin_rpm;
    pinMode(pin_rpm, INPUT);
    if(meassureRPM)
    {
        m_fans[m_fans_count].meassureRPM = true;
        attachFanInterrupt(m_fans[m_fans_count].id);
    }
    m_fans_count += 1;
    return true;
}

bool FanManager::setRPM(uint8_t id, int16_t rpm)
{
    if(id > m_fans_count)
        return false;
    if(rpm == -1)
    {
        setPWM(id, 100);
        return true;
    }
    if(m_fans[id].max_rpm == 0)
    {
        setPWM(id, 100);
        m_fans[id].max_rpm = getRPM(id);
    }
    setPWM(id, (rpm/m_fans[id].max_rpm) * 100);
    return true;
}

bool FanManager::setPWM(uint8_t id, uint8_t pwm_perc)
{
    if(id > m_fans_count)
        return false;
    uint8_t pwm = 0;
    if(m_fans[id].reversed)
        pwm = 255 - ((pwm_perc * 255) / 100);
    else
        pwm = (pwm_perc * 255) / 100;
    m_fans[id].curr_pwm = pwm_perc;
    analogWrite(m_fans[id].pin_pwm, pwm);

    return true;
}

uint16_t FanManager::getRPM(uint8_t id)
{
    if(id > m_fans_count)
        return false;
    return m_fans[id].curr_rpm;
}

uint8_t FanManager::getPWM(uint8_t id)
{
    if(id > m_fans_count)
        return false;
    return m_fans[id].curr_pwm;
}

uint8_t FanManager::getFanCount()
{
    return m_fans_count;
}