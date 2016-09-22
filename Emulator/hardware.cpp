// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		hardware.cpp
//		Purpose:	Hardware Interface
//		Created:	21st September 2016
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include <stdlib.h>
#include "sys_processor.h"
#include "hardware.h"

#ifdef WINDOWS
#include <stdio.h>
#include "gfx.h"																// Want the keyboard access.
#endif

static BYTE8 rows,columns;														// Screen size.
static WORD16 renderingAddress; 												// Screen rendering address
static BYTE8 isScreenOn;														// Non zero if screen on

// *******************************************************************************************************************************
//													Hardware Reset
// *******************************************************************************************************************************

void HWIReset(void) {
	columns = 64;rows = 32;														// Standard screen size
	isScreenOn = 0; 															// Screen off
	renderingAddress = 0x0000; 
	GFXSetFrequency(0);
}

// *******************************************************************************************************************************
//											Process keys passed from debugger
// *******************************************************************************************************************************

#ifdef WINDOWS
BYTE8 HWIProcessKey(BYTE8 key,BYTE8 isRunMode) {
	if (isRunMode) { }
	return key;
}
#endif

// *******************************************************************************************************************************
//												Called at End of Frame
// *******************************************************************************************************************************

WORD16 HWIEndFrame(WORD16 r0,LONG32 clock) {
	renderingAddress = r0; 														// the rendering address is what R0 was set to last time.
	return r0;																	// Return what R0 should be on entering interrupt.
}

// *******************************************************************************************************************************
//						Write to device selected via port 1, control command via port 2
// *******************************************************************************************************************************

void HWIWriteDevice(BYTE8 device,BYTE8 controlValue) {
	//printf("DC:%x %x\n",device,controlValue);

	switch(device) {
		case 2:																	// Device 2 = TV.
			isScreenOn = (controlValue & 3) != 0; 								// 00 off 11 on in 2 lower bits.
			columns = (controlValue & 2) ? 64 : 32;								// Bit 1 is 64 across
			rows = (controlValue & 1) ? 32 : 16; 								// Bit 0 is 32 down.
			break;
	}
}

// *******************************************************************************************************************************
//													Check if screen on
// *******************************************************************************************************************************

BYTE8 HWIIsScreenOn(void) {
	return isScreenOn;
}

// *******************************************************************************************************************************
//								Access screen dimensions (64 or 32 pixels, 32 or 16 rows)
// *******************************************************************************************************************************

BYTE8 HWIScreenWidth(void) { 
	return columns;
}

BYTE8 HWIScreenHeight(void) {
	return rows;
}
// *******************************************************************************************************************************
//													Set the rendering address
// *******************************************************************************************************************************

WORD16 HWIGetDisplayAddress(void) {
	return renderingAddress;
}
