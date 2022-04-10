//----------------------------------------------------------------------------

#include "ControllerHeartbeat.h"

const int   kBPMMin = 30;
const int   kBPMMax = 200;

//----------------------------------------------------------------------------

HeartbeatController::HeartbeatController()
: m_BPMCurve(NULL)
, m_CurrentTime(0)
, m_Population(0)
, m_PeakBPM(60)
{
  memset(m_TimeAcc, 0, sizeof(m_TimeAcc));
  memset(m_Countdown, 0, sizeof(m_Countdown));
}

//----------------------------------------------------------------------------

void  HeartbeatController::Setup(const BPMController *bpmController)
{
  m_BPMCurve = bpmController;

  m_CurrentBPMSampleCount = 1;

  _RebuildBPMKernel();
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
  int nextBeatDelay = (uint32_t(1000) * 60) / 68; // in ms
  if (m_CurrentTime >= nextBeatDelay)
  {
    m_CurrentTime -= nextBeatDelay;
    newHBCount += 1;
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

      newHBCount += v * m_Population;
    }
#endif

//    Serial.println(i);
  }
}

//----------------------------------------------------------------------------

bool  HeartbeatController::Update(int dtMS)
{
  m_CurrentTime += dtMS;

  SBigNum newHBCount = m_Counter;

  if (m_Population <= 2)
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

  if (newHBCount == m_Counter)
    return false;

  m_Counter = newHBCount;
  return true;
}

//----------------------------------------------------------------------------

void  HeartbeatController::Print(LedControl &segDisp, int offset) const
{
  m_Counter.PrintTo7Seg(segDisp, offset);
}

//----------------------------------------------------------------------------