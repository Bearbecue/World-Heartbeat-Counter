//----------------------------------------------------------------------------

#include "ControllerDate.h"
#include "HWPinConfig.h"

int32_t         kDateMin = -100000;
int32_t         kDateMax = 2100;

int32_t         rotEncCounterRaw = 0;

int             prevCLK = 0;
int             prevCLK2 = 0;

//----------------------------------------------------------------------------

void  updateRotaryEncoder()
{
  const int currentCLK = digitalRead(DATE_ROTENC_PIN_A);
  const int currentDT = digitalRead(DATE_ROTENC_PIN_B);
  if (currentDT != currentCLK)
    rotEncCounterRaw--; // CCW: Decrement date
  else
    rotEncCounterRaw++; // CW: Increment date
}

//----------------------------------------------------------------------------

DateController::DateController()
: m_CurrentYear(0)
//: m_CurrentYear(-99953)
//: m_CurrentYear(-100000)
{
}

//----------------------------------------------------------------------------

void  DateController::Setup(int32_t startDate)
{
  m_CurrentYear = startDate;

  // Setup rotary encoder
  pinMode(DATE_ROTENC_PIN_A, INPUT_PULLUP);
  pinMode(DATE_ROTENC_PIN_B, INPUT_PULLUP);

  prevCLK = digitalRead(DATE_ROTENC_PIN_A);
  attachInterrupt(digitalPinToInterrupt(DATE_ROTENC_PIN_A), updateRotaryEncoder, RISING);
}

//----------------------------------------------------------------------------

bool DateController::Update()
{
  bool  changed = false;

  static int32_t  prevRotEncCounterRaw = 0;
  if (rotEncCounterRaw != prevRotEncCounterRaw)
  {
    int32_t  delta = rotEncCounterRaw - prevRotEncCounterRaw;
    prevRotEncCounterRaw = rotEncCounterRaw;
  
    // Note: Here in theory 'delta' should contain the actual offset since last tick.
    // If user turns the rotary encoder real fast, because we have an interrupt setup it should
    // not miss any step, but for some reason, it doesn't work reliably, and even when turning
    // the wheel slowly, we sometimes get large jumps of > 5 or 10 ticks at once.
    // Maybe due to a lower build quality of the rotary enc? maybe the signal needs to be filtered?
    // Should inspect this on the scope and see if there are any parasitics. Maybe plug in a stepper circuit
    // to have clean rising/falling edges if there is noise? or simply filtering it with a small cap would be enough ?
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
#if 0
          Serial.print("Fast time-delta: ");
          Serial.println(timeDelta);
#endif
//          if (m_CurrentYear < -1000)
//            offset *= 10;
//          else
          offset *= 5;
        }/*
        else
        {
          Serial.print("Med time-delta: ");
          Serial.println(timeDelta);
        }*/
      }
//      else if (timeDelta < 100)
//        offset *= 10;
   }
    prevMs = curMs;
  
    // Compute new date
    int32_t newCurYear = clamp(m_CurrentYear + offset, kDateMin, kDateMax);
    if (m_CurrentYear != newCurYear)
    {
      m_CurrentYear = newCurYear;
      changed = true;
    }
  }

  return changed;
}

//----------------------------------------------------------------------------

void  _DateDisplay(LedControl &segDisp, int dispOffset, int32_t value)
{
  int kMaxDigits = 7;

  bool  negative = value < 0;
  if (negative)
    value = -value;

  int digitID = 0;
  while (value != 0 && digitID < kMaxDigits - 1)
  {
    byte  digit = value % 10;
    value /= 10;
    segDisp.setRawDigit(dispOffset * 8 + digitID++, digit, false);
  }

  if (negative)
    segDisp.setRawChar(dispOffset * 8 + digitID++, '-', false);
  else if (digitID == 0)
    segDisp.setRawDigit(dispOffset * 8 + digitID++, 0, false);

  for (int i = digitID; i < kMaxDigits; i++)
    segDisp.setRawChar(dispOffset * 8 + i, ' ', false);
}

//----------------------------------------------------------------------------

void  DateController::Print(LedControl &segDisp, int offset) const
{
  _DateDisplay(segDisp, offset, m_CurrentYear);
}
