# CPE-301-Final-Project

Project Notes:

Eylon Borenstein, Kyle Hedin contributed to this

states: 
Green LED: D53- water level is at threshold (not running but on)
Red LED: D51-error/ above threshold
Blue Led: D49-running/ water is below threshold
yellow LED: D47-fan is turned off


A0 is water sensor
A1 is humidity/temp sensor
52-22 is LCD board (possibly change later)
23 is step motor blue
29 is step motor orange
other two in between 
fan control is D5, will use a transistor
button to D6 is step motor left
button to D7 is step motor right
button to D8 is fan stop


The Real time clock is a physical thing. It shall have SDA on A4 and SCL on A5.


Will need L293D H bridge


D23 L293D 1A (1Y = pink step)
D25 2A (2Y = Orange)
D27 3A (3Y = yellow)
D29 4A (4Y = Blue)

D21 (SCL) = External interupt request (exit disable)

Libraries:
LiquidCrystal by Adafruit and Arduino
DHT sensor library by Adafruit
RTClib by Adafruit
Stepper by Arduinio
