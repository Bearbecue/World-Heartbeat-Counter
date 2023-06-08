//----------------------------------------------------------------------------

#include "ControllerHeartbeat.h"
#include "HWPinConfig.h"
#include "PersistentMemory.h"

const int   kBPMMin = 30;
const int   kBPMMax = 200;
const int   kSyncDivider = 64;  // [0, 1023] -> [0, 16]

//----------------------------------------------------------------------------

HeartbeatController::HeartbeatController()
: m_BPMCurve(NULL)
, m_CurrentTimeAcc(0)
, m_Population(0)
, m_PeakBPM(60)
, m_SyncLevel(0)
{
  memset(m_TimeAcc, 0, sizeof(m_TimeAcc));
  memset(m_Countdown, 0, sizeof(m_Countdown));
  memset(m_Carry, 0, sizeof(m_Carry));
}

//----------------------------------------------------------------------------

void  HeartbeatController::Setup(const BPMController *bpmController)
{
  pinMode(PIN_IN_SYNC, INPUT);
  pinMode(PIN_IN_SRR, INPUT); // Save/Restore/Reset pin

  m_BPMCurve = bpmController;
  m_CurrentBPMSampleCount = 1;
  m_SyncLevel = analogRead(PIN_IN_SYNC) / kSyncDivider;

  _RebuildBPMKernel();

  LoadState();
}

//----------------------------------------------------------------------------

void  HeartbeatController::LoadState()
{
  m_Counter = eeprom_read_beats(g_CurrentStateID);
}

//----------------------------------------------------------------------------

void  HeartbeatController::WriteState()
{
  eeprom_write_beats(g_CurrentStateID, m_Counter);
}

//----------------------------------------------------------------------------

void  HeartbeatController::SetPopulation(uint64_t population)
{
  m_Population = population;
}

//----------------------------------------------------------------------------

void  HeartbeatController::DirtyBPMCurve()
{
  _RebuildBPMKernel();
}

//----------------------------------------------------------------------------

void  HeartbeatController::_RebuildBPMKernel()
{
  // Compute sample count from sync level
  if (m_SyncLevel >= 14)
    m_CurrentBPMSampleCount = 1;
  else if (m_SyncLevel >= 12)
    m_CurrentBPMSampleCount = 2;
  else if (m_SyncLevel >= 10)
    m_CurrentBPMSampleCount = 3;
  else if (m_SyncLevel >= 8)
    m_CurrentBPMSampleCount = 4;
  else if (m_SyncLevel >= 6)
    m_CurrentBPMSampleCount = 6;
  else if (m_SyncLevel >= 4)
    m_CurrentBPMSampleCount = 10;
  else if (m_SyncLevel >= 2)
    m_CurrentBPMSampleCount = kMaxBPMSampleCount;
  else
    m_CurrentBPMSampleCount = kMaxBPMSampleCount; // TODO: Uniform increments

/*  Serial.print(m_SyncLevel);
  Serial.print(" - ");
  Serial.println(m_CurrentBPMSampleCount);*/

  if (m_BPMCurve == NULL)
    return;

  // If we have a single sample, find the highest BPM
  if (m_CurrentBPMSampleCount == 1)
  {
    const int bpmSampleCount = kMaxBPMSampleCount;
    m_BPMCurve->Sample(m_BPMKernel, bpmSampleCount);

    float peakBPMRate = 0.0f;
    m_PeakBPM = 0;
    for (int i = 0; i < bpmSampleCount; i++)
    {
      const float kv = m_BPMKernel[i];
      if (kv > peakBPMRate)
      {
        peakBPMRate = kv;
        m_PeakBPM = COMPUTE_BPM_FROM_SAMPLING_INDEX(i, bpmSampleCount, kBPMMin, kBPMMax);
      }
    }
    m_BPMKernel[0] = 1.0f;
  }
  else
  {
    const int bpmSampleCount = m_CurrentBPMSampleCount;
    const float kernelSum = m_BPMCurve->Sample(m_BPMKernel, bpmSampleCount);
    const float normalizer = kernelSum != 0 ? (1.0f / kernelSum) : 1.0f;
  
    float peakBPMRate = 0.0f;
    m_PeakBPM = 0;
    for (int i = 0; i < bpmSampleCount; i++)
    {
      const float kv = m_BPMKernel[i] * normalizer;
      m_BPMKernel[i] = kv;
      if (kv > peakBPMRate)
      {
        peakBPMRate = kv;
        m_PeakBPM = COMPUTE_BPM_FROM_SAMPLING_INDEX(i, bpmSampleCount, kBPMMin, kBPMMax);
      }
    }
  }
}

