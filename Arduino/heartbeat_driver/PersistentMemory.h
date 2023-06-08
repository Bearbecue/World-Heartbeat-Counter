#pragma once

#include "Helpers.h"
#include "BigNum.h"

//----------------------------------------------------------------------------

const int   kMaxStates = 8;
extern int  g_CurrentStateID;

int     eeprom_read_state();
void    eeprom_write_state(int stateID);

int32_t eeprom_read_date(int stateID);
void    eeprom_write_date(int stateID, int32_t date);
SBigNum eeprom_read_beats(int stateID);
void    eeprom_write_beats(int stateID, const SBigNum &beats);

//----------------------------------------------------------------------------
