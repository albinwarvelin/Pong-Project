  # labwork.S
  # Written 2015-2017 by F Lundevall
  # Skeleton file for IS1200/IS1500 lab 1.
  # The macros PUSH and POP are in the public domain.
  # Please add your own code at the end of the file.

  #
  # Please keep the two macros PUSH and POP unchanged
  #
.macro	PUSH reg
	addi	$sp,$sp,-4
	sw	\reg,0($sp)
.endm

.macro	POP reg
	lw	\reg,0($sp)
	addi	$sp,$sp,4
.endm
  #
  # Please add your own code below this line
  #

# enables interrupts
.global enable_interrupt
enable_interrupt:
	ei
	jr $ra
	
# you can write your code for subroutine "hexasc" below this line
hexasc:
	andi $v0,$a0,0xf	#discard bits except 4 least significant bits
	addi $v0,$v0,0x30	#add 0x30 to return number
	jr $ra			#jump back to saved address in main

#delay with parameter as millisecond
.global delay
delay:
	beqz $a0,enddelay	#if parameter is equal to zero jump to end
	addi $a0,$a0,-1		#else decrement parameter (milliseconds) by one
	li $t0,5374		#initialize loop
loop:	
	beqz $t0,delay 		#if loop variable is equal to zero jump to delay
	addi $t0,$t0,-1		#else decrement variable by one
	j loop 			#jump to loop top
enddelay:	
	jr $ra			#end delay
	
#converts time to string	
.global time2string
time2string:
	andi $a1,$a1,0xffff	#ignore bits other than 16 least significant
	
	li $t0,0x00
	sb $t0,5($a0)		#stores 4th digit
	
	PUSH $a0		#saves memory address in $a0 to stack
	PUSH $ra		#saves return address
	move $a0,$a1		#moves time to $a0 as hexasc takes $a0 as input
	jal hexasc		
	POP $ra
	POP $a0
	sb $v0,4($a0)		#stores 4th digit
	
	PUSH $a0		#saves memory address in $a0 to stack
	PUSH $ra		#saves return address
	move $a0,$a1		#moves time to $a0 as hexasc takes $a0 as input
	srl $a0,$a0,4
	jal hexasc		
	POP $ra
	POP $a0
	sb $v0,3($a0)		#stores 3rd digit
	
	li $t0,0x3a
	sb $t0,2($a0)		#stores 4th digit
	
	PUSH $a0		#saves memory address in $a0 to stack
	PUSH $ra		#saves return address
	move $a0,$a1		#moves time to $a0 as hexasc takes $a0 as input
	srl $a0,$a0,8
	jal hexasc		
	POP $ra
	POP $a0
	sb $v0,1($a0)		#stores 2nd digit
	
	PUSH $a0		#saves memory address in $a0 to stack
	PUSH $ra		#saves return address
	move $a0,$a1		#moves time to $a0 as hexasc takes $a0 as input
	srl $a0,$a0,12
	jal hexasc		
	POP $ra
	POP $a0
	sb $v0,0($a0)		#stores 1st digit
	
	jr $ra