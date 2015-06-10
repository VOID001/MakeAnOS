; The most easy OS
; By VOID001 thx to 30days_make_an_os

;Define the FAT12 Floppy disk settings
ORG		0x7c00				;Tell PC to load Program into 0x7c00 ,这个值必须为0x7c00
JMP		entry
DB		0x90
DB		"VOIDIPL0"
DW		512
DB		1
DW		1
DB		2
DW		224
DW		2880
DB		0xf0
DW		9
DW		18
DW		2
DD		0
DD		2880
DB		0,0,0x29
DD		0xffffffff
DB		"MY_OS      "
DB		"FAT12   "
RESB	18

;Main Program


entry:
	MOV		AX,0
	MOV		SS,AX
	MOV		SP,0x7c00
	MOV		DS,AX
;下面的代码是用来将我们盘内的内容读入内存,读入一个扇区
	MOV		AX,0x0820
	MOV		ES,AX			;设置在内存的存储位置为0x0820
	MOV		CH,0
	MOV		DH,0
	MOV		CL,2
	
	MOV		AH,0x02
	MOV		AL,1
	MOV		BX,0
	MOV		DL,0x00
	INT		0x13			;调用磁盘BIOS
	JC		err				;如果CF标志为1说明出错,跳转到err执行相应代码
fin:
	HLT

	JMP		fin
;输出错误信息并且进入fin循环
err:
	MOV		SI,msg
putloop:
	MOV		AL,[SI]
	ADD		SI,1
	CMP		AL,0
	JE		fin
	MOV		AH,0x0e
	MOV		BX,15
	INT		0x10
	JMP		putloop
	
;Show Message
msg:
DB		0x0a,0x0a
DB		"I am VOID001,Day_1 Finished!"
DB		0x0a
DB		0
RESB	0x7dfe-$
DB		0x55,0xAA

;IPL 的作用是将我们的操作系统装载到内存的指定区域中,这个程序是在计算机一启动就运行的,之后才装载操作系统
