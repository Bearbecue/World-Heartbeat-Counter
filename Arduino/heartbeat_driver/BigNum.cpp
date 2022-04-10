//----------------------------------------------------------------------------

#include "BigNum.h"

static const uint64_t  hbc_0_max = 10000000000000000000ULL;  // 1 digit short to simplify combination with u32 below
static const int       hbc_0_maxDigitCount = 19;
static const int       hbc_1_maxDigitCount = 10;

//----------------------------------------------------------------------------

void  SBigNum::operator += (uint64_t x)
{
  if (x >= hbc_0_max)  // invalid
    return;

  uint64_t  left = hbc_0_max - m_HBC0;
  while (x != 0)
  {
    if (left > x)
    {
      m_HBC0 += x;
      break;
    }

    x -= left;
    m_HBC0 = 0;
    m_HBC1 += 1;

    left = m_HBC0 - hbc_0_max;
  }
}

//----------------------------------------------------------------------------

void SBigNum::PrintToSerial() const
{
  uint64_t  hbc_0 = m_HBC0;
  uint32_t  hbc_1 = m_HBC1;

  char  digits[hbc_0_maxDigitCount + hbc_1_maxDigitCount + 1]; // max number of digits in both nums + null terminating character
  int   digitCount = 0;
  while (hbc_0 != 0)
  {
    digits[digitCount++] = '0' + (hbc_0 % 10);
    hbc_0 /= 10;
  }

  if (hbc_1 != 0)
  {
    while (digitCount < hbc_0_maxDigitCount)
      digits[digitCount++] = '0';
    while (hbc_1 != 0)
    {
      digits[digitCount++] = '0' + (hbc_1 % 10);
      hbc_1 /= 10;
    }
  }

  if (digitCount == 0)
    digits[digitCount++] = '0';

  // flip string
  for (int i = 0, e = digitCount / 2; i < e; i++)
  {
    char  c = digits[i];
    digits[i] = digits[digitCount - i - 1];
    digits[digitCount - i - 1] = c;
  }
  digits[digitCount] = '\0';

  Serial.println(digits);
}

//----------------------------------------------------------------------------

void SBigNum::PrintTo7Seg(LedControl &segDisp, int offset) const
{
  uint64_t  hbc_0 = m_HBC0;
  uint32_t  hbc_1 = m_HBC1;

  const int kMaxValueDigits = 20;//hbc_0_maxDigitCount + hbc_1_maxDigitCount;
  byte  digits[hbc_0_maxDigitCount + hbc_1_maxDigitCount]; // max number of digits in both nums + null terminating character
  int   digitCount = 0;
  while (hbc_0 != 0)
  {
    digits[digitCount++] = hbc_0 % 10;
    hbc_0 /= 10;
  }

  if (hbc_1 != 0)
  {
    while (digitCount < hbc_0_maxDigitCount)
      digits[digitCount++] = '0';
    while (hbc_1 != 0)
    {
      digits[digitCount++] = hbc_1 % 10;
      hbc_1 /= 10;
    }
  }

  if (digitCount == 0)
    digits[digitCount++] = 0;

  bool  dot = false;
  for (int i = 0; i < digitCount; i++)
  {
    const int realDigId = kMaxValueDigits - i - 1;
    const int chipId = offset + realDigId / 8;
    const int digId = realDigId % 8;
    segDisp.setRawDigit(chipId * 8 + digId, digits[i], dot);
    dot = ((i + 1) % 3) == 0;
  }

  // Clear the remaining MSB digits
  for (int i = digitCount; i < kMaxValueDigits; i++)
  {
    const int realDigId = kMaxValueDigits - i - 1;
    const int chipId = offset + realDigId / 8;
    const int digId = realDigId % 8;
    segDisp.setRawChar(chipId * 8 + digId, ' ', false);
  }
}

//----------------------------------------------------------------------------
