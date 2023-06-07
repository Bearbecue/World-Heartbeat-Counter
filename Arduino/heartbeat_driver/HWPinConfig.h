//----------------------------------------------------------------------------
// GPIO pin defs

#define HW_MK 1


#define PIN_OUT_DISP_MOSI   8   // SCL (FastLed crashes the board on init if using GPIO8 (HW MOSI)
#define PIN_OUT_DISP_CLK    6   // SDA (FastLed crashes the board on init if using GPIO6 (HW CLK)
#define PIN_OUT_RAIL0_CS    7   // (FastLed crashes the board on init if using GPIO11 (HW CS0)
#define PIN_OUT_RAIL1_CS    3
#define PIN_OUT_RAIL2_CS    5

#define PIN_OUT_LED_BIRTH   10  // PWM pin
#define PIN_OUT_LED_DEATH   11  // PWM pin
// Available: 12

//----------------------------------------------------------------------------
#if (HW_MK == 1)

# define PIN_IN_BPM(__id)    (A4 + (__id))
# define PIN_IN_BPM_0        PIN_IN_BPM(0) // A4
# define PIN_IN_BPM_1        PIN_IN_BPM(1) // A5
# define PIN_IN_BPM_2        PIN_IN_BPM(2) // A6
# define PIN_IN_BPM_3        PIN_IN_BPM(3) // A7

# define PIN_IN_SYNC         A3  // Beat synchronization
# define PIN_IN_SRR          A2  // Save/Restore/Reset

# define DATE_ROTENC_PIN_A   2
# define DATE_ROTENC_PIN_B   4

#else // MK2 and above

# define PIN_IN_BPM(__id)    (A3 + (__id))
# define PIN_IN_BPM_0        PIN_IN_BPM(0) // A6
# define PIN_IN_BPM_1        PIN_IN_BPM(1) // A5
# define PIN_IN_BPM_2        PIN_IN_BPM(2) // A4
# define PIN_IN_BPM_3        PIN_IN_BPM(3) // A3

# define PIN_IN_SYNC         A2  // Beat synchronization
# define PIN_IN_SRR          A7  // Save/Restore/Reset

# define DATE_ROTENC_PIN_A   3
# define DATE_ROTENC_PIN_B   4

#endif

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
