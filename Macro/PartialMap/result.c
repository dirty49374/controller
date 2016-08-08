/* Copyright (C) 2014-2016 by Jacob Alexander
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

// ----- Includes -----

// Compiler Includes
#include <Lib/MacroLib.h>

// Project Includes
#include <led.h>
#include <print.h>

// Local Includes
#include "result.h"
#include "kll.h"


// ----- Enums -----

typedef enum ResultMacroEval {
	ResultMacroEval_DoNothing,
	ResultMacroEval_Remove,
} ResultMacroEval;




// ----- KLL Generated Variables -----

extern const Capability CapabilitiesList[];

extern const ResultMacro ResultMacroList[];
extern ResultMacroRecord ResultMacroRecordList[];



// ----- Variables -----

// Pending Result Macro Index List
//  * Any result macro that needs processing from a previous macro processing loop

index_uint_t macroResultMacroPendingList[ ResultMacroNum ] = { 0 };
index_uint_t macroResultMacroPendingListSize = 0;


// --- recording control start
#define MAX_RECORD 			5
#define MAX_RECORD_BUFFER_SZ 		500
#define RECORD_BASE 			10000
#define RECORDINGCONTROL_TOGGLE		0
#define RECORDINGCONTROL_PLAY		1
#define RECORDINGCONTROL_PRESS		2
#define RECORDINGCONTROL_RELEASE	3

uint16_t CurrentRecordingSlot;
uint16_t CurrentRecordingLength;

ResultMacro 	  RecodableMacroList[MAX_RECORD];
ResultMacroRecord RecordableMacroRecordList[MAX_RECORD];
uint8_t 	  RecordableGuideBuffer[MAX_RECORD][MAX_RECORD_BUFFER_SZ];

uint8_t		  RecordingUsbCodeSendCapabilityIndex;
uint8_t		  RecordingControlCapabilityIndex;
uint8_t		  RecordingStopped;

extern void Output_usbCodeSend_capability( uint8_t state, uint8_t stateType, uint8_t *args );
extern void Output_recordingControl_capability( uint8_t state, uint8_t stateType, uint8_t *args );
extern void Output_recordingUsbCodeSend_capability( uint8_t state, uint8_t stateType, uint8_t *args );

// -- recording control end

// ----- Functions -----

// Evaluate/Update ResultMacro
inline ResultMacroEval Macro_evalResultMacro( uint16_t resultMacroIndex )
{
	// Lookup ResultMacro
	const ResultMacro* macro;
	ResultMacroRecord* record;

	if (resultMacroIndex < 10000)
	{
		macro  = &ResultMacroList[ resultMacroIndex ];
		record = &ResultMacroRecordList[ resultMacroIndex ];
	}
	else
	{
		macro  = &RecodableMacroList[ resultMacroIndex - RECORD_BASE ];
		record = &RecordableMacroRecordList[ resultMacroIndex - RECORD_BASE ];
	}

	// Current Macro position
	var_uint_t pos = record->pos;

	// Length of combo being processed
	uint8_t comboLength = macro->guide[ pos ];

	// Function Counter, used to keep track of the combo items processed
	var_uint_t funcCount = 0;

	// Combo Item Position within the guide
	var_uint_t comboItem = pos + 1;

	// Iterate through the Result Combo
	while ( funcCount < comboLength )
	{
		// Assign TriggerGuide element (key type, state and scancode)
		ResultGuide *guide = (ResultGuide*)(&macro->guide[ comboItem ]);

		// Do lookup on capability function
		void (*capability)(uint8_t, uint8_t, uint8_t*) = (void(*)(uint8_t, uint8_t, uint8_t*))(CapabilitiesList[ guide->index ].func);

		// Call capability
		capability( record->state, record->stateType, &guide->args );

		// Increment counters
		funcCount++;
		comboItem += ResultGuideSize( (ResultGuide*)(&macro->guide[ comboItem ]) );
	}

	// Move to next item in the sequence
	record->pos = comboItem;

	// If the ResultMacro is finished, remove
	if ( macro->guide[ comboItem ] == 0 )
	{
		record->pos = 0;
		return ResultMacroEval_Remove;
	}

	// Otherwise leave the macro in the list
	return ResultMacroEval_DoNothing;
}


void Result_add( uint32_t index )
{
}


void Result_setup()
{
	// Initialize ResultMacro states
	for ( var_uint_t macro = 0; macro < ResultMacroNum; macro++ )
	{
		ResultMacroRecordList[ macro ].pos       = 0;
		ResultMacroRecordList[ macro ].state     = 0;
		ResultMacroRecordList[ macro ].stateType = 0;
	}

	// Initialize ResultMacro states
	for ( var_uint_t macro = 0; macro < MAX_RECORD; macro++ )
	{
		RecordableGuideBuffer[ macro ][0] = '\0';
		RecodableMacroList[ macro ].guide = &RecordableGuideBuffer[ macro ][0];

		RecordableMacroRecordList[ macro ].pos       = 0;
		RecordableMacroRecordList[ macro ].state     = 0;
		RecordableMacroRecordList[ macro ].stateType = 0;
	}

	CurrentRecordingSlot = 0xFF;

	for (int i=0; i<CapabilitiesNum; ++i)
	{
		if (CapabilitiesList[i].func == Output_recordingControl_capability)
			RecordingControlCapabilityIndex = i;
		else if (CapabilitiesList[i].func == Output_recordingUsbCodeSend_capability)
			RecordingUsbCodeSendCapabilityIndex = i;
		
	}
}


void Result_process()
{
	// Tail pointer for macroResultMacroPendingList
	// Macros must be explicitly re-added
	var_uint_t macroResultMacroPendingListTail = 0;

	// Iterate through the pending ResultMacros, processing each of them
	for ( var_uint_t macro = 0; macro < macroResultMacroPendingListSize; macro++ )
	{
		switch ( Macro_evalResultMacro( macroResultMacroPendingList[ macro ] ) )
		{
		// Re-add macros to pending list
		case ResultMacroEval_DoNothing:
		default:
			macroResultMacroPendingList[ macroResultMacroPendingListTail++ ] = macroResultMacroPendingList[ macro ];
			break;

		// Remove Macro from Pending List, nothing to do, removing by default
		case ResultMacroEval_Remove:
			break;
		}
	}

	// Update the macroResultMacroPendingListSize with the tail pointer
	macroResultMacroPendingListSize = macroResultMacroPendingListTail;
}


void Output_recordingControl_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_recordingControl_capability(cmd,slot)");
		return;
	}

	uint8_t type = args[0];
	uint8_t data = args[1];

	switch (type)
	{
		case RECORDINGCONTROL_TOGGLE:
			if (data >= MAX_RECORD)
				return;

			if (CurrentRecordingSlot == data)
			{
				// deactivate on press
				if ( stateType != 0x00 || state != 0x01 )
					return;

				// stop 
				print("recording stopped : ");
				printHex(CurrentRecordingSlot);
				print(NL);

				CurrentRecordingSlot = 0xFF;

				// prevent restart recording of current key release
				RecordingStopped = 1;
			}
			else
			{
				// activate on release
				if ( stateType != 0x00 || state != 0x03 )
					return;

				if (RecordingStopped)
				{
					RecordingStopped = 0;
					return;
				}

				// start new
				CurrentRecordingSlot = data;
				CurrentRecordingLength = 0;
				RecordableGuideBuffer[ CurrentRecordingSlot ][ 0 ] = '\0';

				print("recording started : ");
				printHex(CurrentRecordingSlot);
				print(NL);
			}
			return;


		case RECORDINGCONTROL_PLAY:
			// activate on press
			if ( stateType != 0x00 || state != 0x01 )
				return;

			// stop current recording if exists
			if (CurrentRecordingSlot != 0xFF)
			{
				CurrentRecordingSlot = 0xFF;
			}

			// replay recording
			macroResultMacroPendingList[ macroResultMacroPendingListSize++ ] = RECORD_BASE + data;
			RecordableMacroRecordList[ data ].state = state;
			RecordableMacroRecordList[ data ].stateType = stateType;
			return;

		case RECORDINGCONTROL_PRESS:
			Output_usbCodeSend_capability(0x01, 0, &data);
			return;

		case RECORDINGCONTROL_RELEASE:
			Output_usbCodeSend_capability(0x03, 0, &data);
			return;
	}
}

void Output_recordingUsbCodeSend_capability( uint8_t state, uint8_t stateType, uint8_t *args )
{
	// Display capability name
	if ( stateType == 0xFF && state == 0xFF )
	{
		print("Output_recordingUsbCodeSend_capability(usbCode)");
		return;
	}

	uint8_t key = args[0];

	if (CurrentRecordingSlot != 0xFF)
	{
		if ( stateType == 0x00 && state != 0x02 ) // press & releasestate
		{
			uint8_t* buffer = &RecordableGuideBuffer[ CurrentRecordingSlot ][ 0 ];
			if (CurrentRecordingLength + 5 >= MAX_RECORD_BUFFER_SZ)
			{
				CurrentRecordingSlot = 0xFF;
			}
			else
			{
				uint8_t pressType = state == 0x01 ? RECORDINGCONTROL_PRESS : RECORDINGCONTROL_RELEASE;

				buffer[ CurrentRecordingLength++ ] = 1;			// one combo
				buffer[ CurrentRecordingLength++ ] = RecordingControlCapabilityIndex;
				buffer[ CurrentRecordingLength++ ] = pressType;
				buffer[ CurrentRecordingLength++ ] = key;
				buffer[ CurrentRecordingLength ] = '\0';

				// printHex(stateType); print(", "); printHex(state); print(", "); printHex(key);
				// print("/   ");
			}
		}
	}
	Output_usbCodeSend_capability( state, stateType, &key );
}
