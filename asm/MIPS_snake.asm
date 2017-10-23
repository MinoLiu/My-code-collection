#	MIPS SNAKE game
#	Bitmap display options:
#	Unit Width in Pixels:	16
#	Unit Height in Pixels:	 16
#	Display Width in Pixels: 512
#	Display Height in Pixels: 512
#	Base address for display: $gp
#---------------------------------------------------------------------------
#	$s0	snakeaddress address
#	$s1	futureheadaddress
#	$s2	newFirstbody or oldheadaddress
#	$s3	bodylength
#	$s4	food.X
#	$s7	food.Y
#	$s5	reverse flag
#	$s6	growup for 1; common for 0
#	$a3	input ASCII
#	$t4	snake.head.X
#	$t5	snake.head.Y
#	$v1	safe_mode_move_position
#	$a2	ate food flag
##########################################################
.data
	outstring: .asciiz "Snake length= "
	dot: .asciiz ", "
	#check_error: .asciiz	"OK\n"
	newline: .asciiz "\n"
	side: .half 32			#Give a length 32 of a bitmap
	black: .half 0x0
	growupflag: .half 1
	vertical:	.half 30
	horizontal:	.half 30
	check_error:	.half 	0
	contourcolor: .word 0xabcdef
	foodcolor: .word 0xCC0000
	snakecolor: .word 0xAAAAAAAA
	bodylength: .half 3
	startaddress: .word 0x10008000
	snakeaddress: .word 0x0	 	#snakeaddress array

	

##########################################################################	
.text
init:			
	li $t0, 0x1000819c		#Assign snake address to $t0
	sw $t0, snakeaddress		#Store snake address from $t0 to "snakeaddress"
	jal clear			#Call clear function
	lw $t0, startaddress		#Load display start address
	jal row				#Call the up row function
	jal columns			#Call the column function
	jal row				#Call the bottom row funtion
	jal snake			#Call the snake funtion
	jal food			#Call the food funtion	
	
################################################################	
Strategy:
	li $a0,10			#Decrease to increase difficulty; Increase to decrease difficulty
	li $v0,32			#Sleep for 60 milliseconds
	syscall
	jal snakeXY			# get snake head coordinate
###############################################################
# $s4 = food.X, $s7 = food.Y, $t4 = head.X, $t5 = head.Y
	lw $s3, bodylength
	bge $s3, 40, Safe_mode		# if bodylength >= 40 , branch Safe_mode
Normal_mode:
	bge $t4,$s4, Move_left		# foodX > headX branch move_left
	j Move_right			# < j move_left
Safe_mode:
	beq $t5, 30, setLeft		# when touch boundary then reset flag and position
	beq $t4, 30, setUnder
	beq $t5, 1, setRight
	beq $t4, 1, setUp
setReturn:
	beq $v1, 1, left_safe		# branch for $v1 position
	beq $v1, 2, under_safe
	beq $v1, 3, right_safe
	beq $v1, 4, up_safe
	j up				# when entry the safe_mode , go to touch top boundary
setLeft:
	li $v1, 1		# set position to left
	li $a2, 0		# reset growup flag
	j setReturn		# return
setUnder:
	li $v1, 2		# set position to left
	li $a2, 0		# reset growup flag
	j setReturn		# return
setRight:
	li $v1, 3		# set position to left
	li $a2, 0		# reset growup flag
	j setReturn		# return
setUp:
	li $v1, 4		# set position to left
	li $a2, 0		# reset growup flag
	j setReturn		# return
reverse:
	lw $s3, bodylength
	li $t6,0	# left count clear
	li $t7,0	# right count clear
	li $t8,0	# under count clear
	li $t9,0	# up count clear
	jal leftcheck	# calculate left space
	jal rightcheck	# calculate right space
	jal undercheck	# calculate under space
	jal upcheck	# calculate up space
	bnez $s5, final	# already reverse one times , gameover
	li $s5, 1	# set flag
leftC:
	blt $t6, $t7, rightC		# left < right branch rightC
	blt $t6, $t8, underC		# left < under branch underC 
	blt $t6, $t9, upC		# left < up branch upC
	j left
rightC:
	blt $t7, $t6, leftC		# same
	blt $t7, $t8, underC
	blt $t7, $t9, upC
	j right
underC:
	blt $t8, $t6, leftC		# same
	blt $t8, $t7, rightC
	blt $t8, $t9, upC
	j under
upC:
	blt $t9, $t6, leftC		# same
	blt $t9, $t7, rightC
	blt $t9, $t8, underC
	j up

