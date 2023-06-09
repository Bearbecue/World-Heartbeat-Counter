//----------------------------------------------------------------------------

#include "ControllerPopulation.h"
#include "HWPinConfig.h"
#include <avr/pgmspace.h>

//#define DEBUG

//----------------------------------------------------------------------------

struct  SPopulationRecord
{
  int32_t date;
  int64_t population;
  float   birthRate;
};

//----------------------------------------------------------------------------

static const SPopulationRecord	kPopulationRecord[] PROGMEM =
{
  // Estimation of initial homo-sapiens fork :D
  // TODO: This should actually dip
  { -100000,         1ULL, 0.000000f },  // Birth of initial mutant with the homo sapiens seed
  { -100000+14,      1ULL, 1.000000f },  // started banging between 14 & 15 and spread the genes !
  { -100000+15,      2ULL, 0.500000f },
  { -100000+16,      3ULL, 0.000000f },
  { -100000+18,      3ULL, 0.000000f },
  { -100000+19,      5ULL, 3.000000f },  // Died at 19 eaten by a mountain lion or something
  { -100000+23,      4ULL, 0.000000f },  // Another offspring died squished by a mammoth
  { -100000+14+15,   4ULL, 0.000000f },  // Another offspring died squished by a mammoth
  { -100000+14+16,   5ULL, 0.250000f },  // First offspring started banging at 15
  { -100000+14+17,  10ULL, 0.500000f },  // banged quite a lot.
  { -100000+14+23,  24ULL, 0.400000f },  // Shit is getting out of hand
  { -100000+14+24,  21ULL, 0.350000f },
  { -99900,        169ULL, 0.220000f },  // Crap
  { -99800,        542ULL, 0.190000f },  // Who wouldve thought the earth will get fucked 100 000 years down the line?
  { -99600,       5673ULL, 0.160000f },
  { -99000,      41701ULL, 0.120000f },

  // Data from https://ourworldindata.org/grapher/population
  { -10000,    4432180ULL, 0.160000f },
  { -9000,     5616908ULL, 0.150000f },
  { -8000,     7242806ULL, 0.140000f },
  { -7000,     9577828ULL, 0.157000f },
  { -6000,    13201749ULL, 0.154000f },
  { -5000,    19075681ULL, 0.146000f },
  { -4000,    28774581ULL, 0.133000f },
  { -3000,    44487076ULL, 0.11900f },
  { -2000,    72585991ULL, 0.085000f },
  { -1000,   110419442ULL, 0.074000f },
  { 0,       232123684ULL, 0.050000f },
  { 100,     236904109ULL, 0.050000f },
  { 200,     240611492ULL, 0.050000f },
  { 300,     227549778ULL, 0.050000f },
  { 400,     241539566ULL, 0.050000f },
  { 500,     253237239ULL, 0.050000f },
  { 600,     271478618ULL, 0.060000f },
  { 700,     278185264ULL, 0.060000f },
  { 800,     285713765ULL, 0.060000f },
  { 900,     310967646ULL, 0.055000f },
  { 1000,    323407502ULL, 0.055000f },
  { 1100,    397923741ULL, 0.055000f },
  { 1200,    444750711ULL, 0.053000f },
  { 1300,    456389012ULL, 0.053000f },
  { 1347,    468712801ULL, 0.054000f }, // Black death start
  { 1351,    443603494ULL, 0.054000f }, // Black death end: Toll = 25million deaths
  { 1400,    442480691ULL, 0.053000f },
  { 1500,    503240510ULL, 0.053000f },
  { 1600,    515751788ULL, 0.052000f },
  { 1700,    591721802ULL, 0.051000f },
  { 1710,    613189682ULL, 0.050000f },
  { 1720,    642718358ULL, 0.050000f },
  { 1730,    664734880ULL, 0.050000f },
  { 1740,    695346984ULL, 0.050000f },
  { 1750,    745664445ULL, 0.050000f },
  { 1760,    779892196ULL, 0.048000f },
  { 1770,    818828252ULL, 0.048000f },
  { 1780,    891049054ULL, 0.048000f },
  { 1790,    931578091ULL, 0.048000f },
  { 1800,    984741151ULL, 0.048000f },
  { 1801,    987628322ULL, 0.048000f },
  { 1802,    991098738ULL, 0.048000f },
  { 1803,    994610260ULL, 0.048000f },
  { 1804,    998163240ULL, 0.048000f },
  { 1805,   1001758044ULL, 0.048000f },
  { 1806,   1005395067ULL, 0.048000f },
  { 1807,   1009074742ULL, 0.048000f },
  { 1808,   1012797513ULL, 0.048000f },
  { 1809,   1016563866ULL, 0.048000f },
  { 1810,   1022630197ULL, 0.048000f },
  { 1811,   1027487469ULL, 0.048000f },
  { 1812,   1033631136ULL, 0.048000f },
  { 1813,   1040443689ULL, 0.048000f },
  { 1814,   1047427292ULL, 0.048000f },
  { 1815,   1054586106ULL, 0.048000f },
  { 1816,   1061924513ULL, 0.048000f },
  { 1817,   1069447070ULL, 0.047000f },
  { 1818,   1077158552ULL, 0.047000f },
  { 1819,   1084757875ULL, 0.047000f },
  { 1820,   1092947422ULL, 0.047000f },
  { 1821,   1099331577ULL, 0.047000f },
  { 1822,   1106253783ULL, 0.047000f },
  { 1823,   1113514792ULL, 0.047000f },
  { 1824,   1120876169ULL, 0.047000f },
  { 1825,   1128342960ULL, 0.047000f },
  { 1826,   1135961188ULL, 0.047000f },
  { 1827,   1143684846ULL, 0.047000f },
  { 1828,   1151515422ULL, 0.047000f },
  { 1829,   1158898384ULL, 0.047000f },
  { 1830,   1166619859ULL, 0.047000f },
  { 1831,   1172266726ULL, 0.047000f },
  { 1832,   1178216298ULL, 0.047000f },
  { 1833,   1183674995ULL, 0.047000f },
  { 1834,   1189194865ULL, 0.046000f },
  { 1835,   1194777021ULL, 0.046000f },
  { 1836,   1200422586ULL, 0.046000f },
  { 1837,   1206132721ULL, 0.046000f },
  { 1838,   1211908617ULL, 0.046000f },
  { 1839,   1217612799ULL, 0.046000f },
  { 1840,   1224118692ULL, 0.046000f },
  { 1841,   1228821489ULL, 0.046000f },
  { 1842,   1234333753ULL, 0.046000f },
  { 1843,   1239786237ULL, 0.045000f },
  { 1844,   1245319917ULL, 0.045000f },
  { 1845,   1250936428ULL, 0.045000f },
  { 1846,   1256637463ULL, 0.045000f },
  { 1847,   1262424720ULL, 0.045000f },
  { 1848,   1268300005ULL, 0.045000f },
  { 1849,   1273394934ULL, 0.045000f },
  { 1850,   1278698170ULL, 0.045000f },
  { 1851,   1281044111ULL, 0.045000f },
  { 1852,   1283605123ULL, 0.045000f },
  { 1853,   1285405357ULL, 0.045000f },
  { 1854,   1287317863ULL, 0.045000f },
  { 1855,   1289344086ULL, 0.045000f },
  { 1856,   1291485555ULL, 0.045000f },
  { 1857,   1293743896ULL, 0.045000f },
  { 1858,   1296120810ULL, 0.045000f },
  { 1859,   1298874784ULL, 0.044000f },
  { 1860,   1303155263ULL, 0.044000f },
  { 1861,   1305490771ULL, 0.044000f },
  { 1862,   1309347024ULL, 0.044000f },
  { 1863,   1313556287ULL, 0.044000f },
  { 1864,   1317850567ULL, 0.044000f },
  { 1865,   1322223328ULL, 0.044000f },
  { 1866,   1326674072ULL, 0.044000f },
  { 1867,   1331210602ULL, 0.044000f },
  { 1868,   1335831902ULL, 0.044000f },
  { 1869,   1341040002ULL, 0.044000f },
  { 1870,   1348154272ULL, 0.044000f },
  { 1871,   1353185947ULL, 0.044000f },
  { 1872,   1360191149ULL, 0.044000f },
  { 1873,   1367778454ULL, 0.044000f },
  { 1874,   1375441175ULL, 0.044000f },
  { 1875,   1383207445ULL, 0.044000f },
  { 1876,   1391057502ULL, 0.044000f },
  { 1877,   1398962396ULL, 0.044000f },
  { 1878,   1406939006ULL, 0.043000f },
  { 1879,   1415456507ULL, 0.043000f },
  { 1880,   1425998653ULL, 0.043000f },
  { 1881,   1434253944ULL, 0.043000f },
  { 1882,   1444559322ULL, 0.043000f },
  { 1883,   1455464560ULL, 0.043000f },
  { 1884,   1466507494ULL, 0.043000f },
  { 1885,   1477744363ULL, 0.043000f },
  { 1886,   1489095893ULL, 0.043000f },
  { 1887,   1500609770ULL, 0.043000f },
  { 1888,   1512259811ULL, 0.043000f },
  { 1889,   1523683199ULL, 0.043000f },
  { 1890,   1536461987ULL, 0.043000f },
  { 1891,   1545852788ULL, 0.043000f },
  { 1892,   1556562451ULL, 0.043000f },
  { 1893,   1567026235ULL, 0.042000f },
  { 1894,   1577633469ULL, 0.042000f },
  { 1895,   1588397643ULL, 0.042000f },
  { 1896,   1599255683ULL, 0.042000f },
  { 1897,   1610265238ULL, 0.042000f },
  { 1898,   1621405678ULL, 0.042000f },
  { 1899,   1633219852ULL, 0.042000f },
  { 1900,   1647405022ULL, 0.042000f },
  { 1901,   1658785643ULL, 0.042000f },
  { 1902,   1672564382ULL, 0.042000f },
  { 1903,   1687012805ULL, 0.042000f },
  { 1904,   1701647404ULL, 0.042000f },
  { 1905,   1716473051ULL, 0.042000f },
  { 1906,   1731494784ULL, 0.042000f },
  { 1907,   1746736854ULL, 0.041000f },
  { 1908,   1762200207ULL, 0.041000f },
  { 1909,   1777067748ULL, 0.041000f },
  { 1910,   1793323592ULL, 0.041000f },
  { 1911,   1804920684ULL, 0.041000f },
  { 1912,   1817990952ULL, 0.041000f },
  { 1913,   1830461187ULL, 0.041000f },
  { 1914,   1843131589ULL, 0.041000f },
  { 1915,   1855949027ULL, 0.041000f },
  { 1916,   1868851504ULL, 0.041000f },
  { 1917,   1881853176ULL, 0.041000f },
  { 1918,   1895037042ULL, 0.041000f },
  { 1919,   1909044309ULL, 0.040000f },
  { 1920,   1926217425ULL, 0.040000f },
  { 1921,   1939587826ULL, 0.040000f },
  { 1922,   1955971631ULL, 0.040000f },
  { 1923,   1973057174ULL, 0.040000f },
  { 1924,   1990416385ULL, 0.040000f },
  { 1925,   2008052088ULL, 0.040000f },
  { 1926,   2025945381ULL, 0.040000f },
  { 1927,   2044097070ULL, 0.040000f },
  { 1928,   2062543972ULL, 0.040000f },
  { 1929,   2081683468ULL, 0.039000f },
  { 1930,   2104067378ULL, 0.039000f },
  { 1931,   2122176586ULL, 0.039000f },
  { 1932,   2143517525ULL, 0.039000f },
  { 1933,   2165540428ULL, 0.039000f },
  { 1934,   2187836793ULL, 0.039000f },
  { 1935,   2210412972ULL, 0.039000f },
  { 1936,   2233367401ULL, 0.039000f },
  { 1937,   2256671866ULL, 0.039000f },
  { 1938,   2280342907ULL, 0.039000f },
  { 1939,   2303096438ULL, 0.039000f },
  { 1940,   2327357739ULL, 0.039000f },
  { 1941,   2344788534ULL, 0.039000f },
  { 1942,   2363887111ULL, 0.038000f },
  { 1943,   2381758939ULL, 0.038000f },
  { 1944,   2399659920ULL, 0.038000f },
  { 1945,   2417413700ULL, 0.038000f },
  { 1946,   2435048602ULL, 0.038000f },
  { 1947,   2454045831ULL, 0.038000f },
  { 1948,   2474648090ULL, 0.038000f },
  { 1949,   2500821769ULL, 0.038000f },
  { 1950,   2536605808ULL, 0.037844f },
  { 1951,   2584034223ULL, 0.037542f },
  { 1952,   2630861688ULL, 0.037239f },
  { 1953,   2677609059ULL, 0.036937f },
  { 1954,   2724846751ULL, 0.036635f },
  { 1955,   2773019915ULL, 0.036332f },
  { 1956,   2822443253ULL, 0.036030f },
  { 1957,   2873306055ULL, 0.035727f },
  { 1958,   2925686678ULL, 0.035425f },
  { 1959,   2979576143ULL, 0.035386f },
  { 1960,   3035160180ULL, 0.035346f },
  { 1961,   3091843506ULL, 0.035307f },
  { 1962,   3150420759ULL, 0.035267f },
  { 1963,   3211000941ULL, 0.035228f },
  { 1964,   3273978271ULL, 0.034985f },
  { 1965,   3339583509ULL, 0.034741f },
  { 1966,   3407922630ULL, 0.034498f },
  { 1967,   3478770102ULL, 0.034254f },
  { 1968,   3551599432ULL, 0.034011f },
  { 1969,   3625680964ULL, 0.033503f },
  { 1970,   3700685676ULL, 0.032995f },
  { 1971,   3775760030ULL, 0.032487f },
  { 1972,   3851650585ULL, 0.031979f },
  { 1973,   3927780518ULL, 0.031471f },
  { 1974,   4003794178ULL, 0.030875f },
  { 1975,   4079480473ULL, 0.030279f },
  { 1976,   4154666824ULL, 0.029682f },
  { 1977,   4229505917ULL, 0.029086f },
  { 1978,   4304533597ULL, 0.028490f },
  { 1979,   4380506180ULL, 0.028341f },
  { 1980,   4458274952ULL, 0.028192f },
  { 1981,   4536996616ULL, 0.028044f },
  { 1982,   4617386524ULL, 0.027895f },
  { 1983,   4699569184ULL, 0.027746f },
  { 1984,   4784011512ULL, 0.027672f },
  { 1985,   4870921665ULL, 0.027598f },
  { 1986,   4960567998ULL, 0.027524f },
  { 1987,   5052521994ULL, 0.027450f },
  { 1988,   5145425992ULL, 0.027376f },
  { 1989,   5237441433ULL, 0.026735f },
  { 1990,   5327529078ULL, 0.026094f },
  { 1991,   5414289382ULL, 0.025453f },
  { 1992,   5498919891ULL, 0.024812f },
  { 1993,   5581597596ULL, 0.024171f },
  { 1994,   5663150426ULL, 0.023779f },
  { 1995,   5744212929ULL, 0.023387f },
  { 1996,   5824891927ULL, 0.022995f },
  { 1997,   5905045647ULL, 0.022603f },
  { 1998,   5984794072ULL, 0.022211f },
  { 1999,   6064239030ULL, 0.021972f },
  { 2000,   6143776621ULL, 0.021733f },
  { 2001,   6222912459ULL, 0.021495f },
  { 2002,   6302062210ULL, 0.021256f },
  { 2003,   6381477292ULL, 0.021017f },
  { 2004,   6461454653ULL, 0.020882f },
  { 2005,   6542205330ULL, 0.020746f },
  { 2006,   6623819401ULL, 0.020611f },
  { 2007,   6706251239ULL, 0.020475f },
  { 2008,   6789396380ULL, 0.020340f },
  { 2009,   6873077808ULL, 0.020166f },
  { 2010,   6957137521ULL, 0.019991f },
  { 2011,   7041509491ULL, 0.019817f },
  { 2012,   7126144677ULL, 0.019642f },
  { 2013,   7210900157ULL, 0.019468f },
  { 2014,   7295610265ULL, 0.019272f },
  { 2015,   7380117870ULL, 0.019075f },
  { 2016,   7464344232ULL, 0.018879f },
  { 2017,   7548182589ULL, 0.018682f },
  { 2018,   7631091110ULL, 0.018486f },
  { 2019,   7713468203ULL, 0.018282f },
  { 2020,   7794798725ULL, 0.018077f },
  { 2021,   7874965730ULL, 0.017873f },
  { 2022,   7901037895ULL, 0.017668f },
  { 2023,   7942645086ULL, 0.017464f },

  // Predictions
  { 2024,   8031800429ULL, 0.017280f },
  { 2025,   8108605388ULL, 0.017150f },
  { 2026,   8184045647ULL, 0.017004f },
  { 2027,   8259276737ULL, 0.016880f },
  { 2031,   8549794072ULL, 0.016390f },
  { 2036,   8888239030ULL, 0.015811f },
  { 2041,   9199776621ULL, 0.015329f },
  { 2046,   9482912459ULL, 0.014893f },
  { 2051,   9735062210ULL, 0.014446f },
  { 2056,   9958477292ULL, 0.014054f },
  { 2061,  10152454653ULL, 0.013673f },
  { 2066,  10318205330ULL, 0.013337f },
  { 2071,  10459819401ULL, 0.013036f },
  { 2076,  10577251239ULL, 0.012765f },
  { 2081,  10674396380ULL, 0.012504f },
  { 2086,  10750077808ULL, 0.012239f },
  { 2091,  10810150426ULL, 0.011980f },
  { 2096,  10852212929ULL, 0.011733f },
  { 2101,  10875891927ULL, 0.011497f },
};
const int kPopulationRecordSize = sizeof(kPopulationRecord) / sizeof(kPopulationRecord[0]);

