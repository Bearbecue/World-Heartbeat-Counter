//----------------------------------------------------------------------------

#include "PersistentMemory.h"
#include <EEPROM.h>

//----------------------------------------------------------------------------

static const int  kStateSize = sizeof(int32_t) +  // date
                               sizeof(int64_t) +  // beats.m_HBC0
                               sizeof(int32_t);   // beats.m_HBC1

//----------------------------------------------------------------------------

int     g_CurrentStateID = 0;

//----------------------------------------------------------------------------

int     eeprom_read_state()
{
  int32_t state = 0;
  EEPROM.get(0, state);
  return clamp(state, 0, kMaxStates);
}

void    eeprom_write_state(int stateID)
{
  int32_t state = clamp(stateID, 0, kMaxStates);
  EEPROM.put(0, state);
}

//----------------------------------------------------------------------------

int32_t eeprom_read_date(int stateID)
{
  int       readPos = sizeof(int32_t) + stateID * kStateSize + 0; // skip stateID + prev states
  int32_t   date = 0;
  EEPROM.get(readPos, date);
  return date;
}

void eeprom_write_date(int stateID, int32_t date)
{
  int       writePos = sizeof(int32_t) + stateID * kStateSize + 0; // skip stateID + prev states
  EEPROM.put(writePos, date);
}

//----------------------------------------------------------------------------

SBigNum eeprom_read_beats(int stateID)
{
  int       readPos = sizeof(int32_t) + stateID * kStateSize + sizeof(int32_t); // skip stateID + prev states + date
  SBigNum   rBeats;
  EEPROM.get(readPos, rBeats.m_HBC0);
  readPos += sizeof(rBeats.m_HBC0);
  EEPROM.get(readPos, rBeats.m_HBC1);
  readPos += sizeof(rBeats.m_HBC1);
  return rBeats;
}

//----------------------------------------------------------------------------

uint64_t nexteq_power_of_two(uint64_t value)
{
  value--;
  for (u32 i = 1; i < sizeof(int64_t) * 8; i += i)
    value |= value >> i;
  value++;
  return value;
}

//----------------------------------------------------------------------------

void eeprom_write_beats(int stateID, const SBigNum &beats)
{
  int       writePos = sizeof(int32_t) + stateID * kStateSize + sizeof(int32_t); // skip stateID + prev states + date
  SBigNum   wBeats = beats;

  // Here, only keep the few last significant digits based on the number scale?
  uint64_t   clearMask = 0xFFFFF80000000000ULL;
  if (wBeats.m_HBC1 == 0)
  {
    const int64_t npot = nexteq_power_of_two(wBeats.m_HBC0);
    const int64_t highMask = ~(npot - 1);
    clearMask |= highMask >> 5;// Keep the high 5 bits
  }

  wBeats.m_HBC0 &= clearMask;  // Clear lower bits to write less data less often

  EEPROM.put(writePos, wBeats.m_HBC0);
  writePos += sizeof(wBeats.m_HBC0);
  EEPROM.put(writePos, wBeats.m_HBC1);
  writePos += sizeof(wBeats.m_HBC1);
}

//----------------------------------------------------------------------------
