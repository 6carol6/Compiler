.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n"
.globl main
.text
read:
	li $v0, 4
	la $a0, _prompt
	syscall
	li $v0, 5
	syscall
	jr $ra
write:
	li $v0, 1
	syscall
	li $v0, 4
	la $a0, _ret
	syscall
	move $v0, $0
	jr $ra
sort:
	move $t0, $a0
	sw $t0 -32($sp)
	li $t0, 0
	sw $t0 -436($sp)
	lw $t0, -436($sp)
	sw $t0 -440($sp)
	li $t0, 0
	sw $t0 -444($sp)
	lw $t0, -444($sp)
	sw $t0 -448($sp)
label1:
	lw $t0, -440($sp)
	sw $t0 -452($sp)
	lw $t0, -32($sp)
	sw $t0 -456($sp)
	lw $t1, -452($sp)
	lw $t2, -456($sp)
	blt $t1, $t2, label2
	j label3
label2:
	lw $t1, -440($sp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0 -460($sp)
	la $t1, -36($sp)
	lw $t2, -460($sp)
	sub $t0, $t1, $t2
	sw $t0 -464($sp)
	li $t0, 100
	sw $t0 -468($sp)
	lw $t0, -440($sp)
	sw $t0 -472($sp)
	li $t0, 2
	sw $t0 -476($sp)
	lw $t1, -472($sp)
	lw $t2, -476($sp)
	mul $t0, $t1, $t2
	sw $t0 -480($sp)
	lw $t1, -468($sp)
	lw $t2, -480($sp)
	sub $t0, $t1, $t2
	sw $t0 -484($sp)
	lw $t0, -464($sp)
	lw $t1, -484($sp)
	sw $t1, 0($t0)
	sw $t0 -464($sp)
	lw $t0, -440($sp)
	sw $t0 -488($sp)
	li $t0, 1
	sw $t0 -492($sp)
	lw $t1, -488($sp)
	lw $t2, -492($sp)
	add $t0, $t1, $t2
	sw $t0 -496($sp)
	lw $t0, -496($sp)
	sw $t0 -440($sp)
	j label1
label3:
	li $t0, 0
	sw $t0 -500($sp)
	lw $t0, -500($sp)
	sw $t0 -440($sp)
label4:
	lw $t0, -440($sp)
	sw $t0 -504($sp)
	lw $t0, -32($sp)
	sw $t0 -508($sp)
	lw $t1, -504($sp)
	lw $t2, -508($sp)
	blt $t1, $t2, label5
	j label6
label5:
	li $t0, 0
	sw $t0 -512($sp)
	lw $t0, -512($sp)
	sw $t0 -448($sp)
label7:
	lw $t0, -448($sp)
	sw $t0 -516($sp)
	lw $t0, -440($sp)
	sw $t0 -520($sp)
	lw $t1, -516($sp)
	lw $t2, -520($sp)
	blt $t1, $t2, label8
	j label9
label8:
	lw $t1, -440($sp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0 -524($sp)
	la $t1, -36($sp)
	lw $t2, -524($sp)
	sub $t0, $t1, $t2
	sw $t0 -528($sp)
	lw $t1, -448($sp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0 -532($sp)
	la $t1, -36($sp)
	lw $t2, -532($sp)
	sub $t0, $t1, $t2
	sw $t0 -536($sp)
	lw $t1, -528($sp)
	lw $t0, 0($t1)
	sw $t0 -540($sp)
	lw $t1, -536($sp)
	lw $t0, 0($t1)
	sw $t0 -544($sp)
	lw $t1, -540($sp)
	lw $t2, -544($sp)
	blt $t1, $t2, label10
	j label11
label10:
	lw $t1, -440($sp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0 -548($sp)
	la $t1, -36($sp)
	lw $t2, -548($sp)
	sub $t0, $t1, $t2
	sw $t0 -552($sp)
	lw $t1, -552($sp)
	lw $t0, 0($t1)
	sw $t0 -556($sp)
	lw $t1, -440($sp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0 -560($sp)
	la $t1, -36($sp)
	lw $t2, -560($sp)
	sub $t0, $t1, $t2
	sw $t0 -564($sp)
	lw $t1, -448($sp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0 -568($sp)
	la $t1, -36($sp)
	lw $t2, -568($sp)
	sub $t0, $t1, $t2
	sw $t0 -572($sp)
	lw $t1, -572($sp)
	lw $t0, 0($t1)
	sw $t0 -576($sp)
	lw $t0, -564($sp)
	lw $t1, -576($sp)
	sw $t1, 0($t0)
	sw $t0 -564($sp)
	lw $t1, -448($sp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0 -580($sp)
	la $t1, -36($sp)
	lw $t2, -580($sp)
	sub $t0, $t1, $t2
	sw $t0 -584($sp)
	lw $t0, -584($sp)
	lw $t1, -556($sp)
	sw $t1, 0($t0)
	sw $t0 -584($sp)
label11:
	lw $t0, -448($sp)
	sw $t0 -588($sp)
	li $t0, 1
	sw $t0 -592($sp)
	lw $t1, -588($sp)
	lw $t2, -592($sp)
	add $t0, $t1, $t2
	sw $t0 -596($sp)
	lw $t0, -596($sp)
	sw $t0 -448($sp)
	j label7
label9:
	lw $t0, -440($sp)
	sw $t0 -600($sp)
	li $t0, 1
	sw $t0 -604($sp)
	lw $t1, -600($sp)
	lw $t2, -604($sp)
	add $t0, $t1, $t2
	sw $t0 -608($sp)
	lw $t0, -608($sp)
	sw $t0 -440($sp)
	j label4
label6:
	li $t0, 0
	sw $t0 -612($sp)
	lw $t0, -612($sp)
	sw $t0 -440($sp)
label12:
	lw $t0, -440($sp)
	sw $t0 -616($sp)
	lw $t0, -32($sp)
	sw $t0 -620($sp)
	lw $t1, -616($sp)
	lw $t2, -620($sp)
	blt $t1, $t2, label13
	j label14
label13:
	lw $t1, -440($sp)
	li $t2, 4
	mul $t0, $t1, $t2
	sw $t0 -624($sp)
	la $t1, -36($sp)
	lw $t2, -624($sp)
	sub $t0, $t1, $t2
	sw $t0 -628($sp)
	lw $t1, -628($sp)
	lw $t0, 0($t1)
	sw $t0 -632($sp)
	lw $t1, -632($sp)
	move $a0, $t1
	addi $sp, $sp, -636
	sw $ra, 0($sp)
	jal write
	lw $ra, 0($sp)
	addi $sp, $sp, 636
	lw $t0, -440($sp)
	sw $t0 -636($sp)
	li $t0, 1
	sw $t0 -640($sp)
	lw $t1, -636($sp)
	lw $t2, -640($sp)
	add $t0, $t1, $t2
	sw $t0 -644($sp)
	lw $t0, -644($sp)
	sw $t0 -440($sp)
	j label12
label14:
	li $t0, 0
	sw $t0 -648($sp)
	move $v0, $t0
	jr $ra
main:
	addi $sp, $sp, -652
	sw $ra, 0($sp)
	jal read
	lw $ra, 0($sp)
	addi $sp, $sp, 652
	move $t0, $v0
	sw $t0 -652($sp)
	lw $t0, -652($sp)
	sw $t0 -656($sp)
	lw $t0, -656($sp)
	sw $t0 -660($sp)
	li $t0, 100
	sw $t0 -664($sp)
	lw $t1, -660($sp)
	lw $t2, -664($sp)
	bge $t1, $t2, label15
	j label16
label15:
	li $t0, 1
	sw $t0 -668($sp)
	lw $t1, -668($sp)
	li $t0, 0
	sub $t0, $t0, $t1
	sw $t0 -672($sp)
	lw $t1, -672($sp)
	move $a0, $t1
	addi $sp, $sp, -676
	sw $ra, 0($sp)
	jal write
	lw $ra, 0($sp)
	addi $sp, $sp, 676
	j label17
label16:
	lw $t0, -656($sp)
	sw $t0 -676($sp)
	lw $a0, -676($sp)
	addi $sp, $sp, -680
	sw $ra, 0($sp)
	jal sort
	move $t0, $v0
	lw $ra, 0($sp)
	addi $sp, $sp, 680
	sw $t0 -680($sp)
label17:
	li $t0, 0
	sw $t0 -684($sp)
	move $v0, $t0
	jr $ra