//----------------------------------------------------------------------------

PopulationController::PopulationController()
: m_Population(0)
, m_PopulationNext(0)
, m_NextBirthDelay(0)
, m_NextDeathDelay(0)
, m_NextBirthCounter(0)
, m_NextDeathCounter(0)
, m_BirthOffset(0)
, m_DeathOffset(0)
, m_BirthLEDIntensity(0)
, m_DeathLEDIntensity(0)
{
}

//----------------------------------------------------------------------------

void  PopulationController::Setup()
{
  pinMode(PIN_OUT_LED_BIRTH, OUTPUT);
  pinMode(PIN_OUT_LED_DEATH, OUTPUT);
}

//----------------------------------------------------------------------------

uint64_t  _pgm_read_qword_near(void *ptr)
{
  uint32_t  pop0 = pgm_read_dword_near(ptr + 0);
  uint32_t  pop1 = pgm_read_dword_near(ptr + sizeof(int32_t));
  return pop0 | (uint64_t(pop1) << 32);
}

//----------------------------------------------------------------------------

float PopulationController::_GetPopulationAtDate(int32_t year)
{
  if (year <= int32_t(pgm_read_dword_near(&kPopulationRecord[0].date)))
  {
      m_Population = _pgm_read_qword_near(&kPopulationRecord[0].population);
      m_PopulationNext = _pgm_read_qword_near(&kPopulationRecord[1].population);
      return pgm_read_float_near(&kPopulationRecord[0].birthRate);
  }

  int32_t date = year;

#if 1 // Binary search
  int id = kPopulationRecordSize / 2;
  int wsize = kPopulationRecordSize;
  while (1)
  {
    if (id == 0)  // we can't be on zero, bug ! Caught it early on
      Serial.println("bug in binary search");
    int     id0 = id - 1;
    int     id1 = id;
    int32_t date0 = pgm_read_dword_near(&kPopulationRecord[id0].date);
    int32_t date1 = pgm_read_dword_near(&kPopulationRecord[id1].date);
    if (date < date0)
    {
      wsize = wsize / 2;
      id -= (wsize - wsize / 2);
    }
    else if (date > date1)
    {
      wsize -= wsize / 2;
      id += wsize / 2;
    }
    else
    {
      // we're on it !
      int64_t     pop0 = _pgm_read_qword_near(&kPopulationRecord[id0].population);
      int64_t     pop1 = _pgm_read_qword_near(&kPopulationRecord[id1].population);
      const float br0 = pgm_read_float_near(&kPopulationRecord[id0].birthRate);
      const float br1 = pgm_read_float_near(&kPopulationRecord[id0].birthRate);

      float birthRate = 0.0f;
      {
        float t = (date - date0) / float(date1 - date0);
        m_Population = pop0 + int64_t((pop1 - pop0) * t);
        birthRate = br0 + (br1 - br0) * t;
      }

#ifdef DEBUG
      Serial.println(String("P0: ") + int32_t(m_Population) + " BR: " + birthRate);
#endif

      // Fetch year + 1
      {
        date++;
//        Serial.println(date);
        if (date > date1) // we jumped to the next entry
        {
          id1++;
          if (id1 < kPopulationRecordSize)
          {
//            Serial.println(date);
//            Serial.println(date);
            date0 = date1;
            pop0 = pop1;
            date1 = pgm_read_dword_near(&kPopulationRecord[id1].date);
            pop1 = _pgm_read_qword_near(&kPopulationRecord[id1].population);
          }
          else
          {
            m_PopulationNext = 10890000000ULL;
            return birthRate;
          }
        }
        float t = (date - date0) / float(date1 - date0);
        m_PopulationNext = pop0 + int64_t((pop1 - pop0) * t);
      }

#ifdef DEBUG
      Serial.println(String("P1: ") + int32_t(m_PopulationNext) + " (" + int32_t(pop0) + ", " + int32_t(pop1) + ")");
#endif
      return birthRate;
    }
  }
#else // Reference implem: linear search
  float birthRate = 0.0f;
  int popCountID = 0;

  for (int i = 0; i < kPopulationRecordSize; i++)
  {
    const int32_t  date1 = pgm_read_dword_near(&kPopulationRecord[i].date);
    if (date1 >= date)
    {
      int id0 = i - 1;
      int id1 = i;
      const int32_t   date0 = pgm_read_dword_near(&kPopulationRecord[id0].date);
      const int64_t   pop0 = _pgm_read_qword_near(&kPopulationRecord[id0].population);
      const int64_t   pop1 = _pgm_read_qword_near(&kPopulationRecord[id1].population);

      float t = (date - date0) / float(date1 - date0);
      int64_t popcount = pop0 + int64_t((pop1 - pop0) * t);

      if (popCountID == 0)
      {
        m_Population = popcount;
        birthRate = pgm_read_float_near(&kPopulationRecord[id0].birthRate);
#ifdef DEBUG
        Serial.println(String("P0: ") + int32_t(popcount) + " BR: " + birthRate);
#endif
        date += 1;
      }
      else if (popCountID == 1)
      {
#ifdef DEBUG
        Serial.println(String("P1: ") + int32_t(popcount) + " (" + int32_t(pop0) + ", " + int32_t(pop1) + ")");
#endif
        m_PopulationNext = popcount;
        return birthRate;
      }
      popCountID++;
    }
  }

  m_PopulationNext = 10890000000ULL;
  return birthRate;
#endif
}

