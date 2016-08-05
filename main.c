/* Copyright (C) 2011-2014 by Jacob Alexander
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/MainLib.h>

// Project Includes
#include <macro.h>
#include <scan_loop.h>
#include <output_com.h>

#include <cli.h>
#include <led.h>
#include <print.h>


#include <Lib/delay.h>
#define MAX_CNT 100
uint32_t tick_times[MAX_CNT] = {};
uint32_t tick_total;
uint8_t tick_cur;

uint32_t report_msec;
void report(uint32_t tick_us)
{
	tick_total -= tick_times[tick_cur];
	tick_total += tick_us;

	tick_times[tick_cur++] = tick_us;

	if (tick_cur == MAX_CNT)
	{
		tick_cur = 0;
	}
	uint32_t m = millis();
	if (report_msec < m)
	{
		uint32_t min = -1, max = 0;
		for (uint8_t i=0; i<MAX_CNT; ++i)
		{
			if (tick_times[i] < min)
				min = tick_times[i];
			if (tick_times[i] > max)
				max = tick_times[i];
		}
		print("frame report: cnt=");
		printInt32(MAX_CNT);
		print(" min=");
		printInt32(min);
		print("us max=");
		printInt32(max);
		print("us avg=");
		printInt32(tick_total / MAX_CNT); 
		print(NL);

		report_msec += 5000;
	}
}
// ----- Functions -----

int main()
{
	// AVR - Teensy Set Clock speed to 16 MHz
#if defined(_at90usb162_) || defined(_atmega32u4_) || defined(_at90usb646_) || defined(_at90usb1286_)
	CLKPR = 0x80;
	CLKPR = 0x00;
#endif

	// Enable CLI
	CLI_init();

	// Setup Modules
	Output_setup();
	Macro_setup();
	Scan_setup();

	// Main Detection Loop
	while ( 1 )
	{
		uint32_t start = micros();

		// Process CLI
		CLI_process();

		// Acquire Key Indices
		// Loop continuously until scan_loop returns 0
		cli();
		while ( Scan_loop() );
		sei();

		// Run Macros over Key Indices and convert to USB Keys
		Macro_process();

		// Sends USB data only if changed
		Output_send();

		report(micros() - start);
	}
}