//----------------------------------------------------------------------------

static float  _integratePulse01(float t0, float t1, float k)
{
  float x = 2*t0 - 1;
  float y = 2*t1 - 1;
  float num = (y*y - 4*t1 - x*x + 4*t0) * k;
  return abs(num / 4 + t1 - t0);
}

//----------------------------------------------------------------------------

static float  _integratePulse(float t0, float t1, float k)
{
  float sum = 0;
  if (t0 < 0.0f)
  {
    sum += _integratePulse01(1.0f - t0, 1.0f, k);
    sum += _integratePulse01(0.0f, t1, k);
  }
  else
  {
    sum += _integratePulse01(t0, t1, k);
  }
  return sum;
}

//----------------------------------------------------------------------------

void  HeartbeatController::_UpdateLoCount(SBigNum &newHBCount, int dtMS)
{
  int       accountedPop = 0;

  const int bpmSampleCount = m_CurrentBPMSampleCount;
  for (int i = 0; i < bpmSampleCount; i++)
  {
    const int targetBPM = bpmSampleCount == 1 ?
                          m_PeakBPM :
                          COMPUTE_BPM_FROM_SAMPLING_INDEX(i, bpmSampleCount, kBPMMin, kBPMMax);
    const int nextBeatDelay = (60 * 1000L) / targetBPM; // in ms

    const int popCount = m_BPMKernel[i] * m_Population;
    accountedPop += popCount;

    m_TimeAcc[i] += dtMS;
    if (m_TimeAcc[i] >= nextBeatDelay)
    {
      m_TimeAcc[i] -= nextBeatDelay;
      newHBCount += popCount;
    }
  }

  const int missing = m_Population - accountedPop;
  if (missing > 0)
  {
    const int targetBPM = m_PeakBPM;
    const int bpmScale = (m_CurrentBPMSampleCount == 1) ? 1 : missing;
    int       nextBeatDelay = (uint32_t(1000) * 60) / (targetBPM * bpmScale); // in ms

    m_CurrentTimeAcc += dtMS;
    if (m_CurrentTimeAcc >= nextBeatDelay)
    {
      m_CurrentTimeAcc -= nextBeatDelay;
      newHBCount += missing / bpmScale;
    }
  }
}

//----------------------------------------------------------------------------

void  HeartbeatController::_UpdateHiCount(SBigNum &newHBCount, int dtMS)
{
//#define INSTANT_BEAT

  const int bpmSampleCount = m_CurrentBPMSampleCount;

  for (int i = 0; i < bpmSampleCount; i++)
  {
    const int targetBPM = bpmSampleCount == 1 ?
                          m_PeakBPM :
                          COMPUTE_BPM_FROM_SAMPLING_INDEX(i, bpmSampleCount, kBPMMin, kBPMMax);
    const int nextBeatDelay = (60 * 1000L) / targetBPM; // in ms

    //Serial.println(targetBPM);
#if 1
    const int   countDownWidth = 30 + 1;
    const float k = 10;
    const float kNorm = 0.42386f;
#else
    const int   countDownWidth = 50 + 1;
    const float k = 100;
    const float kNorm = 0.9925096f;
#endif
    m_TimeAcc[i] += dtMS;
    if (m_TimeAcc[i] >= nextBeatDelay)
    {
      m_TimeAcc[i] -= nextBeatDelay;
      m_Countdown[i] = countDownWidth;
//      Serial.println(v);
#ifdef INSTANT_BEAT
      newHBCount += m_BPMKernel[i] * m_Population;
#endif
    }

#ifndef INSTANT_BEAT
    if (m_Countdown[i] > 0)
    {
      m_Countdown[i]--;
      const float cursor = 1.0f - 2*(m_Countdown[i] / float(countDownWidth-1));  // [-1, 1]

      const float x = k * cursor;
      const float den = 1 + x * x;
      float v = 1.0f / (den * den);
      v *= m_BPMKernel[i];
      v *= kNorm;

      float   hbIncF = v * m_Population + m_Carry[i];
      int64_t hbIncI = hbIncF;
      m_Carry[i] = hbIncI - hbIncF;
      newHBCount += hbIncI;
    }
#endif
  }
}