//----------------------------------------------------------------------------

void  PopulationController::SetYear(int32_t year)
{
  float birthRate = _GetPopulationAtDate(year);
#ifdef DEBUG
  Serial.println(int32_t(m_Population));
#endif
  int64_t yearlyDelta = m_PopulationNext - m_Population;
  int64_t yearlyMS = 365LL * 24LL * 3600LL * 1000LL;

  int64_t yearlyDeltaBirth = m_Population * birthRate;
  int64_t yearlyDeltaDeath = yearlyDeltaBirth - yearlyDelta;

#ifdef DEBUG
  Serial.print("birth rate: ");
  Serial.println(birthRate * 1000.0f);
  Serial.print(year);
  Serial.print(": ");
  Serial.print(int32_t(yearlyDelta));
  Serial.print(" [");
  Serial.print(int32_t(m_Population));
  Serial.print(" , ");
  Serial.print(int32_t(m_PopulationNext));
  Serial.print("] | ");
  Serial.print(float(yearlyDeltaBirth));
  Serial.print(" | ");
  Serial.print(float(yearlyDeltaDeath));
#endif  // DEBUG

  if (yearlyDeltaDeath < 0) // what. no deaths this year? Did I mess something up ?
  {
//    Serial.println(" No deaths ?! : ");
    yearlyDeltaBirth -= yearlyDeltaDeath;
    yearlyDeltaDeath = 0;
  }

  m_NextBirthDelay = (yearlyDeltaBirth > 0) ? (yearlyMS / yearlyDeltaBirth) : 0;
  m_NextDeathDelay = (yearlyDeltaDeath > 0) ? (yearlyMS / yearlyDeltaDeath) : 0;

  // Slightly stagger birth & death indicators to prevent them from flashing at the same time in ancient times
  m_NextBirthCounter = m_NextBirthDelay / 3;
  m_NextDeathCounter = m_NextDeathDelay / 2;

  if (m_NextBirthDelay != 0)
    m_BirthLEDIntensity = 1023;
  if (m_NextDeathDelay != 0)
    m_DeathLEDIntensity = 1023;

  m_BirthOffset = 0;
  m_DeathOffset = 0;

#ifdef DEBUG
  Serial.print(" | ");
  Serial.print(m_NextBirthDelay / 1000.0f);
  Serial.print(" | ");
  Serial.println(m_NextDeathDelay / 1000.0f);
#endif  // DEBUG
}

