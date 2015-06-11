; The most easy OS
; By VOID001 thx to 30days_make_an_os

;Define the FAT12 Floppy disk settings
CYLS	EQU		10			;读取的柱面数 CYLS=10
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
;加入了试错功能
	MOV		AX,0x0820
	MOV		ES,AX			;设置在内存的存储位置为0x0820
	MOV		CH,0			;柱面0
	MOV		DH,0			;磁头0
	MOV		CL,2			;扇区2
;添加了循环读取到18扇区的代码
readloop:
	MOV		SI,0			;错误次数置0
	
retry:
	MOV		AH,0x02
	MOV		AL,1
	MOV		BX,0
	MOV		DL,0x00
	INT		0x13			;调用磁盘BIOS
	JNC		next
	INC		SI				;错误次数+1
	CMP		SI,5
	JA		err
	MOV		AH,0x00			;54,55为什么要初始化不明白
	MOV		DL,0x00
	JMP		retry				;如果CF标志为1说明出错,跳转到err执行相应代码
next:
	MOV		AX,ES			;用这个指令来移动内存指针,ES+0x20 就是当前内存地址后移 0x200即512个单元,也就是一个扇区的大小
	ADD		AX,0x20
	MOV		ES,AX
	ADD		CL,1
	CMP		CL,18
	JBE		readloop
	MOV		CL,1
	ADD		DH,1
	CMP		DH,2
	JB		readloop
	MOV		DH,0
	ADD		CH,1
	CMP		CH,CYLS
	JB		readloop
	;JE		success

	MOV		[0xff0],CH				;这里需要更改使得voidOS.sys能被执行
	JMP		0xc200
;输出错误信息并且进入fin循环
err:
	MOV		SI,msg
putloop:
	MOV		AL,[SI]
	ADD		SI,1
	CMP		AL,0
;	JE		fin
	MOV		AH,0x0e
	MOV		BX,15
	INT		0x10
	JMP		putloop

success:
	MOV		SI,sucmsg
	JMP		putloop
	
;Show Message
msg:
DB		0x0a,0x0a
DB		"VOID OS Load Error!"
DB		0x0a
DB		0

sucmsg
DB		0x0a,0x0a
DB		"VOID OS Load Success!"
DB		0x0a
DB		0

RESB	0x7dfe-$
DB		0x55,0xAA

;IPL 的作用是将我们的操作系统装载到内存的指定区域中,这个程序是在计算机一启动就运行的,之后才装载操作系统
