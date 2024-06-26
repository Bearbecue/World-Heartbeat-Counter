#pragma once

#include "Helpers.h"
#include "7SegDriver.h"

//----------------------------------------------------------------------------

class PopulationController
{
public:
  PopulationController();

  void  Setup();

  void  SetYear(int32_t year);

  // Population accessors
  int64_t Population() const { return m_Population; }

  // Returns 'true' if the population count has changed
  bool  Update(int dtMS);

  // Print the current population counter to the 7-segment displays
  void  Print(LedControl &segDisp, int offset);

private:
  int64_t             m_Population;
  int64_t             m_PopulationNext;

  int64_t             m_NextBirthDelay;
  int64_t             m_NextDeathDelay;
  
  int64_t             m_NextBirthCounter;
  int64_t             m_NextDeathCounter;

  uint32_t            m_BirthOffset;
  uint32_t            m_DeathOffset;

  int                 m_BirthLEDIntensity;
  int                 m_DeathLEDIntensity;

  float               _GetPopulationAtDate(int32_t year);
};

//----------------------------------------------------------------------------
