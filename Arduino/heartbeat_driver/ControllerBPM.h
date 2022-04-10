#pragma once

#include "Helpers.h"
#include "7SegDriver.h"

//----------------------------------------------------------------------------

#define COMPUTE_BPM_FROM_SAMPLING_INDEX(__i, __maxSamples, __start, __end)  ((__start) + ((2 * int(__i) + 1) * ((__end) - (__start))) / (2 * int(__maxSamples)))

//----------------------------------------------------------------------------

class BPMController
{
public:
  BPMController();

  void  Setup();

  // Returns 'true' if the bpm curve has changed
  bool  Update();

  void  DrawCurve(LedControl &segDisp);

  // Returns the sum
  float Sample(float *dst, int sampleCount);
private:
};

//----------------------------------------------------------------------------
