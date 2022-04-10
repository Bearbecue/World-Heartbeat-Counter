#pragma once

#include "Helpers.h"
#include "7SegDriver.h"
#include "BigNum.h"
#include "ControllerBPM.h"

//----------------------------------------------------------------------------

class HeartbeatController
{
public:
  HeartbeatController();

  enum
  {
    kMaxBPMSampleCount = 32,
  };

  void  Setup(const BPMController *bpmController);

  void  SetPopulation(uint64_t population);
  void  DirtyBPMCurve();

  // Returns 'true' if the date has changed
  bool  Update(int dtMS);

  // Print the current heartbeat counter to the 7-segment displays
  void  Print(LedControl &segDisp, int offset) const;

private:
  const BPMController *m_BPMCurve;

  SBigNum             m_Counter;
  
  int                 m_CurrentTime;
  int                 m_CurrentBPMSampleCount;
  int                 m_PeakBPM;

  uint64_t            m_Population;
  
  int                 m_TimeAcc[kMaxBPMSampleCount];
  int                 m_Countdown[kMaxBPMSampleCount];
  float               m_BPMKernel[kMaxBPMSampleCount];

  void                _UpdateLoCount(SBigNum &v, int dtMS);
  void                _UpdateHiCount(SBigNum &v, int dtMS);
  void                _RebuildBPMKernel();
};

//----------------------------------------------------------------------------