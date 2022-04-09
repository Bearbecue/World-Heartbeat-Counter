//----------------------------------------------------------------------------

#include "HWPinConfig.h"
#include "ControllerDate.h"

int32_t         rotEncCounterRaw = 0;
int32_t         kDateMin = -100000;
int32_t         kDateMax = 2100;

int             prevCLK = 0;

//----------------------------------------------------------------------------

void  updateRotaryEncoder()
{
  int currentCLK = digitalRead(DATE_ROTENC_PIN_A);
  int currentDT = digitalRead(DATE_ROTENC_PIN_B);
  if (currentDT != currentCLK)
    rotEncCounterRaw++;
  else
    rotEncCounterRaw--;
}

//----------------------------------------------------------------------------

DateController::DateController()
: m_CurrentYear(2022)
{
}

//----------------------------------------------------------------------------

void  DateController::Setup()
{
  // Setup rotary encoder
  prevCLK = digitalRead(DATE_ROTENC_PIN_A);
  attachInterrupt(digitalPinToInterrupt(DATE_ROTENC_PIN_A), updateRotaryEncoder, RISING);
}

//----------------------------------------------------------------------------

bool DateController::Update()
{
  static int32_t  prevRotEncCounterRaw = 0;
  if (rotEncCounterRaw == prevRotEncCounterRaw)
    return false;

  int32_t  delta = rotEncCounterRaw - prevRotEncCounterRaw;
  prevRotEncCounterRaw = rotEncCounterRaw;

  int32_t  offset = (delta < 0) ? -1 : 1;

  static unsigned long  prevMs = 0;
  unsigned long         curMs = millis();
  if (curMs >= prevMs)
  {
    int32_t timeDelta = curMs - prevMs;
    if (timeDelta < 50)
    {
      if (m_CurrentYear > 1900 || m_CurrentYear < -99900)
        offset *= 2;
      else if (m_CurrentYear > -1000 || m_CurrentYear < -99000)
        offset *= 20;
      else
        offset *= 200;

      if (timeDelta < 20)
      {
        Serial.print("Fast time-delta: ");
        Serial.println(timeDelta);
//        if (m_CurrentYear < -1000)
//          offset *= 10;
//        else
        offset *= 5;
      }/*
      else
      {
        Serial.print("Med time-delta: ");
        Serial.println(timeDelta);
      }*/
    }
//    else if (timeDelta < 100)
//      offset *= 10;
  }
  prevMs = curMs;

  // Compute new date
  m_CurrentYear = clamp(m_CurrentYear + offset, kDateMin, kDateMax);
  return true;
}

//----------------------------------------------------------------------------
