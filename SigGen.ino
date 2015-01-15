#include <si5351.h>
#include <FastIO.h>
#include <Rotary.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// LCD/I2C parameter variables
const byte i2cAddr = 0x27;
const byte backlightPin = 3;
const byte enPin = 2;
const byte rwPin = 1;
const byte rsPin = 0;
const byte dsPin = 4;
const byte d5Pin = 5;
const byte d6Pin = 6;
const byte d7Pin = 7;

// Rotary Encoder parameter variables
const byte encPinA = 0;  // pin D1
const byte encPinB = 1; // pin D0
const byte encPinSw = 4;  // pin D4

// Variables - cleanup?
volatile int turnCount = 0;
int dfindex = 4;
int pos = 19 - dfindex;
const long deltaF[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 }; // scaling factors for freq adjustment
const String tuneRate[] = { "1 Hz   ", "10 Hz  ", "100 Hz ", "1 kHz  ", "10 kHz ", "100 kHz", "1 MHz  ", "10 MHz " };
unsigned long freq = 7190000; // TODO: why is this double and not long?
unsigned long stop = 73000000;
boolean encSwitchState = true;
boolean encLastSwitchState = false;

// Create the LCD/I2C object
LiquidCrystal_I2C lcd(i2cAddr, enPin, rwPin, rsPin, dsPin, d5Pin, d6Pin, d7Pin);
// Create the Rotary Encoder object
Rotary encoder = Rotary(encPinA, encPinB);
// Create the Si5351 object
Si5351 si5351;

// ISR for Rotary Encoder pins...
void readEncoder() {
	unsigned char result = encoder.process();
	if (result == DIR_NONE) {
		// do nothing
		;
	}
	else if (result == DIR_CW) {
		turnCount++;
	}
	else if (result == DIR_CCW) {
		turnCount--;
	}
}

void setup()
{
	// create the ISR for changes on the Rotary Encoder
	attachInterrupt(INT2, readEncoder, CHANGE);
	attachInterrupt(INT3, readEncoder, CHANGE);

	// set up LCD and blank display
	lcd.begin(20, 4);
	lcd.setBacklightPin(backlightPin, POSITIVE);
	lcd.setBacklight(HIGH);
	lcd.home();

	// set up I/O pins (NOTE: rotary encoder pins are handled in the Rotary library)
	pinMode(encPinSw, INPUT_PULLUP);

	// setup the si5351 generator
	si5351.init(SI5351_CRYSTAL_LOAD_10PF);
	si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
	si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_2MA);
	si5351.clock_enable(SI5351_CLK1, 0);  // disable unused outputs
	si5351.clock_enable(SI5351_CLK2, 0);  // disable unused outputs
	si5351.set_freq(freq, SI5351_PLL_FIXED, SI5351_CLK0);

	// test display setup  
	lcd.print("SigGen v0.5 CLK0");
	lcd.setCursor(0, 1);        // go to the 2nd line  
	lcd.print("Drive: 4 mA");
	lcd.setCursor(0, 2);        // go to the third line  
	lcd.print("Tune Rate : ");; // start data at (10,2)
	lcd.print(tuneRate[dfindex]);
	lcd.setCursor(0, 3); // got to 4th line
	changeFreq();
	lcd.setCursor(pos, 3);
	lcd.cursor();
}

void loop()
{
	/*
	TODO: Fix debouncing a la http://arduino.cc/en/Tutorial/Debounce
	or maybe fix in hardware with some capacitors.
	*/

	// handle rotary encoder axial switch in the main code.
	encSwitchState = digitalRead(encPinSw);
	if (encSwitchState == LOW && encLastSwitchState == false)
	{
		// decrement the frequency change index for each click
		dfindex--;

		// wrap around from end to beginning
		if (dfindex < 0)
			dfindex = 7;

		// cursor position and set the debounce variable.
		pos = 19 - dfindex;
		encLastSwitchState = true;

		// change tune rate display accordingly
		lcd.setCursor(12, 2);
		lcd.print(tuneRate[dfindex]);
		lcd.setCursor(pos, 3);
	}

	if (encSwitchState == HIGH && encLastSwitchState == true)
	{
		// unset the debounce variable
		encLastSwitchState = false;
	} // end of encoder switch handling

	// update Rotary Encoder position (pos. #s are right turnCount, neg. #s are left turnCount.
	if (turnCount != 0)
	{
		freq = freq + (turnCount * deltaF[dfindex]);
		changeFreq();
		turnCount = 0; //reset the counter
	}

	// band sweeper code
	//while (freq < stop)
	//{
	//	freq = freq + (turnCount * deltaF[dfindex]);
	//	changeFreq();
	//	turnCount++;
	//	delay(500);
	//}
}

void changeFreq()
{
	// change the output first
	si5351.set_freq(freq, SI5351_PLL_FIXED, SI5351_CLK0);
	String freqStr = String(freq, DEC);

	// blank the line
	lcd.setCursor(12, 3);
	lcd.print("       ");
	lcd.setCursor(0, 3);
	lcd.print("Freq: ");

	// handle the various sizes of "freq"
	if (freq >= 10000000){
		lcd.setCursor(12, 3);
		lcd.print(freq);
	}
	else {
		if (freq < 1000000){
			lcd.setCursor(14, 3);
			lcd.print(freq);
		}
		else {
			lcd.setCursor(13, 3);
			lcd.print(freq);
		}
	}

	// establish the cursor position
	pos = 19 - dfindex;
	lcd.setCursor(pos, 3);
}