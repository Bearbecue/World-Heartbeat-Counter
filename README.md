# World Heartbeat counter
This is a heartbeat counter that "counts" (estimates) the heartbeats of humanity in realtime based on the population count and a distribution of BPMs that can be controlled independently.
The population count cannot be set directly, instead it is based on the year.
A big control wheel allows to change the current year, which updates the population count to match the population count of that year.
The current year can go from 100 000 BC (birth of the first homo-sapiens :) population count = 1), up to year 2100 AC.
A green LED flashes whenever a new human is born, a red LED flashes whenever a human dies.

![Front panel description](Images/HowTo-en.png)


The hearbteat sync wheel allows to pseudo-synchronize the heartbeats of all humanity, it gives a nicer pulsating feel when watching the device, and turns it into a contemplative "clock" :)
When unsynchronized, the counter increases uniformly over time.

Demo of the machine running & cycling through the 8 recorded states, with heartbeats mostly synchronized:
[![D2 Head sculpt teaser](https://img.youtube.com/vi/0Ew_5UpqQ8A/0.jpg)](https://www.youtube.com/watch?v=0Ew_5UpqQ8A)


# Displays

1: Heartbeats
20 digits, enough to count up to a hundred billion billion heartbeats, equivalent to 200 years of runtime with the current population at an average of 120 bpm, 300 years at an average of 80 bpm, and 400 years at an average of 60 bpm. Should be enough to not worry about maxing out the counter :)

2: Population
How many humans on earth at the selected date.

3: Year
... the year :) goes from -100 000 (which is estimated to be roughly the time at which the first homo sapiens appeared), up to 2100. It could go further but the confidence in the predictions becomes so low that I stopped it at 2100

4: Population BPM distribution
The left of the graph represents 30 BPM, the right of the graph 200 BPM. The spike on the above image is roughly at around60 BPM

5: Births LED (green)
Based on the birth estimates of the selected year. Like with the population, the more you go back in time, the more approximate it becomes. The general tendency seems to be a deceleration.
Each flash = 1 birth on earth. Does not try to take into account annual variations such as seasons, flashes using the average birth frequency of the year.

6: Deaths LED (red)
Based on the birth estimates of the selected year and on the yearly population variation.
Each flash = 1 death on earth. Like for the births LED, it uses the average frequency of the selected year.


# Controls

7: Heartbeat synchronization wheel
When it's turned all the way to the left: No sync, it's the most "realistic" mode, every human has their heartbeats happening at different moments, the counter increases uniformly, constantly.
When it's turned all the way to the right: The entire humanity has synchronized heartbeats. The microcontroller computes the dominant BPM in the BPM distribution curve, and everybody beats at this frequency, with a small random factor that slightly "spreads" the beats around the synchronization peak, and which for sufficiently large populations is basically similar to a gaussian distribution. It's the mode I feel is the most "contemplative", where you can visually see the wave of heartbeats propagating along the digits.
Intermediate wheel positions make the behavior vary smoothly between both extremes.

8: Year adjustment wheel
Allows to change the current year, see "Known issues" down below for more details

9: BPM distribution wheels
Each wheel controls the height of one section of the BPM distribution curve, and to set a custom BPM distribution.

10: Memory cycle switch
The microcontroller records 8 different states, so it's possible to record 8 different dates & counter values.
A short press on the switch displays the next state. Every time the state is changed, it records the previous state (year + counter, but not population, so when you go back on this state, the population resets to its initial value for the year. Not great, but I simply did not think about saving it)
When you reach the last state, and you press again, it wraps around to the first state, etc...
A press of more than 4 seconds resets the current counter to zero. It can't be undone (Unless you unplug the power and the 5 minutes auto-save did not happen yet, and the state wasn't changed by doing a short press on the button again)

The goal to have multiple states was initially to be able to do a few presets at specific years, to see the difference in counting speeds at different times in history, while still keeping one whose goal is just to count as far as possible, if you keep the device plugged in permanently.
However, only the currently displayed counter is advanced, the other 7 counters are paused in the background.


# Known issues

- When you keep population increase, normally after one year it should reach the population of the next year, BUT the year won't auto-increase, you will have to increase it by hand using the year selection wheel. It will update the population computed previously.

- The BPM distribution display in the current only prototype (cf video) has a few bad contacts on some LED columns, I didn't manage to fully fix it :(

