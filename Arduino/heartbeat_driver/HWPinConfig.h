//----------------------------------------------------------------------------
// GPIO pin defs

#define PIN_OUT_DISP_MOSI   8  // SCL (FastLed crashes the board on init if using GPIO8 (HW MOSI)
#define PIN_OUT_DISP_CLK    7  // SDA (FastLed crashes the board on init if using GPIO6 (HW CLK)
#define PIN_OUT_RAIL0_CS    6  // (FastLed crashes the board on init if using GPIO11 (HW CS0)
#define PIN_OUT_RAIL1_CS    5  // We can use these pins if we need more rails

#define PIN_IN_BPM(__id)    (A4 + 3 - (__id))

//----------------------------------------------------------------------------

#define PIN_IN_BPM_0        PIN_IN_BPM(0)
#define PIN_IN_BPM_1        PIN_IN_BPM(1)
#define PIN_IN_BPM_2        PIN_IN_BPM(2)
#define PIN_IN_BPM_3        PIN_IN_BPM(3)

//----------------------------------------------------------------------------

#define DATE_ROTENC_PIN_A   3
#define DATE_ROTENC_PIN_B   4

//----------------------------------------------------------------------------
// Wire mappings
//  PIN_OUT_DISP*_MOSI -> brown wire
//  PIN_OUT_DISP*_CLK  -> red wire
//  PIN_OUT_DISP*_CS   -> yellow wire
//
// Wire mappings 2
//  PIN_OUT_DISP*_MOSI -> yellow wire
//  PIN_OUT_DISP*_CLK  -> blue wire
//  PIN_OUT_DISP*_CS   -> green wire