Move_left:
	bgt $t5, $s7, up		#  headY > foodY branch up
	blt $t5, $s7, under		# headY < foodY branch under
	j left				# headY == foodY j left
Move_right:
	bgt $t5,$s7, up			#  headY > headY	branch up
	blt $t5,$s7, under		#  headY < headY	branch under
	j right				# headY == foodY j right
	
###############################################################

#	lw $t0,0xffff0000 	
#	blez $t0, Direction 
#	lw $a3,0xffff0004
	
#Direction:	
#	beq $a3, 0x73,under		#Check if it is 's'
#	beq $a3, 0x64,right		#Check if it is 'd'
#	beq $a3, 0x61,left		#Check if it is 'a'
#	beq $a3, 0x77, up		#Check if it is 'w'
#	j Strategy				#Thread sleep and Key check loop
	
################################################################# check move
leftcheck:
	la $s0, snakeaddress 		#Load the head address to $s0
	lw $s0, ($s0)			#Load old head address to $s0
loop1:	subi $s0,$s0, 4			#Get next head
	lw $t0, ($s0)			#Load color occupied by the next head address in ($s1)
	addi $t6, $t6, 1		# left count + 1
	beqz $t0, loop1			# if $t0 color == black branch loop1
	jr $ra
rightcheck:
	la $s0, snakeaddress 		#Load the head address to $s0
	lw $s0, ($s0)			#Load old head address to $s0
loop2:	addi $s0,$s0,4			#New head address
	lw $t0, ($s0)			#Load color occupied by the next head address in ($s1)
	addi $t7, $t7, 1		# left count + 1
	beqz $t0, loop2			# if $t0 color == black branch loop2
	jr $ra
undercheck:							
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s0,($s0)			#Load old head address to $s2
loop3:	addi $s0,$s0,128		#New head address to $s1
	lw $t0, ($s0)			#Load color occupied by the next head address in ($s1)
	addi $t8, $t8, 1		# left count + 1
	beqz $t0, loop3			# if $t0 color == black branch loop3
	jr $ra
upcheck:
	la $s0, snakeaddress 		#Load the head address to $s0
	lw $s0,($s0)			#Load old head address to $s2
loop4:	subi $s0,$s0,128		#New head address
	lw $t0, ($s0)			#Load color occupied by the next head address in ($s1)
	addi $t9, $t9, 1		# left count + 1
	beqz $t0, loop4			# if $t0 color == black branch loop4
	jr $ra
#################################################################	
#execution(NEW snake head address)
under:							
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s2,($s0)			#Load old head address to $s2
	addi $s1,$s2,128		#New head address to $s1
	
	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, reverse		# if color == contourcolor branch reverse
	beq $t0,$t2, reverse		# if color == snakecolor branch reverse
	
	
	j judge				#goto judge	
right:
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s2,($s0)			#Load old head address
	addi $s1,$s2,4			#New head address
	
	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, reverse		# if color == contourcolor branch reverse
	beq $t0,$t2, reverse		# if color == snakecolor branch reverse
	
	j judge

left:
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s2,($s0)			#Load old head address
	subi $s1,$s2,4			#New head address

	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, reverse		# if color == contourcolor branch reverse
	beq $t0,$t2, reverse		# if color == snakecolor branch reverse
	
	j judge

up:
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s2,($s0)			#Load old head address
	subi $s1,$s2,128		#New head address
	
	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, reverse		# if color == contourcolor branch reverse
	beq $t0,$t2, reverse		# if color == snakecolor branch reverse
	
	j judge
	
under_safe:
	bnez $a2, us2			# if $a2 == 1 , mean already ate food branch us2
	seq $t0, $t5,$s7		# if headY == foodY, $t0 = 1
	beqz $t0, us2			# $t0 == 0, branch us2
	ble $s4, 15, us2		# foodX <= 15 branch us2
	j left				# go to eat
us2:
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s2,($s0)			#Load old head address to $s2
	addi $s1,$s2,128		#New head address to $s1
	
	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, reverse		# if color == contourcolor branch reverse
	beq $t0,$t2, reverse		# if color == snakecolor branch reverse
	
	
	j judge				#goto judge
right_safe:
	bnez $a2,rs2			# if $a2 == 1 , mean already ate food branch rs2
	seq $t0, $t4, $s4		# if headX == foodX, $t0 = 1
	beqz $t0, rs2			# $t0 == 0, branch rs2
	bgt $s7, 15, rs2		# foodY > 15 branch rs2
	j under				# go to eat
rs2:
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s2,($s0)			#Load old head address
	addi $s1,$s2,4			#New head address
	
	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, reverse		# if color == contourcolor branch reverse
	beq $t0,$t2, reverse		# if color == snakecolor branch reverse
	
	j judge

