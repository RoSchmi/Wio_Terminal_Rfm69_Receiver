# Wio_Terminal_Rfm69_Receiver

This is work in progress.

Wio Terminal receives sensor data transmitted via Adafruit Rfm69HCW radio.
Uses LowPowerLab Rfm69 library.

Sensor data are sent by two different nodes. One node sends three temperature values from a solar plant (Collector-, Storage- and Water temperature).
The other node sends values read from a smartmeter (Current, Power and Work) as well as on/off states of the solar plant pump).
The values are transmitted in a very special non standard data format.


