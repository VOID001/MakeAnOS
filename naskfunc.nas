; naskfunc
; TAB=4

[FORMAT "WCOFF"]		;制作的目标文件模式
[INSTRSET "i486p"]
[BITS 32]				;32位机器语言

[FILE "naskfunc.nas"]
	
	GLOBAL	_io_hlt,_write_mem8

[SECTION .text]			;必须写

_io_hlt:	;void io_hlt(void);
	HLT
	RET

_write_mem8:	;void write_mem8(int addr,int data);
	;定义新函数用于向内存中写入数据
	MOV		ECX,[ESP+4]	;ESP+4中存放的是addr的值
	MOV		AL,[ESP+8]	;ESP+8中存放的是data的值
	MOV		[ECX],AL
	RET


