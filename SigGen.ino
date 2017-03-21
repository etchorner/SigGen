/**

  SigGen.ino - RF Signal Generator Control System

  Copyright (C) 2015 Christopher M. Horner, chris.horner@gmail.com
  p
  This program is free software : you can redistribute it and / or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.If not, see <http://www.gnu.org/licenses/>.

 **/
#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>
#include <Bounce2.h>
#include <Wire.h>
#include <si5351.h>
#include <LiquidCrystal_I2C.h>

// LCD/I2C parameter variables
#define i2cAddr 0x27
// Rotary Encoder parameter variables
#define encPinA 2  // Nano pin D2
#define encPinB 3 // Nano pin D3
#define encPinSw 4  // Nano pin D4
// Other Variables - cleanup?
int dfindex = 4;
int pos = 19 - dfindex;
const long deltaF[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 }; // scaling factors for freq adjustment
const String tuneRate[] = { "1 Hz   ", "10 Hz  ", "100 Hz ", "1 kHz  ",
                            "10 kHz ", "100 kHz", "1 MHz  ", "10 MHz "
                          };
unsigned long freq = 7190000; // TODO: why is this double and not long?
unsigned long stop = 73000000;

// Create the objects
LiquidCrystal_I2C lcd(i2cAddr, 20, 4);
Encoder encoder = Encoder(encPinA, encPinB);
Si5351 si5351;
Bounce debouncer = Bounce();

void setup() {
  // set up LCD and blank display
  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.home();

  // set up I/O pins (NOTE: rotary encoder pins are handled in the Encoder library)
  pinMode(encPinSw, INPUT_PULLUP);
  debouncer.attach(encPinSw);
  debouncer.interval(5);

  // setup the si5351 generator
  si5351.init(SI5351_CRYSTAL_LOAD_10PF, 0, 0); // 10pf for the Adafruit si5351 modules, uses ABM8G-25.000MHZ-B4Y-T or ABM8G-25.000MHZ-4Y-T3
  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.set_freq(freq, SI5351_CLK0);

  // generate/terminate signals
  si5351.output_enable(SI5351_CLK0, HIGH);  // turn on desired output
  si5351.set_clock_pwr(SI5351_CLK1, LOW);  // disable unused outputs
  si5351.set_clock_pwr(SI5351_CLK2, LOW);  // disable unused outputs

  // test display setup
  lcd.print("SigGen v1.0 CLK0");
  lcd.setCursor(0, 1);       // go to the 2nd line
  lcd.print("Drive: 8 mA");
  lcd.setCursor(0, 2);        // go to the third line
  lcd.print("Tune Rate : ");
  ; // start data at (10,2)
  lcd.print(tuneRate[dfindex]);
  lcd.setCursor(0, 3); // go to 4th line
  changeFreq();
  lcd.setCursor(pos, 3);
  lcd.cursor();
}

void loop() {
  // handle rotary encoder axial switch in the main code loop. 
  if (debouncer.update()) { // don't bother if nothing has changed...
    if (debouncer.fell()) {  // don't bother if it hasn't gone LOW....
      // decrement the frequency change index for each click
      dfindex--;

      // wrap around from end to beginning
      if (dfindex < 0)
        dfindex = 7;

      // cursor position and set the debounce variable.
      pos = 19 - dfindex;

      // change tune rate display accordingly
      lcd.setCursor(12, 2);
      lcd.print(tuneRate[dfindex]);
      lcd.setCursor(pos, 3);
    }
    
    // change freq according to the Encoder accumulation, if any
    freq = freq + (encoder.read() * deltaF[dfindex]);
    changeFreq();
    encoder.write(0);
  }
}

void changeFreq() {
  // change the output first
  si5351.set_freq(freq, SI5351_CLK0);
  String freqStr = String(freq, DEC);

  // blank the line
  lcd.setCursor(12, 3);
  lcd.print("       ");
  lcd.setCursor(0, 3);
  lcd.print("Freq: ");

  // handle the various sizes of "freq"
  if (freq >= 10000000) {
    lcd.setCursor(12, 3);
    lcd.print(freq);
  } else {
    if (freq < 1000000) {
      lcd.setCursor(14, 3);
      lcd.print(freq);
    } else {
      lcd.setCursor(13, 3);
      lcd.print(freq);
    }
  }

  // establish the cursor position
  pos = 19 - dfindex;
  lcd.setCursor(pos, 3);

}
