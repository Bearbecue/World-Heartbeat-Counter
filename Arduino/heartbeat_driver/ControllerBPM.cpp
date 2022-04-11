//----------------------------------------------------------------------------

#include "HWPinConfig.h"
#include "ControllerBPM.h"

//----------------------------------------------------------------------------

#define BPM_RATIO_QUANTIZER  32

const int   kBPMScaleMax = 800;
const int   kBPMHistorySize = 32;

int         bpmRatioHistories[4][kBPMHistorySize] = {};
int         bpmRatioHistoryIndex = 0;

float       bpmRatios[4] = {};
const int   kCellsX = 4;
const int   kCellsY = 2;

const int   kDimX = kCellsX*8;
const int   kDimY = kCellsY*8;

//const int   kBPMMin = 50;
//const int   kBPMMax = 200;
//const int   kBPMSampleCount = kBPMMax-kBPMMin;
//float       bpmDistribution[kBPMSampleCount];

SVec2I kBPMPoints[] =
{
  SVec2I(30, 0),
  SVec2I(50, 0),
  SVec2I(75, kBPMScaleMax),
  SVec2I(100, 300),
  SVec2I(150, 100),
  SVec2I(200, 0),
};
const int   kPointCount = sizeof(kBPMPoints) / sizeof(kBPMPoints[0]);

//----------------------------------------------------------------------------

BPMController::BPMController()
{
}

//----------------------------------------------------------------------------

void  BPMController::Setup()
{
  // Setup pot pins as input
  for (int i = 0; i < 4; i++)
    pinMode(PIN_IN_BPM(i), INPUT);

  // Read pot values & setup initial histogram
  bpmRatioHistoryIndex = 0;
  for (int i = 0; i < 4; i++)
  {
    const int rawValue = analogRead(PIN_IN_BPM(i)) / BPM_RATIO_QUANTIZER;
    for (int j = 0; j < kBPMHistorySize; j++)
      bpmRatioHistories[i][j] = rawValue;
    bpmRatios[i] = rawValue;
  }

  // Update BPM curve control-points
  for (int i = 0; i < 4; i++)
    kBPMPoints[1 + i].y = clamp(int(bpmRatios[i]) * BPM_RATIO_QUANTIZER, 0, kBPMScaleMax);
}

//----------------------------------------------------------------------------

bool  BPMController::Update()
{
  for (int i = 0; i < 4; i++)
  {
    const int rawValue = analogRead(PIN_IN_BPM(i));
    bpmRatioHistories[i][bpmRatioHistoryIndex] = rawValue / BPM_RATIO_QUANTIZER;
  }
  bpmRatioHistoryIndex = (bpmRatioHistoryIndex + 1) % kBPMHistorySize;

  bool  dirty = false;
  for (int i = 0; i < 4; i++)
  {
    int sum = 0;
    for (int j = 0; j < kBPMHistorySize; j++)
      sum += bpmRatioHistories[i][j];
    float newVF = sum / float(kBPMHistorySize);

//    int newValue = sum / kBPMHistorySize;
    if (bpmRatios[i] - 0.5f > newVF ||
        bpmRatios[i] + 0.5f < newVF)
    {
      dirty = true;
      bpmRatios[i] = newVF;
      kBPMPoints[1 + i].y = clamp(int(bpmRatios[i]) * BPM_RATIO_QUANTIZER, 0, kBPMScaleMax);
    }
  }

  return dirty;
}

//----------------------------------------------------------------------------

#if 1
float _InterpolateBPMCP(const struct SVec2F &p0, const struct SVec2F &p1, const struct SVec2F &p2, const struct SVec2F &p3, float f)
{
  const float f2 = f * f;
  const float f3 = f2 * f;

  const float wt1 = f3 - f2;
  const float wt0 = wt1 + (f - f2);
  const float wp1 = f2 - (wt1 + wt1);

  SVec2F  t0(p1.x - p0.x, p1.x - p0.x);
  SVec2F  t1(p3.x - p2.x, p3.y - p2.y);

  const float kT = 0.5f;

  return p1.y + (p2.y - p1.y) * wp1 + t0.y * kT * wt0 + t1.y * kT * wt1;
}
#else
float _InterpolateBPMCP(const struct SVec2I &p0, const struct SVec2I &p1, float t)
{
  return p0.y + (p1.y - p0.y) * t;
}
#endif

//----------------------------------------------------------------------------

