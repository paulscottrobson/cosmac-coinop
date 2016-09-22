// *******************************************************************************************************************************
// *******************************************************************************************************************************
//
//		Name:		processor.c
//		Purpose:	Processor Emulation.
//		Created:	21st September 2016
//		Author:		Paul Robson (paul@robsons.org.uk)
//
// *******************************************************************************************************************************
// *******************************************************************************************************************************

#include <stdlib.h>
#ifdef WINDOWS
#include <stdio.h>
#endif
#include "sys_processor.h"
#include "sys_debug_system.h"
#include "hardware.h"

// *******************************************************************************************************************************
//														   Timing
// *******************************************************************************************************************************

#define CRYSTAL_CLOCK 	(1000000L)													// Clock cycles per second (1.0 Mhz)
#define CYCLE_RATE 		(CRYSTAL_CLOCK/8)											// Cycles per second (8 clocks per cycle)
#define FRAME_RATE		(60)														// Frames per second (60)
#define CYCLES_PER_FRAME (CYCLE_RATE / FRAME_RATE)									// Cycles per frame (2,083 cycles per frame)

// *******************************************************************************************************************************
//														CPU / Memory
// *******************************************************************************************************************************

static BYTE8   	D,DF,X,P,T,IE,temp8;
static WORD16   R[16],temp16,cycles;
static BYTE8 	currentDevice;
static WORD16 	maxCycles;

static BYTE8 romMemory[2048];														// ROM Memory   (000-7FF)
static BYTE8 ramMemory[512];														// RAM Memory 	(800-9FF, 4 x mirrored)

// *******************************************************************************************************************************
//											 Memory and I/O read and write macros.
// *******************************************************************************************************************************

static inline BYTE8 __Read(WORD16 a) {
	return ((a) & 0x800) ? ramMemory[(a) & 0x1FF]:romMemory[(a) & 0x7FF];
}

static inline void __Write(WORD16 a,BYTE8 d) {
	if ((a) & 0x800) ramMemory[(a) & 0x1FF] = d;
}

#define READ(a) 	__Read(a)
#define WRITE(a,d) 	__Write(a,d)

// *******************************************************************************************************************************
//													   Port Interfaces
// *******************************************************************************************************************************

#include "_1801_include.h"

#define OUTPORT1(n)	currentDevice = (n)											// Out 1 sets the current currentDevice
#define OUTPORT2(n) HWIWriteDevice(currentDevice,n)								// Out 2 writes to the current device.

#include "_1801_ports.h"

// *******************************************************************************************************************************
//														Reset the CPU
// *******************************************************************************************************************************

void CPUReset(void) {
	HWIReset();
	RESET();
	maxCycles = CYCLES_PER_FRAME;
}

// *******************************************************************************************************************************
//												Execute a single instruction
// *******************************************************************************************************************************

BYTE8 CPUExecuteInstruction(void) {

	BYTE8 opcode = FETCH();															// Fetch opcode

	switch(opcode) {																// Execute it.
		#include "_1801_opcodes.h"
	}

	cycles = cycles + 2;															// 2 cycles per instruction
	if (cycles < maxCycles) return 0;												// Not completed a frame.
	if (IE != 0 && HWIIsScreenOn()) {												// Fire interrupt if it is enabled.
		INTERRUPT();
	}
	R[0] = HWIEndFrame(R[0],CRYSTAL_CLOCK);											// End of Frame code, setting R0 correctly.
	cycles = cycles - maxCycles;													// Adjust this frame rate.
	return FRAME_RATE;																// Return frame rate.
}

#ifdef INCLUDE_DEBUGGING_SUPPORT

// *******************************************************************************************************************************
//		Execute chunk of code, to either of two break points or frame-out, return non-zero frame rate on frame, breakpoint 0
// *******************************************************************************************************************************

BYTE8 CPUExecute(WORD16 breakPoint1,WORD16 breakPoint2) { 
	do {
		BYTE8 r = CPUExecuteInstruction();											// Execute an instruction
		if (r != 0) return r; 														// Frame out.
	} while (R[P] != breakPoint1 && R[P] != breakPoint2);							// Stop on breakpoint.
	return 0; 
}

// *******************************************************************************************************************************
//									Return address of breakpoint for step-over, or 0 if N/A
// *******************************************************************************************************************************

WORD16 CPUGetStepOverBreakpoint(void) {
	BYTE8 opcode = CPUReadMemory(R[P]);												// Current opcode.
	if (opcode >= 0xD0 && opcode <= 0xDF) return (R[P]+1) & 0xFFFF;					// If SEP Rx then step is one after.
	return 0;																		// Do a normal single step
}

// *******************************************************************************************************************************
//												Read/Write Memory
// *******************************************************************************************************************************

BYTE8 CPUReadMemory(WORD16 address) {
	return READ(address);
}

void CPUWriteMemory(WORD16 address,BYTE8 data) {
	WRITE(address,data);
}

// *******************************************************************************************************************************
//												Load a binary file into RAM
// *******************************************************************************************************************************

#include <stdio.h>

void CPULoadBinary(const char *fileName) {
	FILE *f = fopen(fileName,"rb");
	fread(romMemory,1,2048,f);
	fclose(f);
}

// *******************************************************************************************************************************
//											Retrieve a snapshot of the processor
// *******************************************************************************************************************************

static CPUSTATUS s;																	// Status area

CPUSTATUS *CPUGetStatus(void) {
	s.d = D;s.df = DF;s.p = P;s.x = X;s.t = T;s.ie = IE;							// Registers
	for (int i = 0;i < 16;i++) s.r[i] = R[i];										// 16 bit Registers
	s.cycles = cycles;s.pc = R[P];													// Cycles and "PC"
	return &s;
}

#endif
