//----------------------------------------------------------------------------
// MAX7219 libraries:
// - DigitLed72XX : Uses SPILibrary (hardware implem, faster, only works on specific pins)
// - MD_MAX72xx : Use SPILibrary (HW & SW implems)
// - LedControl : Use shiftOut / software SPI, slower. Better API
// - LedController : More "modern", handles both software & hardware SPI, a lot more code,
//   lots of C++ bullshit, very convoluted, suspiciously inefficient code patterns a bit everywhere.
//----------------------------------------------------------------------------

#include "HWPinConfig.h"
#include "Helpers.h"
#include "7SegDriver.h"
#include "PopulationTable.h"
#include "ControllerBPM.h"
#include "ControllerDate.h"
#include "BigNum.h"

//----------------------------------------------------------------------------
// NOTE !!! We should be able to use the same MOSI & CLK pins between all rails,
// and only have a dedicated 'CS' pin, that's the purpose of the CS pin.
// However I did not double-check this. Check it !

LedControl  displayRails[] =
{
  LedControl(PIN_OUT_DISP_MOSI, PIN_OUT_DISP_CLK, PIN_OUT_RAIL0_CS, 8), // MOSI, CLK, CS, # of 7219s daisy-chained together
  LedControl(PIN_OUT_DISP_MOSI, PIN_OUT_DISP_CLK, PIN_OUT_RAIL1_CS, 8),
};
const int   kDispRailCount = sizeof(displayRails) / sizeof(displayRails[0]); // number of HW rails, up to 8 displays of 8 digits each

//----------------------------------------------------------------------------
// Address of main counter display
const int   kDispRail_Main = 0;
const int   kDispOffset_Main = 0;
const int   kChipCount_Main = 3;

// Address of population display
const int   kDispRail_Population = 0;
const int   kDispOffset_Population = kDispOffset_Main + kChipCount_Main;
const int   kChipCount_Population = 2;

// Address of date display
const int   kDispRail_Date = 0;
const int   kDispOffset_Date = kDispOffset_Population + kChipCount_Population;
const int   kChipCount_Date = 1;

// Address of curve display
const int   kDispRail_Curve = 1;
const int   kDispOffset_Curve = 0;
const int   kChipCount_Curve = 8;

//----------------------------------------------------------------------------

unsigned long   prevFrameMs = 0;
unsigned long   cur_time = 0;

DateController  currentDate;

uint64_t        popcount = 0;

SBigNum         hbCount;

//----------------------------------------------------------------------------

void setup()
{
#if 1
//  pinMode(A0, INPUT);
  pinMode(DATE_ROTENC_PIN_A, INPUT_PULLUP);
  pinMode(DATE_ROTENC_PIN_B, INPUT);
#endif

  for (int i = 0; i < 4; i++)
    pinMode(PIN_IN_BPM(i), INPUT);

  // Clear displays
  for (int r = 0; r < kDispRailCount; r++)
  {
    LedControl  &segDisp = displayRails[r];
    for (int i = 0; i < segDisp.getDeviceCount(); i++)
    {
      segDisp.clearDisplay(i);
      segDisp.setIntensity(i, 0);
      segDisp.shutdown(i, false);
    }
  }

  SetupBPMControls();
  currentDate.Setup();
  SetupPopulationControls();

  Serial.begin(9600);

  prevFrameMs = millis();

  setIntensity_Counter(1.0f);
  setIntensity_Population(1.0f);
  setIntensity_Date(1.0f);
  setIntensity_Curve(0.3f);

  ResetBPMControls();

  popcount = GetPopulationAtDate(currentDate.Year());

  _PopulationDisplay(popcount);
  _DateDisplay(currentDate.Year());

  _DrawBPMCurve(displayRails[kDispRail_Curve]);
}

//----------------------------------------------------------------------------

void  _SetIntensity_Impl(float intensity, int railId, int chipOffset, int chipCount)
{
  const int hwIntensity = clamp(int(intensity * 15), 0, 15);

  LedControl  &segDisp = displayRails[railId];
  for (int i = chipOffset; i < chipCount; i++)
    segDisp.setIntensity(i, hwIntensity);

  for (int i = chipOffset; i < chipCount; i++)
    segDisp.shutdown(i, false);
}

//----------------------------------------------------------------------------

void  setIntensity_Counter(float intensity) { _SetIntensity_Impl(intensity, kDispRail_Main, kDispOffset_Main, kChipCount_Main); }
void  setIntensity_Population(float intensity) { _SetIntensity_Impl(intensity, kDispRail_Population, kDispOffset_Population, kChipCount_Population); }
void  setIntensity_Date(float intensity) { _SetIntensity_Impl(intensity, kDispRail_Date, kDispOffset_Date, kChipCount_Date); }
void  setIntensity_Curve(float intensity) { _SetIntensity_Impl(intensity, kDispRail_Curve, kDispOffset_Curve, kChipCount_Curve); }

//----------------------------------------------------------------------------

static float  _integratePulse01(float t0, float t1, float k)
{
  float x = 2*t0 - 1;
  float y = 2*t1 - 1;
  float num = (y*y - 4*t1 - x*x + 4*t0) * k;
  return abs(num / 4 + t1 - t0);
}

//----------------------------------------------------------------------------

static float  _integratePulse(float t0, float t1, float k)
{
  float sum = 0;
  if (t0 < 0.0f)
  {
    sum += _integratePulse01(1.0f - t0, 1.0f, k);
    sum += _integratePulse01(0.0f, t1, k);
  }
  else
  {
    sum += _integratePulse01(t0, t1, k);
  }
  return sum;
}

//----------------------------------------------------------------------------

