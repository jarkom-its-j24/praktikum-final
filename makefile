prepare:
	dd if=/dev/zero of=bin/floppy.img bs=512 count=2880

bootloader:
	nasm -f bin src/bootloader.asm -o bin/bootloader.bin

library:
	bcc -ansi -c src/std_lib.c -o bin/std_lib.o -Iinclude
	bcc -ansi -c src/filesystem.c -o bin/filesystem.o -Iinclude
	bcc -ansi -c src/shell.c -o bin/shell.o -Iinclude

kernel:
	nasm -f as86 src/kernel.asm -o bin/kernel_asm.o
	bcc -ansi -c src/kernel.c -o bin/kernel_c.o -Iinclude

link:
	ld86 -o bin/kernel.bin -d bin/kernel_c.o bin/kernel_asm.o bin/std_lib.o bin/filesystem.o bin/shell.o
	dd if=bin/bootloader.bin of=bin/floppy.img bs=512 count=1 conv=notrunc
	dd if=bin/kernel.bin of=bin/floppy.img bs=512 seek=1 conv=notrunc

build: prepare bootloader library kernel link

run:
# TODO: adjust to your local bochs path
# wsl:
	bochs.exe
# linux:
#	bochs
