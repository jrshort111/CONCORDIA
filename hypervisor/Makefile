.PHONY: build clean run test

ARCH := x86_64
TARGET := $(ARCH)-unknown-none
BOOT_ASM := src/boot/boot.s
KERNEL_SRC := src/main.c
CONSOLE_SRC := src/console.c
CPU_SRC := src/cpu.c
MEMORY_SRC := src/memory.c
IOMMU_SRC := src/iommu.c
SYSTEM_MANAGER_SRC := src/system_manager.c
INPUT_MANAGER_SRC := src/input_manager.c
MONITOR_SRC := src/monitor.c
DASHBOARD_SRC := src/dashboard.c
KERNEL_LOADER_SRC := src/kernel_loader.c
LINUX_STUB_ASM := stubs/linux_stub.s
WINDOWS_STUB_ASM := stubs/windows_stub.s
BUILD_DIR := build
ISO_DIR := $(BUILD_DIR)/iso
KERNEL_BIN := $(BUILD_DIR)/kernel.bin
ISO_IMAGE := $(BUILD_DIR)/concordia.iso

build: $(ISO_IMAGE)

$(KERNEL_BIN): $(BOOT_ASM) $(KERNEL_SRC) $(CONSOLE_SRC) $(CPU_SRC) $(MEMORY_SRC) $(IOMMU_SRC) $(SYSTEM_MANAGER_SRC) $(INPUT_MANAGER_SRC) $(MONITOR_SRC) $(DASHBOARD_SRC) $(KERNEL_LOADER_SRC) $(LINUX_STUB_ASM) $(WINDOWS_STUB_ASM)
	mkdir -p $(BUILD_DIR)
	# Compile hypervisor boot and kernel modules
	nasm -f elf64 $(BOOT_ASM) -o $(BUILD_DIR)/boot.o
	gcc -c $(KERNEL_SRC) -o $(BUILD_DIR)/kernel.o -nostdlib -fno-builtin -I src
	gcc -c $(CONSOLE_SRC) -o $(BUILD_DIR)/console.o -nostdlib -fno-builtin -I src
	gcc -c $(CPU_SRC) -o $(BUILD_DIR)/cpu.o -nostdlib -fno-builtin -I src
	gcc -c $(MEMORY_SRC) -o $(BUILD_DIR)/memory.o -nostdlib -fno-builtin -I src
	gcc -c $(IOMMU_SRC) -o $(BUILD_DIR)/iommu.o -nostdlib -fno-builtin -I src
	gcc -c $(SYSTEM_MANAGER_SRC) -o $(BUILD_DIR)/system_manager.o -nostdlib -fno-builtin -I src
	gcc -c $(INPUT_MANAGER_SRC) -o $(BUILD_DIR)/input_manager.o -nostdlib -fno-builtin -I src
	gcc -c $(MONITOR_SRC) -o $(BUILD_DIR)/monitor.o -nostdlib -fno-builtin -I src
	gcc -c $(DASHBOARD_SRC) -o $(BUILD_DIR)/dashboard.o -nostdlib -fno-builtin -I src
	gcc -c $(KERNEL_LOADER_SRC) -o $(BUILD_DIR)/kernel_loader.o -nostdlib -fno-builtin -I src
	# Compile stub kernels as raw 64-bit binaries
	nasm -f bin $(LINUX_STUB_ASM) -o $(BUILD_DIR)/linux_stub.bin
	nasm -f bin $(WINDOWS_STUB_ASM) -o $(BUILD_DIR)/windows_stub.bin
	# Link hypervisor kernel
	ld -T linker.ld $(BUILD_DIR)/boot.o $(BUILD_DIR)/kernel.o $(BUILD_DIR)/console.o $(BUILD_DIR)/cpu.o $(BUILD_DIR)/memory.o $(BUILD_DIR)/iommu.o $(BUILD_DIR)/system_manager.o $(BUILD_DIR)/input_manager.o $(BUILD_DIR)/monitor.o $(BUILD_DIR)/dashboard.o $(BUILD_DIR)/kernel_loader.o -o $(KERNEL_BIN)

$(ISO_IMAGE): $(KERNEL_BIN)
	mkdir -p $(ISO_DIR)/boot/grub
	mkdir -p $(ISO_DIR)/boot/stubs
	cp $(KERNEL_BIN) $(ISO_DIR)/boot/
	cp $(BUILD_DIR)/linux_stub.bin $(ISO_DIR)/boot/stubs/
	cp $(BUILD_DIR)/windows_stub.bin $(ISO_DIR)/boot/stubs/
	cp grub.cfg $(ISO_DIR)/boot/grub/
	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR)

run: $(ISO_IMAGE)
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 2G -smp 4 -nographic

clean:
	rm -rf $(BUILD_DIR)
