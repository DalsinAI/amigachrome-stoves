	.text
	.globl _start
_start:
	move.l	%sp,%a0
	jsr	main
	move.l	%d0,%d1
	moveq	#1,%d0
	trap	#0