void loop()
{
  ReadBPMControls();

  if (currentDate.Update())
  {
    // Date has changed
    // Update population count at date
    popcount = GetPopulationAtDate(currentDate.Year());

    _PopulationDisplay(popcount);
    _DateDisplay(currentDate.Year());
  }

#if 1
  const int dtMs = SyncFrameTick(10);
  cur_time += dtMs;

  SBigNum newHBCount = hbCount;

  if (popcount <= 2)
  {
    int nextBeatDelay = (uint32_t(1000) * 60) / 68; // in ms
    if (cur_time >= nextBeatDelay)
    {
      cur_time -= nextBeatDelay;
      newHBCount += 1;
    }
  }
  else
  {
    int nextBeatDelay = (uint32_t(1000) * 60) / 68; // in ms
  
    // when sync == 1, everyone beats at the same tick    (discrete spike curve, slope = +inf)
    // when sync == 0, everyone beats at a different tick (flat curve, slope = 0)
    float sync = 1;//clamp(input / 1000.0f, 0.0f, 1.0f);//1.0f; // should be 1023, but allow for pot imprecisions & voltage losses
    bool  synchronized = true;//sync > 0.99f;
    float v = 0.0f;
    float total_integral = 1.0f;
    if (!synchronized)
    {
      static float  prevT = 0.0f;
      float t = cur_time / float(nextBeatDelay);
      if (cur_time >= nextBeatDelay)
      {
        while (cur_time >= nextBeatDelay)
        {
          cur_time -= nextBeatDelay;
          t -= 1.0f;
        }
        prevT -= 1.0f;
      }
  
      float slope = 1.0f / (1 - sync);
      //v = slope * abs(t - 0.5f) * 2 - (slope - 1);
      total_integral = -(slope - 2) / 2;
      v = _integratePulse(prevT, t, slope) / total_integral;
  
      Serial.print("[");
      Serial.print(prevT);
      Serial.print(", ");
      Serial.print(t);
      Serial.print("]: ");
      Serial.println(v);
  
      prevT = t;
    }
    else
    {
#if 0
      const int   countDownWidth = 30 + 1;
      const float k = 10;
      const float kNorm = 0.42386f;
#else
      const int   countDownWidth = 50 + 1;
      const float k = 100;
      const float kNorm = 0.9925096f;
#endif
      static int  countDown = 0;
  
      if (cur_time >= nextBeatDelay)
      {
        cur_time -= nextBeatDelay;
        total_integral = 1.0f;
        v = 0.0f;
        countDown = countDownWidth;
      }
  
      if (countDown > 0)
      {
        countDown--;
        const float cursor = 1.0f - 2*(countDown / float(countDownWidth-1));  // [-1, 1]
  
        const float x = k * cursor;
        const float den = 1 + x * x;
        v = 1.0f / (den * den);

        v *= kNorm;
      }
    }

    if (v != 0)
    {
      newHBCount += v * popcount;
  
      // 2892479536812345671
      // 10000000000000000000
      // 11561240708638945646987
    }
  }

  if (newHBCount != hbCount)
  {
    hbCount = newHBCount;
    hbCount.PrintTo7Seg(displayRails[kDispRail_Main], kDispOffset_Main);
  }
#endif

  {
    LedControl  &segDisp = displayRails[kDispRail_Main];
    segDisp.flushDeviceState();
  }
}

//----------------------------------------------------------------------------

int SyncFrameTick(int targetMs)
{
  const int       kDefaultUpdateMsOnInternalOverflow = 10; // When u32 overflows (after ~50 days), default to 10ms step
  unsigned long   curMs = 0;
  int             updateMs = 0;
  do
  {
    curMs = millis();
    updateMs = ((curMs >= prevFrameMs) ? (curMs - prevFrameMs) : kDefaultUpdateMsOnInternalOverflow);
    if (updateMs < targetMs)
      delay(targetMs - updateMs);
  } while (updateMs < targetMs);

  prevFrameMs = curMs;
  
  return updateMs;
}

//----------------------------------------------------------------------------

void  _PopulationDisplay(int64_t value)
{
  int dispOffset = kDispOffset_Population;
  int kMaxDigits = 12;
  LedControl  &segDisp = displayRails[kDispRail_Population];

  int digitID = 0;
  while (value != 0 && digitID < kMaxDigits)
  {
    byte  digit = value % 10;
    value /= 10;
    int   did = digitID++;
    segDisp.setDigit(dispOffset + (did / 8), did % 8, digit, false);
  }

  if (digitID == 0)
    segDisp.setRawDigit(dispOffset * 8 + digitID++, 0, false);

  for (int i = digitID; i < kMaxDigits; i++)
    segDisp.setRawChar((dispOffset + (i / 8)) * 8 + (i % 8), ' ', false);
}

//----------------------------------------------------------------------------

void  _DateDisplay(int32_t value)
{
  int dispOffset = kDispOffset_Date;
  int kMaxDigits = 7;
  LedControl  &segDisp = displayRails[kDispRail_Date];

  bool  negative = value < 0;
  if (negative)
    value = -value;

  int digitID = 0;
  while (value != 0 && digitID < kMaxDigits - 1)
  {
    byte  digit = value % 10;
    value /= 10;
    segDisp.setDigit(dispOffset, digitID++, digit, false);
  }

  if (negative)
    segDisp.setRawChar(dispOffset * 8 + digitID++, '-', false);
  else if (digitID == 0)
    segDisp.setRawDigit(dispOffset * 8 + digitID++, 0, false);

  for (int i = digitID; i < kMaxDigits; i++)
    segDisp.setRawChar(dispOffset * 8 + i, ' ', false);
}

//----------------------------------------------------------------------------
