#pragma once

#include "Helpers.h"
#include "7SegDriver.h"

//----------------------------------------------------------------------------

struct  SBigNum
{
  uint64_t        m_HBC0;
  uint32_t        m_HBC1;

  SBigNum() : m_HBC0(0), m_HBC1(0) {}

  bool  operator != (const SBigNum &other) { return m_HBC0 != other.m_HBC0 || m_HBC1 != other.m_HBC1; }
  bool  operator == (const SBigNum &other) { return !(*this != other); }

  void  operator += (uint64_t x);

  void  PrintToSerial() const;
  void  PrintTo7Seg(LedControl &segDisp, int offset) const;
};

//----------------------------------------------------------------------------