left_safe:
	bnez $a2, ls2			# if $a2 == 1 , mean already ate food branch ls2
	seq $t0, $t4, $s4		# if headX == foodX, $t0 = 1
	beqz $t0, ls2			# $t0 == 0, branch ls2
	ble $s7, 15, ls2		# if foodY <= 15, branch ls2
	j up				# go to eat
ls2:
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s2,($s0)			#Load old head address
	subi $s1,$s2,4			#New head address

	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, reverse		# if color == contourcolor branch reverse
	beq $t0,$t2, reverse		# if color == snakecolor branch reverse
	
	j judge

up_safe:
	bnez $a2, ups2			# if $a2 == 1 , mean already ate food branch ups2
	seq $t0, $t5,$s7		# if headY == foodY, $t0 = 1
	beqz $t0, ups2			# if $t0 == 0, branch ups2
	bgt $s4, 15, ups2		# if headX > 15, branch ups2
	j right				# go to eat
ups2:
	la $s0, snakeaddress		#Load the head address to $s0
	lw $s2,($s0)			#Load old head address
	subi $s1,$s2,128		#New head address
	
	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, reverse		# if color == contourcolor branch reverse
	beq $t0,$t2, reverse		# if color == snakecolor branch reverse
	
	j judge
	
##################################################################
judge:
	li $s5, 0 			#reset $s5 flag
	jal  check_final
	#lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	#lw $t3,foodcolor		#Load foodcolor to $t3
	#beq $t0,$t3,growup		#If I eat the food, I grow up
	seq $t0, $s4, $t4	# headX == foodX, $t0 = 1 
	beqz $t0, snake_move	# if $t0 == 0 , branch snake_move
	seq $t0, $s7, $t5	# headY == foodY, $t0 = 1
	beq $t0, 1, growup	# $t0 == 1, branch growup
snake_move:	
	lw $t0,snakecolor		#Load snakecolour to $t0
	lw $s3,bodylength		#Load bodylength to $s3
	sw $t0,($s1)			#Colour new head
	sw $s1,($s0)			#Store new head address to "snakeaddress"
	jal snakebody_shift
	j  Strategy



final:
	li $s3,3			#Reset bodylength to 3
	sh $s3,bodylength		
	lw $a3,0x10010000
	li $a0,500			#Sleep for 0.5s when dead
	li $v0,32
	syscall
	j init			#Return to the game

#function
################################################################################	
#initialization use	function
################################################################################
#Clear the screen (initialize the bitmap)-----------------------------------------------------
clear:
	lw $t0,startaddress		#startaddress = 0x10008000
	lh $t1, side			#Load the side of the bitmap
	mul $t1,$t1,$t1			#Width Pixels * Height Pixels
	lh $t2, black
clearloop:					#Loop until every display address has the black number in it (zero)		
	sw $t2,($t0)			#Initialize all pixels to Black
	addi $t0,$t0,4			#Next pixel
	subi $t1,$t1,1			#pixels need to be initialize = pixels need to be initialize - 1
	bnez $t1,clearloop		#if (pixels need to be initialize != 0) goto clearloop
	jr $ra				#back to mStrategyn

	
#This function is used to draw both the up row and the bottom row-------------------------------
row:								
	lw $t1, contourcolor		#Load contourcolor
	lh $t2, side			#Load the side of the bitmap
drawrow:					#Row loop					
	sw $t1,($t0)			#Draw contour
	addi $t0,$t0,4			#Next pixel
	subi $t2,$t2,1			#pixels need to be draw = pixels need to be draw - 1
	bnez $t2, drawrow		#if (pixels need to be draw != 0) goto drawrow
	jr $ra				#back to mStrategyn
	
	
#Draw the columns--------------------------------------------------------------------------------	
columns:						
	lw $t0, startaddress		#Load the start address
	addi $t0,$t0,128		#Go to the next line: add 0x80
	lh $t2, side			#Load the side of the bitmap
	subi $t2,$t2,2			#Substract the up and down row pixel where x=0 and x=31
drawcolumns:				#Columns Loop
	sw $t1,($t0)			#Draw contourcolor (the countourcolor is still in $t1 after row function was called)
	addi $t0,$t0,124		#Add 31 pixels to the end of the row
	sw $t1,($t0)			#Load contourcolor (draw the column at the end of rows)
	addi $t0,$t0,4			#Next pixel (from the last pixel to the first pixel in next line)
	subi $t2,$t2,1			#pixels need to be draw = pixels need to be draw - 1
	bnez $t2, drawcolumns		#if (pixels need to be draw != 0) goto drawcolumns
	jr $ra				#back to mStrategyn
	
	
