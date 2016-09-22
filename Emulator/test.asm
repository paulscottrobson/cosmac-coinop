
	cpu 	1802
	
r0 = 0
r1 = 1
r2 = 2
r3 = 3
r4 = 4
r5 = 5
r6 = 6
rc = 12
re = 14
rf = 15

screen = 0800h

	ldi 	41
	ghi 	r0
	phi 	r3
	ldi 	start & 255
	plo 	r3
	sep 	r3
start:

	ldi 	9 																; set stack to 9FFh
	phi 	r2
	ldi 	0FFh
	plo 	r2

	ldi 	screen/256														; R0 points to screen
	phi 	r0
	ldi 	0
	plo 	r0
	phi 	r1 																; set interrupt
	ldi 	interruptRoutine & 255
	plo 	r1

	sex 	r3 																; X = P = 3
	out 	1
	db 		2 																; select device 2
	out 	2
	db 	 	3 																; command 3 (TV on)

	ldi 	screen/256
	phi		r4
	ldi 	0
	plo 	r4
fill:
	ldi 	255
	str 	r4
	inc 	r4
	glo 	r4
	bnz 	fill
wait:
	br 		wait

interruptExit:
	lda 	r2 																; restore D and return.
	ret 
interruptRoutine:
	dec 	r2 																; save XP on stack
	sav 
	dec 	r2 																; save D on stack.
	str 	r2
	ldi 	screen/256														; reset screen address
	phi 	r0
	ldi 	0
	plo 	r0
	br 		interruptExit
