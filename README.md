# Wio_Terminal_Rfm69_Receiver


Wio Terminal receives and displays sensor data transmitted via Adafruit Rfm69HCW radio.
Uses LowPowerLab Rfm69 library.

Electrical connection: See folder pictures.

Sensor data are sent by two different nodes. One node sends three temperature values from a solar plant (collector-, storage- and watertemperature).
The other node sends values read from a smartmeter (Current, Power and Work) as well as on/off states of the solar plant pump).
The values are transmitted in a very special non standard data format.
The actual values are dislayed as numbers on the Wio Terminal display.

Pressing the 5-way button of the Wio Terminal for more than 2 sec toogles the display between power and temperature values.
Backlight reduction can be reverted by noise (e.g. hand clapping)
There is a more elaborated version of this App with "Uncanny eyes" animation when waking up
https://github.com/RoSchmi/WioTerminalRepos/tree/master/Proj/Wio_Terminal_Rfm69_Receiver_Eyes

