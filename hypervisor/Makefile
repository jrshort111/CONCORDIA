.PHONY: build clean run test

ARCH := x86_64
TARGET := $(ARCH)-unknown-none
BOOT_ASM := src/boot/boot.s
KERNEL_SRC := src/main.c
CONSOLE_SRC := src/console.c
CPU_SRC := src/cpu.c
MEMORY_SRC := src/memory.c
BUILD_DIR := build
ISO_DIR := $(BUILD_DIR)/iso
KERNEL_BIN := $(BUILD_DIR)/kernel.bin
ISO_IMAGE := $(BUILD_DIR)/concordia.iso

build: $(ISO_IMAGE)

$(KERNEL_BIN): $(BOOT_ASM) $(KERNEL_SRC) $(CONSOLE_SRC) $(CPU_SRC) $(MEMORY_SRC)
	mkdir -p $(BUILD_DIR)
	nasm -f elf64 $(BOOT_ASM) -o $(BUILD_DIR)/boot.o
	gcc -c $(KERNEL_SRC) -o $(BUILD_DIR)/kernel.o -nostdlib -fno-builtin -I src
	gcc -c $(CONSOLE_SRC) -o $(BUILD_DIR)/console.o -nostdlib -fno-builtin -I src
	gcc -c $(CPU_SRC) -o $(BUILD_DIR)/cpu.o -nostdlib -fno-builtin -I src
	gcc -c $(MEMORY_SRC) -o $(BUILD_DIR)/memory.o -nostdlib -fno-builtin -I src
	ld -T linker.ld $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/console.o $(BUILD_DIR)/cpu.o $(BUILD_DIR)/memory.o -o $(KERNEL_BIN)

$(ISO_IMAGE): $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	cp grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR)

run: $(ISO_IMAGE)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 2G -smp 4 -nographic

clean:
	rm -rf $(BUILD_DIR)
