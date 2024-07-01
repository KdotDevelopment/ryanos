#pragma once

#include <stdint.h>
#include "acpi.hpp"

namespace PCI {
	struct PCIDeviceHeader {
		uint16_t vendor_id;
		uint16_t device_id;
		uint16_t command;
		uint16_t status;
		uint8_t revision_id;
		uint8_t program_interface;
		uint8_t subclass;
		uint8_t Class; //AWWW WHAT class is already defined...
		uint8_t cache_line_size;
		uint8_t latency_timer;
		uint8_t header_type;
		uint8_t bist;
	};

	struct PCIHeader0 {
		PCIDeviceHeader header;
		uint32_t bar0;
		uint32_t bar1;
		uint32_t bar2;
		uint32_t bar3;
		uint32_t bar4;
		uint32_t bar5;
		uint32_t cardbus_cis_ptr;
		uint16_t subsystem_vendor_id;
		uint16_t subsystem_id;
		uint32_t expansion_rom_base_address;
		uint8_t capabilities_ptr;
		uint8_t reserved_0;
		uint16_t reserved_1;
		uint32_t reserved_2;
		uint8_t interrupt_line;
		uint8_t interrupt_pin;
		uint8_t min_grant;
		uint8_t max_latency;
	};

	void enumerate_pci(ACPI::MCFGHeader *mcfg);

	extern const char *device_classes[];

	const char *get_vendor_name(uint16_t vendor_id);
	const char *get_device_name(uint16_t vendor_id, uint16_t device_id);
	const char *get_subclass_name(uint8_t class_code, uint8_t subclass_code);
	const char *get_program_interface_name(uint8_t class_code, uint8_t subclass_code, uint8_t prog_interface);
};