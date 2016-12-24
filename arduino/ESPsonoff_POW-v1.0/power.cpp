/*
  This libary is an enhanced version of the ITEAD.cc library

  It reads the values from the Sonoff POW HLW8012 chip and calculates the respective
  Power, current, and voltage values. Only voltage or current reading can be chosen.

  Copyright (c) [2016] [Iteand.cc and Andreas Spiess]

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "power.h"
#include <Arduino.h>

uint32_t ESP8266PowerClass::power_freq_cnt = 0;
uint32_t ESP8266PowerClass::current_freq_cnt = 0;
uint32_t ESP8266PowerClass::voltage_freq_cnt = 0;
uint8_t ESP8266PowerClass::power_test_mode = IOTGO_HLW8012_TEST_MODE_REPEAT;
uint8_t ESP8266PowerClass::current_test_mode = IOTGO_HLW8012_TEST_MODE_REPEAT;
uint8_t ESP8266PowerClass::voltage_test_mode = IOTGO_HLW8012_TEST_MODE_REPEAT;
bool ESP8266PowerClass::power_flag = false;
bool ESP8266PowerClass::current_flag = false;
bool ESP8266PowerClass::voltage_flag = false;
uint32_t ESP8266PowerClass::power_freq= 0;
uint32_t ESP8266PowerClass::voltage_freq= 0;
uint32_t ESP8266PowerClass::current_freq= 0;

void ESP8266PowerClass::measurePowerFreq(void)
{
    power_freq_cnt++;
}
void ESP8266PowerClass::measureCurrenFreq(void)   // counts number of interrupts CF1 pin
{
    current_freq_cnt++;
    if(voltage_freq_cnt != 0)
    {
        voltage_freq_cnt = 0;
    }
}
void ESP8266PowerClass::measureVoltageFreq(void)    // counts number of interrupts CF1 pin
{
    voltage_freq_cnt++;
    if(current_freq_cnt != 0)
    {
        current_freq_cnt = 0;
    }
}
void ESP8266PowerClass::getAverageFreq(uint32_t (&history_queue)[10],uint32_t &freq)
{
    uint8_t i = 0;
    for (i = 9; i >= 1; i--)
    {
        history_queue[i] = history_queue[i - 1];
    }
    history_queue[0] = freq;
    for (i = 1; i < 10; i++)
    {
        freq += history_queue[i];
    }
    freq = (uint32_t)((freq*1.0)/i + 0.5); 
}

void ESP8266PowerClass::getFreq(uint32_t &cnt,uint32_t &freq_cnt,uint8_t &test_mode,uint32_t &freq,
                 uint8_t &hlw8012_bad_once_cnt,uint8_t &hlw8012_bad_repeat_cnt,uint32_t (&history_queue)[10])
{
    
    if (IOTGO_HLW8012_TEST_MODE_REPEAT == test_mode)
    {
        if (cnt >= 1000)
        {
            if (freq_cnt > 10)
            {
                hlw8012_bad_repeat_cnt = 0;
                freq = freq_cnt * 100;
                getAverageFreq(history_queue,freq);
            }
            else
            {
                hlw8012_bad_repeat_cnt++;
                if (hlw8012_bad_repeat_cnt >= 2)
                {
                    hlw8012_bad_repeat_cnt = 0;
                    test_mode = IOTGO_HLW8012_TEST_MODE_ONCE;
                }
            }       
            freq_cnt = 0;
            cnt = 0;
            
        }
    }
    else if (IOTGO_HLW8012_TEST_MODE_ONCE == test_mode)
    {
        if (1 == freq_cnt)
        {
            if (cnt >= 100)
            {
                hlw8012_bad_once_cnt = 0;
                freq = (uint32_t)(100000.0/(cnt) + 0.5);
                getAverageFreq(history_queue,freq);
            }
            else
            {
                hlw8012_bad_once_cnt++;
                if (hlw8012_bad_once_cnt >= 2)
                {
                    hlw8012_bad_once_cnt = 0;
                    test_mode = IOTGO_HLW8012_TEST_MODE_REPEAT;
                }
            }
            cnt = 0; 
            freq_cnt = 0;
            
        }
        
        if (cnt >= (10*1000 + 1000))
        {
            test_mode = IOTGO_HLW8012_TEST_MODE_REPEAT;
            cnt = 0;
            freq_cnt = 0;
            freq = 0;
            os_memset(history_queue,0,10);
            
        }
    }
}

void ESP8266PowerClass::timerCallback(void)
{
    static uint32_t power_1ms_cnt = 0;
    static uint32_t curent_1ms_cnt = 0;
    static uint32_t voltage_1ms_cnt = 0;
    static uint8_t power_bad_once_cnt = 0;
    static uint8_t power_bad_repeat_cnt = 0;
    static uint8_t current_bad_once_cnt = 0;
    static uint8_t current_bad_repeat_cnt = 0;
    static uint8_t voltage_bad_once_cnt = 0;
    static uint8_t voltage_bad_repeat_cnt = 0;
    static uint32_t power_history_queue[10] = {0};
    static uint32_t current_history_queue[10] = {0};
    static uint32_t voltage_history_queue[10] = {0};
    power_1ms_cnt++;
    curent_1ms_cnt++;
    voltage_1ms_cnt++;
    if(power_flag)
    {
        getFreq(power_1ms_cnt,power_freq_cnt,power_test_mode,power_freq,
                                        power_bad_once_cnt,power_bad_repeat_cnt,power_history_queue);  // fills all these variables with numbers, either continuous or once
    }
    if(current_flag)
    {
        getFreq(curent_1ms_cnt,current_freq_cnt,current_test_mode,current_freq,
                                        current_bad_once_cnt,current_bad_repeat_cnt,current_history_queue);
    }
    if(voltage_flag)
    {
        getFreq(voltage_1ms_cnt,voltage_freq_cnt,voltage_test_mode,voltage_freq,
                                        voltage_bad_once_cnt,voltage_bad_repeat_cnt,voltage_history_queue);
                                        
    } 
}

ESP8266PowerClass::ESP8266PowerClass()  // Constructor
{
    this->power_param = {12.286,0};
    this->current_param = {14.818,0};
    this->voltage_param = {0.435,0};
    
    this->power_pin = 14;   // CF
    this->current_voltage_pin = 13;  //CF1
    this->sel_pin = 5;   // Chip select
}
ESP8266PowerClass::ESP8266PowerClass(int8_t power_pin,int8_t current_voltage_pin,int8_t sel_pin)
{
    ESP8266PowerClass();
    this->power_pin = power_pin;
    this->current_voltage_pin = current_voltage_pin;
    this->sel_pin = sel_pin;
}

void ESP8266PowerClass::enableMeasurePower(void)
{
    this->power_flag = true;
    pinMode(this->power_pin,INPUT_PULLUP);
    attachInterrupt(this->power_pin,measurePowerFreq,FALLING);   // interrupt on CF
}
void ESP8266PowerClass::selectMeasureCurrentOrVoltage(MEASURETYPE dev_type)
{

#if 1
    pinMode(this->sel_pin,OUTPUT);  // CF1
    detachInterrupt(this->current_voltage_pin);
    voltage_freq = 0;
    current_freq = 0;
    pinMode(this->current_voltage_pin,INPUT_PULLUP);
    switch(dev_type)
    {
        case CURRENT:
        {
            this->current_flag = true;
            this->voltage_flag = false;
            digitalWrite(this->sel_pin,HIGH);
            attachInterrupt(this->current_voltage_pin,measureCurrenFreq,FALLING);
        }break;
        case VOLTAGE:
        {   
            this->current_flag = false;
            this->voltage_flag = true;
            digitalWrite(this->sel_pin,LOW);
            attachInterrupt(this->current_voltage_pin,measureVoltageFreq,FALLING);
        }break;
        default:
        {
            /*
                do nothing
            */
        }
    }
