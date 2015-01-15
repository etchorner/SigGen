#Description:
SigGen is a very basic implementation of a one channel signal generator, which I needed 
to come up with in order to finish building a 40 meter direct conversion receiver. The code in this repository is for the 
Arduino Micro which makes up the brains of this little test gear project.

This project uses an Arduino microcontroller to manage an Si5351 Clock Generator which is capable of generating RF signals 
from 8 kHz to about 160 Mhz. The output is a square wave 3Vpp signal with selectable drive levels from 2 mA to 8 mA. A 
four row by twenty column LCD is part of the user interface. Both the LCD and the Si5351 modules are controlled and 
monitored through the I2C bus presented by the Arduino controller. The final piece of the system, as of 1/14/2015, is a rotary encoder with push-button which is used to operate the signal generator system.

In large part, this project would be a dead ender were it not for the work of Jason Milldrun, [NT7S](http://nt7s.com/), from whose Si5351Arduino [library](https://github.com/etherkit/Si5351Arduino) my accomplishment springs. This library stands well above anything else I've seen in terms of ease of use and reliability. 

##Operation
If wired according to the schematic (to be attached later), one can change the tuning rate, or tuning steps, by pushing the encoder axial push-button. The rate will decrement from the most significant digit (10 MHz) to the least (1 Hz), at which point the rate will 'wrap-around to the most significant digit again.

To tune or alter the output frequency, rotate the encoder. The frequency will increase as the encoder is rotated clockwise and decrease when rotated counter-clockwise. The rate of frequency change is a result of the tuning rate you select; if the rate is set to '1 Hz', each detent in the encoder will raise or lower the output by 1 Hz.

The output can be taken via coaxial or other shielded cable or can be connected to an antenna. In the prototype system, it has been used to troubleshoot and align a direct conversion receiver, successfully, with a twelve inch length of insulated wire acting as an antenna.

##CHANGELOG
**1/14/2015, v0.1, Initial Operating Capability (IOC)**
* One (of three) output channels active, set to 2 mA drive, all others off).
* User can select tuning rates, in discrete digit steps from 1 Hz to 10 Mhz, using rotary encoder pushbutton.
* User can change output frequency by rotating rotary encoder; CW rotation for increment, CCW for decrement.
* Frequency changes are in the tuning rate digit selected by the user and accumulate properly.

##TODO:
- [ ] Add schematics/drawings
- [ ] Fix tuning rate/step skipping (Issue SigGen#1) 
- [ ] Add ability to control drive level
- [ ] Add EEPROM or other NVRAM method of storing last sets (freq, tune rate, and drive) at power off
- [ ] {HARDWARE}Implement step attenuator on output to give precise dbm level outpus. May involve code to control switching mechanism (relay, pin diodes, other?).
- [ ] Change UI frequency readout to add thousands and millions seperator commas.