//----------------------------------------------------------------------------

bool  PopulationController::Update(int dtMS)
{
  bool  hasBirth = false;
  bool  hasDeath = false;

  int kLEDFadeSpeed = 6;
  m_BirthLEDIntensity = max(m_BirthLEDIntensity - dtMS * kLEDFadeSpeed, 0);
  m_DeathLEDIntensity = max(m_DeathLEDIntensity - dtMS * kLEDFadeSpeed, 0);
  
  if (m_NextBirthDelay != 0)
  {
    m_NextBirthCounter -= dtMS;
    while (m_NextBirthCounter < 0)
    {
      m_NextBirthCounter += m_NextBirthDelay;
      m_BirthOffset++;
      hasBirth = true;
      m_BirthLEDIntensity = 1023;
    }
  }

  if (m_NextDeathDelay != 0)
  {
    m_NextDeathCounter -= dtMS;
    while (m_NextDeathCounter < 0)
    {
      m_NextDeathCounter += m_NextDeathDelay;
      m_DeathOffset++;
      hasBirth = true;
      m_DeathLEDIntensity = 1023;
    }
  }

  analogWrite(PIN_OUT_LED_BIRTH, m_BirthLEDIntensity / 4);  // [0, 1023] -> [0, 255]
  analogWrite(PIN_OUT_LED_DEATH, m_DeathLEDIntensity / 4);

  return hasBirth | hasDeath;
}