void  BPMController::DrawCurve(LedControl &segDisp)
{
#if 0

  for (int x = 0; x < kDimX; x++)
  {
    segDisp.setRawData((x % 8) + 8 * (1 + (x / 8) * kCellsY), 0xFF);
    segDisp.setRawData((x % 8) + 8 * (0 + (x / 8) * kCellsY), 0xFF);
  }

  segDisp.flushDeviceState();
#else
  int bpmStart = kBPMPoints[0].x;
  int bpmEnd = kBPMPoints[kPointCount-1].x;

  const SVec2I  kPreStartPoint(bpmStart - 20, 0);
  const SVec2I  kPostEndPoint(bpmEnd + 20, 0);
  int curCPId = 0;
  int curCPX = kBPMPoints[curCPId].x;
  int nextCPX = kBPMPoints[curCPId+1].x;
  int yPrev = 0;
  const float kBPMNormalizer = 1.0f / kBPMScaleMax;

  for (int x = 0; x < kDimX; x++)
  {
    const int bpm = COMPUTE_BPM_FROM_SAMPLING_INDEX(x, kDimX, bpmStart, bpmEnd);
    while (bpm > nextCPX)
    {
      curCPId++;
      if (curCPId >= kPointCount-1)
        break;
      curCPX = nextCPX;
      nextCPX = kBPMPoints[curCPId+1].x;
    }
    const float t = (bpm - curCPX) / float(nextCPX - curCPX);
#if 1
    const int yCur = clamp(int(_InterpolateBPMCP( curCPId > 0 ? kBPMPoints[curCPId - 1] : kPreStartPoint,
                                            kBPMPoints[curCPId],
                                            kBPMPoints[curCPId + 1],
                                            curCPId + 2 < kPointCount ? kBPMPoints[curCPId + 2] : kPostEndPoint,
                                            t) * kDimY * kBPMNormalizer), 0, kDimY-1);
#else
    const int yCur = clamp(int(_InterpolateBPMCP(kBPMPoints[curCPId], kBPMPoints[curCPId + 1], t) * kDimY * kBPMNormalizer), 0, kDimY-1);
#endif

    // Draw line between 'yPrev' and 'yCur'
    byte  colBits[2] = {};
    colBits[0] = 0;
    colBits[1] = 0;
#if 0
    for (int y = 0; y <= yCur; y++)
      colBits[y / 8] |= 1 << (7 - y % 8);
#else
    if (yPrev != yCur)
    {
      int yInc = yPrev <= yCur ? 1 : -1;
      int yOff = yPrev <= yCur ? 1 : -1;
      
      for (int y = yPrev + yOff; y != yCur + yOff; y += yInc)
        colBits[y / 8] |= 1 << (7 - y % 8);

      yPrev = yCur;
    }
    else
      colBits[yCur / 8] |= 1 << (7 - yCur % 8);
#endif

    colBits[0] = (colBits[0] >> 1) | (colBits[0] << 7);
    colBits[1] = (colBits[1] >> 1) | (colBits[1] << 7);
    segDisp.setRawData((x % 8) + 8 * (1 + (x / 8) * kCellsY), colBits[0]);
    segDisp.setRawData((x % 8) + 8 * (0 + (x / 8) * kCellsY), colBits[1]);
  }

  segDisp.flushDeviceState();
#endif
}

//----------------------------------------------------------------------------

float  BPMController::Sample(float *dst, int sampleCount)
{
  int bpmStart = kBPMPoints[0].x;
  int bpmEnd = kBPMPoints[kPointCount-1].x;

  const SVec2I  kPreStartPoint(bpmStart - 20, 0);
  const SVec2I  kPostEndPoint(bpmEnd + 20, 0);
  int curCPId = 0;
  int curCPX = kBPMPoints[curCPId].x;
  int nextCPX = kBPMPoints[curCPId+1].x;
  int yPrev = 0;
  const float kBPMNormalizer = 1.0f / kBPMScaleMax;

  float  sum = 0;
  for (int x = 0; x < sampleCount; x++)
  {
    const int bpm = COMPUTE_BPM_FROM_SAMPLING_INDEX(x, sampleCount, bpmStart, bpmEnd);
    while (bpm > nextCPX)
    {
      curCPId++;
      if (curCPId >= kPointCount-1)
        break;
      curCPX = nextCPX;
      nextCPX = kBPMPoints[curCPId+1].x;
    }
    const float t = (bpm - curCPX) / float(nextCPX - curCPX);
#if 1
    const float yCur = clamp(_InterpolateBPMCP( curCPId > 0 ? kBPMPoints[curCPId - 1] : kPreStartPoint,
                                            kBPMPoints[curCPId],
                                            kBPMPoints[curCPId + 1],
                                            curCPId + 2 < kPointCount ? kBPMPoints[curCPId + 2] : kPostEndPoint,
                                            t) * kBPMNormalizer, 0.0f, 1.0f);
#else
    const int yCur = clamp(_InterpolateBPMCP(kBPMPoints[curCPId], kBPMPoints[curCPId + 1], t) * kBPMNormalizer, 0.0f, 1.0f);
#endif
    dst[x] = yCur;

    sum += yCur;
  }
  return sum;
}

//----------------------------------------------------------------------------
