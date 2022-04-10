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

unsigned long         prevFrameMs = 0;

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

  Serial.begin(115200);

  prevFrameMs = millis();

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

void loop()
{
  // Update BPM curve
  if (bpm.Update())
  {
    heartbeats.DirtyBPMCurve();
    bpm.DrawCurve(displayRails[kDispRail_Curve]);
  }

  // Update current date
  if (currentDate.Update())
  {
    // Date has changed, update population count at current date
    population.SetYear(currentDate.Year());
    heartbeats.SetPopulation(population.Population());
  
    population.Print(displayRails[kDispRail_Population], kDispOffset_Population);
    currentDate.Print(displayRails[kDispRail_Date], kDispOffset_Date);
  }

  const int dtMS = SyncFrameTick(10);
  if (heartbeats.Update(dtMS))
  {
    heartbeats.Print(displayRails[kDispRail_Main], kDispOffset_Main);
  }

  // Flush main display
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