//----------------------------------------------------------------------------

void  HeartbeatController::Reset()
{
  m_Counter = SBigNum(); // Reset count to zero !
}

//----------------------------------------------------------------------------

bool  HeartbeatController::Update(int dtMS)
{
  {
    const int rawSync = analogRead(PIN_IN_SYNC) / kSyncDivider;
    if (m_SyncLevel != rawSync)
    {
      m_SyncLevel = rawSync;
      _RebuildBPMKernel();
    }
  }

  {
    static int  prevStateID = 0;
    if (prevStateID != g_CurrentStateID)
    {
      prevStateID = g_CurrentStateID;
      return true;
    }
  }

  SBigNum newHBCount = m_Counter;
  if (m_Population <= 200)
  {
    _UpdateLoCount(newHBCount, dtMS);
  }
  else
  {
#if 1
    _UpdateHiCount(newHBCount, dtMS);
#else
    // when sync == 1, everyone beats at the same tick    (discrete spike curve, slope = +inf)
    // when sync == 0, everyone beats at a different tick (flat curve, slope = 0)
    float sync = 1;//clamp(input / 1000.0f, 0.0f, 1.0f);//1.0f; // should be 1023, but allow for pot imprecisions & voltage losses
    bool  synchronized = true;//sync > 0.99f;
    float total_integral = 1.0f;
    if (!synchronized)
    {
      int nextBeatDelay = (uint32_t(1000) * 60) / 68; // in ms
      float v = 0.0f;

      static float  prevT = 0.0f;
      float t = m_CurrentTime / float(nextBeatDelay);
      if (m_CurrentTime >= nextBeatDelay)
      {
        while (m_CurrentTime >= nextBeatDelay)
        {
          m_CurrentTime -= nextBeatDelay;
          t -= 1.0f;
        }
        prevT -= 1.0f;
      }
  
      float slope = 1.0f / (1 - sync);
      //v = slope * abs(t - 0.5f) * 2 - (slope - 1);
      total_integral = -(slope - 2) / 2;
      v = _integratePulse(prevT, t, slope) / total_integral;
  
      Serial.print("[");
      Serial.print(prevT);
      Serial.print(", ");
      Serial.print(t);
      Serial.print("]: ");
      Serial.println(v);
  
      prevT = t;

      if (v != 0)
      {
        newHBCount += v * m_Population;
    
        // 2892479536812345671
        // 10000000000000000000
        // 11561240708638945646987
      }
    }
    else
    {
#if 1
#else
      int nextBeatDelay = (uint32_t(1000) * 60) / 68; // in ms
      float v = 0.0f;
#if 0
      const int   countDownWidth = 30 + 1;
      const float k = 10;
      const float kNorm = 0.42386f;
#else
      const int   countDownWidth = 50 + 1;
      const float k = 100;
      const float kNorm = 0.9925096f;
#endif
      static int  countDown = 0;

      if (m_CurrentTime >= nextBeatDelay)
      {
        m_CurrentTime -= nextBeatDelay;
        total_integral = 1.0f;
        v = 0.0f;
        countDown = countDownWidth;
      }
  
      if (countDown > 0)
      {
        countDown--;
        const float cursor = 1.0f - 2*(countDown / float(countDownWidth-1));  // [-1, 1]
  
        const float x = k * cursor;
        const float den = 1 + x * x;
        v = 1.0f / (den * den);

        v *= kNorm;
      }

      if (v != 0)
      {
        newHBCount += v * m_Population;
    
        // 2892479536812345671
        // 10000000000000000000
        // 11561240708638945646987
      }
#endif
    }
#endif
  }

  const bool  changed = (newHBCount != m_Counter);
  m_Counter = newHBCount;

  // periodically backup the current count to the EEPROM
  // in case there is a power-cut
  {
    static int32_t  dirtyDelayMS = 0;
    dirtyDelayMS += dtMS;
    if (dirtyDelayMS <= 5 * 60 * 1000)  // check it every 5 minutes (will actually take a lot longer to write something, because the low bits of the count are cleared before saving)
    {
      dirtyDelayMS = 0;
      WriteState();
    }
  }

  return changed;
}

//----------------------------------------------------------------------------

void  HeartbeatController::Print(LedControl &segDisp, int offset) const
{
  m_Counter.PrintTo7Seg(segDisp, offset);
}

//----------------------------------------------------------------------------