- The year selection wheel is a bit crusty. When the year is close to our current date, the wheel goes through the years "slowly", when we arrive towards the middle-ages or BC, it goes faster, otherwise it's unmanageable to manually go to -10 000 or -100 000. I tried to detect in software when the turning rate gets slower, and to change the years more precisely, while fast turns would change the year much more quickly, but sometimes there are misses (I suspect maybe some crosstalk in the nightmare of wires on the inside of the case, in addition to a suspicious construction of this aliexpress wheel, in addition to not good-enough de-bouncing on the wheel pins.
So, when you want to go to a specific year, it's often pretty annoying, you need to go slowly and gently when you get close to it. But sometimes if you go too slow it produces a huge bounce in years... anyway. If there was a major thing to improve it would be this wheel.

# Data & sources

The population count is based on the actual data for the modern era, extrapolations for the future, and rough estimates for the ancient past. It includes some specific events in history such as the black death of 1347-1351. For sure it is missing a lot of historically-known interesting spikes and dips right now.

Sources are based on:
https://ourworldindata.org/grapher/population
https://en.m.wikipedia.org/wiki/World_population

Curve of the actual population data used:
![Population curve](Images/data.png)

* Blue curve: Population, log scale on the left
* Red curve: Birth coefficient, scale on the right

The graph used by the machine goes up to 100000 BC, but the curve above only displays down to 3000BC otherwise it's unreadable and not very interesting.


# Internals

![Internal nightmare wiring](Images/Inner_Nightmare.jpg)

![Internal contents](Images/Inner_Guts.jpg)


# Rough notes & initial design ideas
	7-seg display statistical counter of world heartbeats. Use pots to tune heartbeat distribution. Use pot to vary between synchronized and nonsynchronized.
	Pots control height of bezier curve controlpoints? Or smooth curve that goes through the points.
	Bargraph display of actual curve.
	Aluminum casing with acrylic front plate ?
	Population count & heartbeat count.
	If running for 30 days, should be able to count up to:
	80*60*24*30*7.9bn = 2.73e+16 : 17 digits, needs 3 max7219, 24 digits
		20 digits @ 200 bpm: can count for 120 years
		20 digits @ 120 bpm: can count for 200 years
		80 bpm: 300 years
		60 bpm: 400 years
		u64 can go to 1.8e+19
		Use an u64 and an u32 and do a manual carry ?
	Save the value in arduino ROM so that it's preserved if power is lost ?
	Protected reset button

Main display
	0.8" (20.32mm) displays
	20 digits: 5x arrays of 4 digits, can display up to 9 999 999 999 999 999
	dimension: 36 cm x 2.57 cm
	3 MAX7219 (data rail A)
	RSET: 28k / 30k
	
Population display
	12 digits: 3x arrays of 4 digits, can display up to 999 999 999 999
	dimensions: 17.7 cm x 1.78 cm
	2 MAX7219 (data rail A)
	RSET: 25k / 27k

Date display
	7 digits: 2x array of 3 digits + 1 digit, can display from -999 999 to 9 999 999 (display -10 000 BC to 2100 / 2200)
	dimensions: 8.86 cm x 1.9 cm
	1 MAX7219 (data rail A)
	RSET: 28k / 30k

Curve display
	2x4 array of 8x8 LED dot matrix cells
	dimensions: 8.0 cm x 4.0 cm
	8 MAX7219 (data rail B)
	RSET: 28k

Pots
	consider we have 7 digital inputs available
- Hearbeat distribution curve control:
	- 4 pots ? @ bpm: 60, 90, 120, 150 ?
	- 5 pots ? @ bpm: 50, 70, 90, 120, 160 ?
- Year selection: Use a rotary encoder with a nonlinear mapping allowing to go back in time HARD: -300 000 BC to 2100 ?
	- 1 RotEnc: Main time-machine wheel
	- 1 pot: Additional year fine-tune ?
- Synchronization control
	- 1 pot: Heartbeats synced vs unsynced
- Population manual override ? -> simple on/off switch ?

Misc I/O
- On/off switch
- Power plug
- 3 pushbuttons: save, restore, reset. Wired to a single analog input, just feeding different voltages. Use a triple resistor voltage divider.
	- base state: GND
	- reset: +1.6V
	- restore: +3.3V
	- save: +5V
- Brightness control pot ?



Time machine pot ? Allow scrubbing between -10000 BC (neolithic) and 2100 and sample homo sapiens sapiens population curve. No actually start from the first homo sapiens. See the initial heartbeat of 1 individual.
-> Use large rotary encoder (6cm)
2nd small pot to control precision of the turns..

Scale heart rate across ages: shift minmax of the curve based on rough estimate of avg human size. (Ex: chimps have a resting heart rate of ~95bpm instead of ~75 for modern humans)

Additional pot & switch to control manual population vs auto population ?
Need some analog system bc arduino inputs limited to 1023 steps ?

