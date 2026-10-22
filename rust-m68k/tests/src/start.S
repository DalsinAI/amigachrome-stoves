	.text
	.globl _start
_start:
	jsr rmain
	move.l %d0,%d1
	moveq #1,%d0
	trap #0
	.globl puts
puts:	move.l 4(%sp),%a0
	move.l %a0,%d0
1:	tst.b (%a0)+
	bne 1b
	sub.l %d0,%a0
	subq.l #1,%a0
	move.l %d2,-(%sp)
	move.l %d3,-(%sp)
	move.l %d0,%d2
	move.l %a0,%d3
	moveq #1,%d1
	moveq #4,%d0
	trap #0
	pea nl
	move.l (%sp)+,%d2
	moveq #1,%d3
	moveq #1,%d1
	moveq #4,%d0
	trap #0
	move.l (%sp)+,%d3
	move.l (%sp)+,%d2
	moveq #0,%d0
	rts
	.globl abort
abort:	moveq #6,%d1
	moveq #1,%d0
	trap #0
	.data
nl:	.byte 10
	.section .note.GNU-stack,"",@progbits
