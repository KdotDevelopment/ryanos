#include "kernel.hpp"
#include "efi_memory.hpp"
#include "pci.hpp"
#include "io.hpp"
#include "../shell/shell.hpp"
#include "../shell/out.hpp"
#include "../lib/cstr.hpp"
#include "../fs/fat32.hpp"
#include "../fs/ahci.hpp"
#include "../gdt/gdt.hpp"
#include "../memory/page_frame_allocator.hpp"
#include "../memory/page_table_manager.hpp"
#include "../memory/page_map_indexer.hpp"
#include "../memory/paging.hpp"
#include "../memory/mem.hpp"
#include "interrupts/interrupts.hpp"
#include "interrupts/pit/pit.hpp"
#include "interrupts/idt.hpp"
#include <stdint.h>

void prepare_memory(boot_info_t *boot_info) {
	uint64_t m_map_entries = boot_info->m_map_size / boot_info->m_map_descriptor_size;

	global_allocator = PageFrameAllocator();
	global_allocator.read_efi_memory_map(boot_info->m_map, boot_info->m_map_size, boot_info->m_map_descriptor_size);

	uint64_t kernel_size = (uint64_t)&_KernelEnd - (uint64_t)&_KernelStart;
	uint64_t kernel_pages = (uint64_t)kernel_size / 4096 + 1;

	global_allocator.lock_pages(&_KernelStart, kernel_pages);

	//init_dynamic_mem();

	//allocator.lock_pages(graphics.backbuffer, (graphics.get_width() * graphics.get_height() * 4) / 4096 + 1);

	PageTable *PML4 = (PageTable *)global_allocator.request_page();
	memset(PML4, 0, 0x1000);

	page_table_manager = PageTableManager(PML4);

	for(uint64_t i = 0; i < get_memory_size(boot_info->m_map, m_map_entries, boot_info->m_map_descriptor_size); i += 0x1000) {
		page_table_manager.map_memory((void *)i, (void *)i); //virtual memory = physical memory
	}

	uint64_t fb_base = (uint64_t)boot_info->framebuffer->base_address;
	uint64_t fb_size = (uint64_t)boot_info->framebuffer->buffer_size + 0x1000; //add extra page for padding
	global_allocator.lock_pages((void *)fb_base, fb_size / 4096 + 1); //locks framebuffer so it can't be overriden
	for(uint64_t i = fb_base; i < fb_base + fb_size; i += 4096) {
		page_table_manager.map_memory((void *)i, (void *)i); //virtual framebuffer = physical framebuffer
	}

	asm("mov %0, %%cr3" : : "r" (PML4)); //putting PML4 into cr3 register
}

IDTR idtr;
void set_idt_gate(void* handler, uint8_t entry_offset, uint8_t type_attr, uint8_t selector){
    IDTDescEntry* interrupt = (IDTDescEntry*)(idtr.offset + entry_offset * sizeof(IDTDescEntry));
    interrupt->set_offset((uint64_t)handler);
    interrupt->type_attr = type_attr;
    interrupt->selector = selector;
}

void prepare_interrupts() {
	idtr.limit = 0x0FFF;
	idtr.offset = (uint64_t)global_allocator.request_page();

	set_idt_gate((void*)page_fault_handler, 0xE, IDT_TA_INTERRUPT_GATE, 0x08);
    set_idt_gate((void*)double_fault_handler, 0x8, IDT_TA_INTERRUPT_GATE, 0x08);
    set_idt_gate((void*)gp_fault_handler, 0xD, IDT_TA_INTERRUPT_GATE, 0x08);
    set_idt_gate((void*)keyboard_int_handler, 0x21, IDT_TA_INTERRUPT_GATE, 0x08);
	set_idt_gate((void*)mouse_int_handler, 0x2C, IDT_TA_INTERRUPT_GATE, 0x08);
    set_idt_gate((void*)pit_int_handler, 0x20, IDT_TA_INTERRUPT_GATE, 0x08);

	asm("lidt %0" : : "m" (idtr));

	remap_pic();

	asm("sti");
}

void prepare_acpi(boot_info_t *boot_info) {
	ACPI::SDTHeader *xsdt = (ACPI::SDTHeader *)(boot_info->rsdp->xsdt_address);

	ACPI::MCFGHeader *mcfg = (ACPI::MCFGHeader *)ACPI::find_table(xsdt, (char *)"MCFG");

	PCI::enumerate_pci(mcfg);
}

Graphics temp_graphics = Graphics(NULL, NULL);
Shell temp_shell = Shell();
extern "C" void _start(boot_info_t *boot_info) {
	GDTDescriptor gdt_descriptor;
	gdt_descriptor.size = sizeof(GDT) - 1;
	gdt_descriptor.offset = (uint64_t)&default_gdt;

	load_gdt(&gdt_descriptor); //function defined in ASM

	prepare_memory(boot_info);

	init_heap((void *)0x0000100000000000, (50 * 1024 * 1024) / 4096); //50 MiB //0x0000100000000000

	prepare_interrupts();

	temp_graphics = Graphics(boot_info->framebuffer, boot_info->font);
	graphics = &temp_graphics;

	PIT::set_divisor(2983*8); //resonable precision, 1193182 / 2983 = ~399.993966, this means ~400 ticks a second

	temp_shell = Shell();
	shell = &temp_shell;
	shell->init_mouse();

	outb(PIC1_DATA, 0b11111000);
	outb(PIC2_DATA, 0b11101101);

	shell->init_shell();

	//prepare_acpi(boot_info);

	/*AHCI::Port *port = ((AHCI::AHCIDriver *)(PCI::get_ahci_driver(0)))->disk_drive; //gets the main hard drive
	port->buffer = (uint8_t *)global_allocator.request_page();
	memset(port->buffer, 0, 0x1000);

	port->read(12, 4, port->buffer);
	for(int t = 0; t < 1024; t++) {
		out::cprint(port->buffer[t]);
	}*/

	//FAT32 fs(0);

	shell->start_loop();

	while(true);
}