#endif
}
void ESP8266PowerClass::setPowerParam(double param_a,double param_b)
{
    this->power_param.param_a = param_a;
    this->power_param.param_b = param_b;
}

void ESP8266PowerClass::setCurrentParam(double param_a,double param_b)
{
    this->current_param.param_a = param_a;
    this->current_param.param_b = param_b;
}

void ESP8266PowerClass::setVoltageParam(double param_a,double param_b)
{
    this->voltage_param.param_a = param_a;
    this->voltage_param.param_b = param_b;
}

DEVPARAM ESP8266PowerClass::getPowerParam(void)
{
    return power_param;
}
DEVPARAM ESP8266PowerClass::getCurrentParam(void)
{
    return current_param;
}

DEVPARAM ESP8266PowerClass::getvoltageParam(void)
{
    return voltage_param;
}
void ESP8266PowerClass::startMeasure(void)
{
    os_timer_setfn(&myTimer, (os_timer_func_t *)&ESP8266PowerClass::timerCallback, NULL);
    os_timer_arm(&myTimer, 1, true);  // every ms
}

double ESP8266PowerClass::getPower(void)
{
    uint32_t power = 0;
    power = power_param.param_a * power_freq + power_param.param_b * 100;
    return power / 100.0;
}
double ESP8266PowerClass::getCurrent(void)
{
    double current = (current_param.param_a * current_freq + current_param.param_b*100.0);
    
    return current/100.0;
}

double ESP8266PowerClass::getCurrFrequency(void)
{
    uint32_t frequency = 0;
    frequency = current_freq ;
    return frequency;
}


double ESP8266PowerClass::getVoltage(void)
{
    uint32_t voltage = 0;
    voltage = voltage_param.param_a * voltage_freq + voltage_param.param_b * 100;
    return voltage / 100.0;
}

