#自制的Makefile 参考原光盘内的Makefile

default:
	make img

#文件生成规则
ipl.bin: ipl.nas Makefile
	../tools/nask ipl.nas ipl.bin ipl.lst

helloos.img: ipl.bin Makefile
	../tools/edimg imgin:../tools/fdimg0at.tek \
		wbinimg src:ipl.bin len:512 from:0 to:0 imgout:helloos.img

#指令
asm:
	make -r ipl.bin

img:
	make -r helloos.img

run:
	echo "Please run the disk in Virtualbox"

clean:
	
	-rm ipl.bin
	-rm ipl.lst

src_only:
	make clean
	-rm helloos.img