#Draw the snake-----------------------------------------------------------------------------------
snake:					
	la $s0,snakeaddress		#Load the address of "snakeaddress"
	lw $s3,bodylength		#Load bodylength
	lw $t0,snakecolor		#Load snakecolor
	lw $t1,($s0)			#Load the address of snake's head which was saved in "snakeaddress" to $t1
drawsnake:				#Snake loop
	sw $t0,($t1)			#Store snake color
	sw $t1,($s0)			#Store next bodypart address to "snakeaddress" array
	subi $t1,$t1,4			#Move $t1 to the next bodypart address (the next bodypart is on the left of the head)
	addi $s0,$s0,4			#Move to the next element of "snakeaddress" array
	subi $s3,$s3,1			#bodylength = bodylength - 1
	bnez $s3, drawsnake		#if (bodylength != 0) goto drawsnake
snakeXY:
	la $s0, snakeaddress		#Load the head address to $s0
	lw $t0, ($s0)
	subi $t0,$t0,0x10008000		#head.X = ( headAddress - 0x10008000 ) % 0x80 /4
	div $t0,$t0,0x80
	mfhi $t0
	mflo $t5			#$t5 = head.Y = ( headAddress - 0x10008000) / 0x80	
	div $t4,$t0,4			#$t4 = head.X
	jr $ra				#back to mStrategyn

#Draw the food with  a pseudorandom address----------------------------------------------------------
food:						
	li $a1,0x3c0			#$a1= 32 pixel * (32 - 2)pixel ; food will be randomly gernerate within 0x10008080 ~ (0x10008F80 - 4)
	li $v0,42			#Generate a random number to $a0 ( from 0 to value($a1) )
	syscall		
	sll $a0, $a0, 2			#Multiply $a0 with 4 to generate the address (4 bytes for each pixel)
	add $a0, $a0,0x10008080		#Put the random number on the bitmap address
	lw $t0, ($a0)			#And then check if the new address is already colored
	bnez $t0,food			#Food loop
	lw $t1,foodcolor		#Load foodcolor
	sw $t1,($a0)			#Finally draw the food
foodXY:  #Calculate food's coordinate
	subi $t0,$a0,0x10008000		#food.X = ( foodaddress - 0x10008000 ) % 0x80 /4
	div $t0,$t0,0x80
	mfhi $t0			
	mflo $s7			#food.Y = ( foodaddress - 0x10008000) / 0x80	
	div $s4,$t0,4			#$s4 = food.X
	jr $ra
########################################################################################################	
#"judge" use function
########################################################################################################
#----------------------------------------------------------------------------------------------
check_final:			
	lw	$t0,($s1)		#Load color occupied by the next head address in ($s1)
	lw $t1,contourcolor		#Load contourcolor to $t1
	lw $t2,snakecolor		#Load snakecolor to $t2
	beq $t0,$t1, final		#If I hit the countour, I lose
	beq $t0,$t2,final		#If I bit myself, I lose
	jr $ra				#back to common

#snake growup -----------------------------------------------------------------------------------
growup:	
	lw $s3,bodylength		
	addi $s3,$s3,1			#Increase bodylength
	sw $s3, bodylength		#Store new bodylength
	li $a2, 1			# Growup flag
	lw $t0,snakecolor
	sw $t0,($s1)			#color new head
	lw $s2,($s0)			#Load old head address
	sw $s1,($s0)			#Store new head address to "snakeaddress"	
	li $s6,1			
	sh $s6,growupflag	
#print message-----------------------------------------
	li $v0,4
	la $a0, outstring
	syscall
	li $v0, 1
	lw $a0, bodylength
	syscall
	li $v0,4
	la $a0, newline
	syscall
#--------------------------------------------------------
	jal food				#in initialization use function
		
	j	snake_move
	
	
snakebody_shift:
	subi $s3,$s3,1			#bodylength = bodylength - 1
	addi $s0,$s0,4			#Move to the next element of "snakeaddress" array
	lw $t0,($s0)			#Load old bodypart address in "snakeaddress" array to $t0
	sw $s2,($s0)			#Store next new bodypart to "snakeaddress" array
	move $s2,$t0			#Store old bodypart from $s2 to $t0 for being new next bodypart
	bnez $s3,snakebody_shift			#if (bodylength != 0) goto shift	
	lw $t1,($s0)			#Load the tStrategyl from the end of "snakeaddress" array to $t1
	lh $t2,black			#Erase tStrategyl
	sw $t2,($t1)			
	jr $ra				#back to growup or common	
	
	
############################################################################################################


############################################################################################################
