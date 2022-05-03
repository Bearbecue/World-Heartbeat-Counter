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
#include "ControllerBPM.h"
#include "ControllerDate.h"
#include "ControllerPopulation.h"
#include "ControllerHeartbeat.h"

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

DateController        currentDate;
BPMController         bpm;
PopulationController  population;
HeartbeatController   heartbeats;

//----------------------------------------------------------------------------

void setup()
{
  // Clear displays
  for (int r = 0; r < kDispRailCount; r++)
  {
    LedControl  &segDisp = displayRails[r];
    for (int i = 0; i < segDisp.getDeviceCount(); i++)
    {
      segDisp.clearDisplay(i);
      segDisp.shutdown(i, false);
      segDisp.setIntensity(i, 0);
    }
  }

//  displayRails[kDispRail_Date].setScanLimit(kDispOffset_Date, 4);

  Serial.begin(115200);

  setIntensity_Counter(1.0f);
  setIntensity_Population(1.0f);
  setIntensity_Date(1.0f);
  setIntensity_Curve(0.3f);

  bpm.Setup();
  heartbeats.Setup(&bpm);
  currentDate.Setup();
  population.Setup();
  
  population.SetYear(currentDate.Year());
  heartbeats.SetPopulation(population.Population());

  population.Print(displayRails[kDispRail_Population], kDispOffset_Population);
  currentDate.Print(displayRails[kDispRail_Date], kDispOffset_Date);

  bpm.DrawCurve(displayRails[kDispRail_Curve]);
}

//----------------------------------------------------------------------------

static int  _GetLoopDt()
{
  static long int prevMS = millis() - 10;
  const long int  curMS = millis();
  const int       dtMS = (curMS > prevMS) ? curMS - prevMS : 10;  // by default 10 ms when wrapping around
  prevMS = curMS;
  return dtMS;
}

//----------------------------------------------------------------------------

void loop()
{
  const int dtMS = _GetLoopDt();

  // Update BPM curve
#if 1
  if (bpm.Update()) // ~0.74ms
  {
    heartbeats.DirtyBPMCurve(); // ~8.43ms
    bpm.DrawCurve(displayRails[kDispRail_Curve]); // ~8.60ms
  }
#endif

  // Update current date
#if 1
  if (currentDate.Update()) // ~0.01ms
  {
    // Date has changed, update population count at current date
    population.SetYear(currentDate.Year()); // ~0.42ms
    heartbeats.SetPopulation(population.Population());  // 0ms
  
    currentDate.Print(displayRails[kDispRail_Date], kDispOffset_Date);  // ~0.20ms
    population.Print(displayRails[kDispRail_Population], kDispOffset_Population); // ~0.48ms
  }
#endif

#if 1
  if (population.Update(dtMS))  // ~0.05ms
  {
    heartbeats.SetPopulation(population.Population());  // 0ms
    population.Print(displayRails[kDispRail_Population], kDispOffset_Population); // ~1.17ms
  }
#endif

#if 1
  if (heartbeats.Update(dtMS))  // ~0.23ms
  {
    heartbeats.Print(displayRails[kDispRail_Main], kDispOffset_Main); // ~1.2ms for full display
  }
#endif

  // Flush main display
#if 1
  {
    LedControl  &segDisp = displayRails[kDispRail_Main];
    segDisp.flushDeviceState(); // ~15ms when everything is updating
  }
#endif

  {
    static long int prevMS = millis();
    long int        curMS = millis();
    static int32_t  count = 0;
    static int32_t  acc = 0;
    acc += dtMS;
    count += 1;
    if (curMS - prevMS > 1000)
    {
      Serial.print("Avg tick time: ");
      Serial.print(acc / float(count));
      Serial.print(" - ");
      Serial.print((curMS - prevMS) / float(count));
      Serial.println(" ms");
      count = 0;
      acc = 0;
      prevMS = curMS;
    }
  }

  SyncFrameTick(10);
}

//----------------------------------------------------------------------------

int SyncFrameTick(int targetMs)
{
  const int             kDefaultUpdateMsOnInternalOverflow = 10; // When u32 overflows (after ~50 days), default to 10ms step
  static unsigned long  prevFrameMs = millis() - kDefaultUpdateMsOnInternalOverflow;  // use default dt on initial call
  unsigned long         curMs = 0;
  int                   updateMs = 0;
  do
  {
    curMs = millis();
    updateMs = ((curMs >= prevFrameMs) ? (curMs - prevFrameMs) : kDefaultUpdateMsOnInternalOverflow);
    // delay() for 1ms less than what we should theoretically wait for.
    // Improves waiting precision because delay() overshoots of 200us on average.
    // Doing this brings is down one magnitude level to ~35us on average
    // Not that we care, but cleaner. Will cause a few loop iterations to busy-run only calling 'millis()'
    // without any delay until the desired wait target is reached
    if (targetMs - 1 > updateMs)
      delay(targetMs - 1 - updateMs);
  } while (updateMs < targetMs);

  prevFrameMs = curMs;
  return updateMs;
}

//----------------------------------------------------------------------------

void  setIntensity_Counter(float intensity) { _SetIntensity_Impl(intensity, kDispRail_Main, kDispOffset_Main, kChipCount_Main); }
void  setIntensity_Population(float intensity) { _SetIntensity_Impl(intensity, kDispRail_Population, kDispOffset_Population, kChipCount_Population); }
void  setIntensity_Date(float intensity) { _SetIntensity_Impl(intensity, kDispRail_Date, kDispOffset_Date, kChipCount_Date); }
void  setIntensity_Curve(float intensity) { _SetIntensity_Impl(intensity, kDispRail_Curve, kDispOffset_Curve, kChipCount_Curve); }

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
