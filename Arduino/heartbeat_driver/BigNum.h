
#include "Helpers.h"
#include "7SegDriver.h"

//----------------------------------------------------------------------------

struct  SBigNum
{
  uint64_t        hbc_0;
  uint32_t        hbc_1;

  SBigNum() : hbc_0(0), hbc_1(0) {}

  bool  operator != (const SBigNum &other) { return hbc_0 != other.hbc_0 || hbc_1 != other.hbc_1; }

  void  operator += (uint64_t x);

  void  PrintToSerial() const;
  void  PrintTo7Seg(LedControl &segDisp, int offset) const;
};

//----------------------------------------------------------------------------
