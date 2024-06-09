prepare:
	dd if=/dev/zero of=bin/floppy.img bs=512 count=2880

asm:
	nasm -f bin src/bootloader.asm -o bin/bootloader.bin
	nasm -f as86 src/kernel.asm -o bin/kernel_asm.o

library:
	bcc -ansi -c src/std_lib.c -o bin/std_lib.o -I../include
	bcc -ansi -c src/filesystem.c -o bin/filesystem.o -I../include
	bcc -ansi -c src/shell.c -o bin/shell.o -I../include
	bcc -ansi -c src/kernel.c -o bin/kernel_c.o -I../include

link:
	ld86 -o bin/kernel.bin -d bin/kernel_c.o bin/kernel_asm.o bin/std_lib.o bin/filesystem.o bin/shell.o
	dd if=bin/bootloader.bin of=bin/floppy.img bs=512 count=1 conv=notrunc
	dd if=bin/kernel.bin of=bin/floppy.img bs=512 seek=1 conv=notrunc

build: prepare asm library link

run:
# TODO: adjust to your local bochs path
# wsl:
	bochs.exe
# linux:
#	bochs

generate:
	gcc test/generate.c test/testlib -o bin/generate
	bin/generate ${test}
