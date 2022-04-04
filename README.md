# World Heartbeat counter

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

