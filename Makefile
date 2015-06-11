#定义变量
TOOLPATH = ../tools/
INCPATH = ../tools/voidOS/
MAKE = make -r
NASK = $(TOOLPATH)nask 
EDIMG = $(TOOLPATH)edimg
IMGTOL = $(TOOLPATH)imgtol
CC1 = $(TOOLPATH)cc1 -I$(INCPATH) -Os -Wall -quiet
GAS2NASK = $(TOOLPATH)gas2nask -a
OBJ2BIM = $(TOOLPATH)obj2bim
BIM2HRB = $(TOOLPATH)bim2hrb
RULEFILE =$(TOOLPATH)voidOS/voidOS.rul
COPY = cp
DEL = rm
MAKEFONT = $(TOOLPATH)makefont
BIN2OBJ = $(TOOLPATH)bin2obj

#自制的Makefile 参考原光盘内的Makefile

default:
	$(MAKE) img

#文件生成规则

ipl.bin: ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst


#edimg 在冒号后要紧跟文件名,不能在冒号之后加任何空白字符

asmhead.bin: asmhead.nas Makefile
	$(NASK) asmhead.nas asmhead.bin asmhead.lst

bootpack.gas: bootpack.c Makefile
	$(CC1) -o bootpack.gas bootpack.c

bootpack.nas: bootpack.gas Makefile
	$(GAS2NASK) bootpack.gas bootpack.nas

bootpack.obj: bootpack.nas Makefile
	$(NASK) bootpack.nas bootpack.obj bootpack.lst

naskfunc.obj: naskfunc.nas Makefile
	$(NASK) naskfunc.nas naskfunc.obj naskfunc.lst

bootpack.bim:bootpack.obj naskfunc.obj hankaku.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:bootpack.bim stack:3136k map:bootpack.map \
		bootpack.obj naskfunc.obj hankaku.obj

hankaku.bin: hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt hankaku.bin

hankaku.obj: hankaku.bin Makefile
	$(BIN2OBJ) hankaku.bin hankaku.obj _hankaku

#3MB+64KB=3136KB

bootpack.hrb: bootpack.bim Makefile
	$(BIM2HRB) bootpack.bim bootpack.hrb 0

voidOS.img: ipl.bin voidOS.sys Makefile
	$(EDIMG) imgin:../tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:voidOS.sys to:@: \
		imgout:voidOS.img

voidOS.sys: asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > voidOS.sys

#指令
asm:
	$(MAKE) ipl.bin

img:
	$(MAKE) voidOS.img

run:
	echo "Please run the disk in Virtualbox"

clean:
	
	-$(DEL) *.bin
	-$(DEL) *.lst
	-$(DEL) *.gas
	-$(DEL) *.obj
	-$(DEL) bootpack.nas
	-$(DEL) bootpack.map
	-$(DEL) bootpack.bim
	-$(DEL) bootpack.hrb
	-$(DEL) voidOS.sys
	-$(DEL) voidOS.img

src_only:
	$(MAKE) clean
	-$(DEL) voidOS.img
