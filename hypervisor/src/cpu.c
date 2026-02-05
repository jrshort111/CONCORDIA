#include "cpu.h"
#include "console.h"
#include "types.h"

#define MSR_EFER 0xC0000080
#define MSR_STAR 0xC0000081
#define MSR_LSTAR 0xC0000082
#define MSR_CSTAR 0xC0000083
#define MSR_SFMASK 0xC0000084
#define MSR_GSBASE 0xC0000101
#define MSR_KERNELGSBASE 0xC0000102
#define MSR_APIC_BASE 0x1B
#define IA32_FEATURE_CONTROL 0x3A

#define APIC_ID_REG 0x20
#define CPUID_FEATURES 0x1
#define CPUID_EXTENDED 0x80000001

static cpu_info_t cpu_list[MAX_CPUS];
static uint32_t cpu_count = 0;

static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static inline void write_msr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = (value >> 32) & 0xFFFFFFFF;
    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

static inline void cpuid(uint32_t leaf, uint32_t *eax, uint32_t *ebx, 
                         uint32_t *ecx, uint32_t *edx) {
    asm volatile("cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(0));
}

static inline void sgdt(gdt_descriptor_t *desc) {
    asm volatile("sgdt %0" : "=m"(*desc));
}

static inline void lgdt(const gdt_descriptor_t *desc) {
    asm volatile("lgdt %0" : : "m"(*desc));
}

static inline void lidt(const gdt_descriptor_t *desc) {
    asm volatile("lidt %0" : : "m"(*desc));
}

void cpu_detect_cores(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // Get CPUID leaf 0x0B (Extended Topology Enumeration)
    cpuid(0x0, &eax, &ebx, &ecx, &edx);
    
    if (eax < 0x0B) {
        console_write_string("WARNING: CPUID leaf 0x0B not supported, using basic detection\n");
        // Fallback: detect from CPUID 1
        cpuid(CPUID_FEATURES, &eax, &ebx, &ecx, &edx);
        uint32_t max_cores = (ebx >> 16) & 0xFF;
        cpu_count = (max_cores > 0) ? max_cores : 1;
        return;
    }
    
    // Use leaf 0x0B for more accurate core detection
    cpuid(0x0B, &eax, &ebx, &ecx, &edx);
    uint32_t num_threads = ebx & 0xFFFF;
    cpu_count = (num_threads > 0) ? num_threads : 1;
}

uint32_t cpu_get_apic_id(void) {
    uint32_t eax, ebx, ecx, edx;
    cpuid(CPUID_FEATURES, &eax, &ebx, &ecx, &edx);
    return (ebx >> 24) & 0xFF;
}

uint8_t cpu_is_linux_core(uint32_t apic_id) {
    return (apic_id >= LINUX_CORES_START && apic_id <= LINUX_CORES_END) ? 1 : 0;
}

void cpu_enable_features(void) {
    uint32_t eax, ebx, ecx, edx;
    
    // Check for SSE
    cpuid(CPUID_FEATURES, &eax, &ebx, &ecx, &edx);
    if (edx & (1 << 25)) {
        uint64_t cr4;
        asm volatile("mov %%cr4, %0" : "=r"(cr4));
        cr4 |= (1 << 9);  // CR4.OSFXSR
        asm volatile("mov %0, %%cr4" : : "r"(cr4));
        console_write_string("SSE enabled\n");
    }
    
    // Check for AVX
    if (ecx & (1 << 28)) {
        uint64_t cr4;
        asm volatile("mov %%cr4, %0" : "=r"(cr4));
        cr4 |= (1 << 18);  // CR4.OSXSAVE
        asm volatile("mov %0, %%cr4" : : "r"(cr4));
        
        // Enable AVX in XCR0
        uint32_t low, high;
        asm volatile("xgetbv" : "=a"(low), "=d"(high) : "c"(0));
        low |= (1 << 1) | (1 << 2);  // Enable XMM and YMM
        asm volatile("xsetbv" : : "a"(low), "d"(high), "c"(0));
        console_write_string("AVX enabled\n");
    }
    
    // Check for NX bit
    cpuid(0x80000001, &eax, &ebx, &ecx, &edx);
    if (edx & (1 << 20)) {
        uint64_t efer = read_msr(MSR_EFER);
        efer |= (1 << 11);  // NXE
        write_msr(MSR_EFER, efer);
        console_write_string("NX bit enabled\n");
    }
}

void cpu_setup_gdt(void) {
    // GDT descriptors (already set in boot.s, but ensure they're correct)
    // 0x00: Null descriptor
    // 0x08: 64-bit code segment
    // 0x10: 64-bit data segment
    // 0x18: TSS (Task State Segment) - will be set up later
    console_write_string("GDT already set in boot code\n");
}

void cpu_setup_idt(void) {
    // Create IDT for basic exception handling
    // For now, this is a placeholder
    console_write_string("IDT setup: TODO\n");
}

void cpu_init(void) {
    console_write_string("Initializing CPU subsystem...\n");
    
    // Detect number of CPU cores
    cpu_detect_cores();
    char buf[32];
    itoa(cpu_count, buf, 10);
    console_write_string("  Cores detected: ");
    console_write_string(buf);
    console_write_string("\n");
    
    // Get current APIC ID
    uint32_t apic_id = cpu_get_apic_id();
    console_write_string("  Current APIC ID: ");
    itoa(apic_id, buf, 10);
    console_write_string(buf);
    if (cpu_is_linux_core(apic_id)) {
        console_write_string(" (Linux Cell)\n");
    } else {
        console_write_string(" (Windows Cell)\n");
    }
    
    // Enable CPU features
    console_write_string("Enabling CPU features...\n");
    cpu_enable_features();
    
    // Setup GDT
    cpu_setup_gdt();
    
    // Setup IDT
    cpu_setup_idt();
    
    console_write_string("CPU initialization complete\n");
}
