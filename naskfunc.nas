; naskfunc
; TAB=4

[FORMAT "WCOFF"]		;制作的目标文件模式
[BITS 32]				;32位机器语言

[FILE "naskfunc.nas"]
	
	GLOBAL	_io_hlt

[SECTION .text]			;必须写

_io_hlt:	;void io_hlt(void);
	HLT
	RET
