.set noreorder
.set noat

.globl    array_size
.globl    array_start

.text
.globl _start
.ent _start

_start:

	la	$a0, array_start
	la  	$a1, array_size
	lw   	$a1, 0($a1) // number of elements in the array

	addi	$t0, $0, 4
	mul	$t1, $a1, $t0
	add	$s0, $a0, $t1 //inicializing max address
	addi	$t7, $0, 0 //boolean if array is ordered
while:
	bne	$t7, $0, end_loop //if no swap was performed, array must be ordered
	nop
	beq	$s0, $a0, end_loop //max iterations necessary to order the array were reached
	nop

	la	$s1, array_start //inicializing iterator
	addi	$t7, $0, 1
	sub	$s0, $s0, $t0
	
for:
	beq	$s1, $s0, while //if iteration is over go back
	nop

	lw	$t1, 0($s1)
	add	$a2, $0, $s1
	addi	$s1, $s1, 4
	lw	$t2, 0($s1)
	add	$a3, $0, $s1

	slt	$t3, $t2, $t1
	beq	$t3, $0, for //if i+1 > i swap them, else continue
	nop

swap:
	lw	$t1, 0($a2)
	lw	$t2, 0($a3)
	sw	$t2, 0($a2)
	sw	$t1, 0($a3)
	addi	$t7, $0, 0
	j	for
	nop
	

//Final infinite loop
end_loop:
	cache 9, 0($0)  // flush cache memory
	break           // stop the simulator
	j end_loop
	nop

.end _start

.data
// .align    2 // not supported by QtMips yet

array_size:
.word	15
array_start:
.word	5, 3, 4, 1, 15, 8, 9, 2, 10, 6, 11, 1, 6, 9, 12

// Specify location to show in memory window
#pragma qtmips focus memory array_size
