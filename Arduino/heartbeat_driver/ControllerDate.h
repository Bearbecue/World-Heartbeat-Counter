#pragma once

#include "Helpers.h"
#include "7SegDriver.h"

//----------------------------------------------------------------------------

class DateController
{
public:
  DateController();

  void  Setup();

  // Returns 'true' if the date has changed
  bool  Update();

  // Date accessors
  int32_t Year() const { return m_CurrentYear; }

  void  Print(LedControl &segDisp, int offset) const;

private:
  int32_t m_CurrentYear;
};

//----------------------------------------------------------------------------
