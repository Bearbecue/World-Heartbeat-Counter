#pragma once
//----------------------------------------------------------------------------
//  Various helper functions

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

//----------------------------------------------------------------------------

template<typename _T, typename _TA, typename _TB>
static inline _T clamp(_T v, _TA vMin, _TB vMax) { return v < vMin ? vMin : v > vMax ? vMax : v; }

//----------------------------------------------------------------------------

#define STR_STARTS_WITH(__str, __literal) (!strncmp(__str, __literal, sizeof(__literal)-1))

//----------------------------------------------------------------------------

struct  SVec2I
{
  int x;
  int y;
  SVec2I() : x(0), y(0) {}
  SVec2I(int ax, int ay) : x(ax), y(ay) {}
};

//----------------------------------------------------------------------------

struct  SVec2F
{
  float x;
  float y;
  SVec2F() : x(0), y(0) {}
  SVec2F(float ax, float ay) : x(ax), y(ay) {}
  SVec2F(const struct SVec2I &vec) : x(vec.x), y(vec.y) {}
};

//----------------------------------------------------------------------------