//----------------------------------------------------------------------------

void  _PopulationDisplay(LedControl &segDisp, int dispOffset, int64_t value)
{
  // Perfs:
  // Displaying all 12 digits @ 888888888888:
  //  1x int64_t: 1.58ms
  //  2x int32_t: 0.77ms
  //  3x int16_t: 0.48ms <--- best
  
  int kMaxDigits = 12;
  int digitID = 0;
  int dot = -1;

  int nextDIDTarget = 4;
  while (value != 0)
  {
    int16_t valuePart = value % 10000;
    value /= 10000;

    while (valuePart != 0 && digitID < kMaxDigits)
    {
      byte  digit = valuePart % 10;
      valuePart /= 10;
      int   did = digitID++;
  
      bool  hasDot = (dot == 2);
      dot++;
      if (hasDot)
        dot = 0;
  
      segDisp.setRawDigit((dispOffset + (did / 8)) * 8 + did % 8, digit, hasDot);
    }
    if (value != 0)
    {
      while (digitID < nextDIDTarget)
      {
        int   did = digitID++;
        bool  hasDot = (dot == 2);
        dot++;
        if (hasDot)
          dot = 0;
        segDisp.setRawDigit((dispOffset + (did / 8)) * 8 + did % 8, 0, hasDot);
      }
    }
    nextDIDTarget += 4;
  }

  if (digitID == 0)
    segDisp.setRawDigit(dispOffset * 8 + digitID++, 0, false);

  for (int i = digitID; i < kMaxDigits; i++)
    segDisp.setRawChar((dispOffset + (i / 8)) * 8 + (i % 8), ' ', false);
}

//----------------------------------------------------------------------------

void  PopulationController::Print(LedControl &segDisp, int offset)
{
  int64_t currentPop = m_Population + (m_BirthOffset - m_DeathOffset);
  _PopulationDisplay(segDisp, offset, currentPop);
}

//----------------------------------------------------------------------------
