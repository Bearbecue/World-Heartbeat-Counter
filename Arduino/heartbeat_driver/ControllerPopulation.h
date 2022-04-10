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

  // Date accessors
  int64_t Population() const { return m_Population; }

  // Print the current population counter to the 7-segment displays
  void  Print(LedControl &segDisp, int offset);

private:
  int64_t             m_Population;
};

//----------------------------------------------------------------------------
