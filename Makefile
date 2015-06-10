#定义变量
TOOLPATH = ../tools/
MAKE = make -r
NASK = $(TOOLPATH)nask 
EDIMG = $(TOOLPATH)edimg
IMGTOL = $(TOOLPATH)imgtol
COPY = cp
DEL = rm

#自制的Makefile 参考原光盘内的Makefile

default:
	$(MAKE) img

#文件生成规则
ipl.bin: ipl.nas Makefile
	$(NASK) ipl.nas ipl.bin ipl.lst
#edimg 在冒号后要紧跟文件名,不能在冒号之后加任何空白字符
voidOS.img: ipl.bin voidOS.sys Makefile
	$(EDIMG) imgin:../tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 \
		copy from:voidOS.sys to:@: \
		imgout:voidOS.img

voidOS.sys: voidOS.nas Makefile
	$(NASK) voidOS.nas voidOS.sys voidOS.lst

#指令
asm:
	$(MAKE) ipl.bin

img:
	$(MAKE) voidOS.img

run:
	echo "Please run the disk in Virtualbox"

clean:
	
	-$(DEL) ipl.bin
	-$(DEL) ipl.lst
	-$(DEL) voidOS.sys
	-$(DEL)	voidOS.lst

src_only:
	$(MAKE) clean
	-$(DEL) voidOS.img
