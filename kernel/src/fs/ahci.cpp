#include "ahci.hpp"
#include "../shell/out.hpp"
#include "../graphics/graphics.hpp"
#include "../memory/mem.hpp"
#include "../memory/page_table_manager.hpp"
#include "../memory/page_frame_allocator.hpp"

AHCI::PortType check_port_type(AHCI::HBAPort *port) {
	uint32_t sata_status = port->sata_status;

	uint8_t interface_power_management = (sata_status >> 8) & 0b111;
	uint8_t device_detection = sata_status & 0b111;

	if(device_detection != HBA_PORT_DEV_PRESENT) return AHCI::PortType::NONE;
	if(interface_power_management != HBA_PORT_IPM_ACTIVE) return AHCI::PortType::NONE;

	switch(port->signature) {
		case SATA_SIG_ATAPI: return AHCI::PortType::SATAPI;
		case SATA_SIG_ATA: return AHCI::PortType::SATA;
		case SATA_SIG_SEMB: return AHCI::PortType::SEMB;
		case SATA_SIG_PM: return AHCI::PortType::PM;
		default: return AHCI::PortType::NONE;
	}
}

void AHCI::Port::configure() {
	stop_cmd();

	out::println("test");
	graphics->swap();

	void *new_base = global_allocator.request_page();
	hba_port->command_list_base = (uint32_t)(uint64_t)new_base; //huh?
	hba_port->command_list_base_upper = (uint32_t)((uint64_t)new_base >> 32);
	memset((void *)(hba_port->command_list_base), 0, 1024);

	void *fis_base = global_allocator.request_page();
	hba_port->fis_base_address = (uint32_t)(uint64_t)fis_base;
	hba_port->fis_base_address_upper = (uint32_t)((uint64_t)fis_base >> 32);
	memset(fis_base, 0, 256);

	HBACommandHeader *command_header = (HBACommandHeader *)((uint64_t)hba_port->command_list_base + ((uint64_t)hba_port->command_list_base_upper << 32));

	for(int i = 0; i < 32; i++) {
		command_header[i].prdt_length = 8;

		void *command_table_address = global_allocator.request_page();
		uint64_t address = (uint64_t)command_table_address + (i << 8);
		command_header[i].command_table_base_address = (uint32_t)address;
		command_header[i].command_table_base_address_upper = (uint32_t)((uint64_t)address >> 32);
		memset(command_table_address, 0, 256);
	}

	start_cmd();
}

void AHCI::Port::stop_cmd() {
	hba_port->command_status &= ~HBA_PXCMD_ST;
	hba_port->command_status &= ~HBA_PXCMD_FRE;

	while(true) {
		if(hba_port->command_status & HBA_PXCMD_FR) continue;
		if(hba_port->command_status & HBA_PXCMD_CR) continue;
		break;
	}
}

void AHCI::Port::start_cmd() {
	while(hba_port->command_status & HBA_PXCMD_CR);

	hba_port->command_status |= HBA_PXCMD_FRE;
	hba_port->command_status |= HBA_PXCMD_ST;
}

AHCI::AHCIDriver::AHCIDriver(PCI::PCIDeviceHeader *pci_base_address) {
	this->pci_base_address = pci_base_address;

	abar = (HBAMemory *)((PCI::PCIHeader0 *)pci_base_address)->bar5;

	page_table_manager.map_memory(abar, abar);
	probe_ports();

	for(int i = 0; i < port_count; i++) {
		Port *port = ports[i];
		port->configure();
	}
}

void AHCI::AHCIDriver::probe_ports() {
	uint32_t ports_implemented = abar->ports_implemented;
	for(int i = 0; i < 32; i++) {
		if(ports_implemented & (1 << i)) { //port is active
			PortType port_type = check_port_type(&abar->ports[i]);

			if(port_type == PortType::SATA || port_type == PortType::SATAPI) {
				ports[port_count] = new Port();
				ports[port_count]->port_type = port_type;
				ports[port_count]->hba_port = &abar->ports[i];
				ports[port_count]->port_number = port_count;
				port_count++;
			}
		}
	}
}

AHCI::AHCIDriver::~AHCIDriver() {

}