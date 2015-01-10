#include <si5351.h>
#include <FastIO.h>
#include <Rotary.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// LCD/I2C Macros
#define I2C_ADDR 0x27
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

// Rotary Encoder Macros
#define ENC_PIN_A 0  // pin D1
#define ENC_PIN_B 1  // pin D0
#define ENC_PIN_SW 4  // pin D4

// Variables - cleanup?
volatile int turnCount = 0;
int dfindex = 3;
int pos = 19 - dfindex;
const long deltaF[] = { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 }; // scaling factors for freq adjustment
const String tuneRate[] = { "1 Hz   ", "10 Hz  ", "100 Hz ", "1 kHz  ", "10 kHz ", "100 kHz", "1 MHz  ", "10 MHz " };
unsigned long freq = 5700000; // TODO: why is this double and not long?
boolean encSwitchState = true;
boolean encLastSwitchState = false;

// Create the LCD/I2C object
LiquidCrystal_I2C lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
// Create the Rotary Encoder object
Rotary encoder = Rotary(ENC_PIN_A, ENC_PIN_B);
// Create the Si5351 object
Si5351 si5351;

// ISR for Rotary Encoder pins...here's where menu/frequency code will go.
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
	lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
	lcd.setBacklight(HIGH);
	lcd.home();

	// set up I/O pins (NOTE: rotary encoder pins are handled in the library)
	pinMode(ENC_PIN_SW, INPUT_PULLUP);

	// setup the si5351 generator
	si5351.init(SI5351_CRYSTAL_LOAD_8PF);
	si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
	si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_4MA);
	si5351.clock_enable(SI5351_CLK1, 0);  // disable unused outputs
	si5351.clock_enable(SI5351_CLK2, 0);  // disable unused outputs
	si5351.set_freq(freq, SI5351_PLL_FIXED, SI5351_CLK0);

	// test display setup  
	lcd.print("Signal Generator");
	lcd.setCursor(0, 1);        // go to the 2nd line  
	lcd.print("NT7S Si5351 lib");
	lcd.setCursor(0, 2);        // go to the third line  
	lcd.print("Tune Rate : ");; // start data at (10,2)
	lcd.print(tuneRate[dfindex]);
	lcd.setCursor(0, 3); // got to 4th line
	lcd.print("Freq: ");
	changeFreq();
	lcd.setCursor(pos, 3);
	lcd.cursor();

	Serial.begin(9600);
}

void loop()
{
	/*
	TODO: Fix debouncing a la http://arduino.cc/en/Tutorial/Debounce
	or maybe fix in hardware with some capacitors.
	*/

	// handle rotary encoder axial switch in the main code.
	encSwitchState = digitalRead(ENC_PIN_SW);
	if (encSwitchState == LOW && encLastSwitchState == false)
	{
		// increment the frequency change index for each click
		dfindex++;

		// wrap around from end to beginning
		if (dfindex > 7)
			dfindex = 0;

		// cursor position and set the debounce variable.
		pos = 19 - dfindex;
		encLastSwitchState = true;

		// change display accordingly
		lcd.setCursor(0, 2);
		lcd.print("Tune Rate : ");
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
		turnCount = 0;
	}


	// band sweeper code
	//while (freq	<stop)
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
