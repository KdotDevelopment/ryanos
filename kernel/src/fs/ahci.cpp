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

	void *new_base = global_allocator.request_page();
	hba_port->command_list_base = (uint32_t)(uint64_t)new_base;
	hba_port->command_list_base_upper = (uint32_t)((uint64_t)new_base >> 32);
	memset((void *)(uint64_t)(hba_port->command_list_base), 0, 1024);

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

bool AHCI::Port::read(uint64_t sector, uint32_t sector_count, void *buffer) {
	uint64_t spin = 0; //timeout in case port gets stuck

	while((hba_port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) { //make sure we dont issue command while another is running (on another thread)
		spin++;
	}
	if(spin == 1000000) {
		return false; //port is hung!
	}

	uint32_t sector_low = (uint32_t) sector;
	uint32_t sector_high= (uint32_t) (sector >> 32);

	hba_port->interrupt_status = (uint32_t)-1; //clear pending interrupt bits

	HBACommandHeader *cmd_header = (HBACommandHeader *)(uint64_t)hba_port->command_list_base;
	cmd_header->command_fis_length = sizeof(FIS_REG_H2D) / sizeof(uint32_t); //command FIS size
	cmd_header->write = 0; //read
	cmd_header->prdt_length = 1;

	HBACommandTable *command_table = (HBACommandTable *)(uint64_t)(cmd_header->command_table_base_address);
	memset(command_table, 0, sizeof(HBACommandTable) + (cmd_header->prdt_length - 1) * sizeof(HBAPRDTEntry));

	command_table->prdt_entry[0].data_base_address = (uint32_t)(uint64_t)buffer;
	command_table->prdt_entry[0].data_base_address_upper = (uint32_t)((uint64_t)buffer >> 32);
	command_table->prdt_entry[0].byte_count = (sector_count << 9) - 1; //512 bytes per sector
	command_table->prdt_entry[0].interrupt_on_completion = 1;

	FIS_REG_H2D *cmd_fis = (FIS_REG_H2D *)&command_table->command_fis;
	cmd_fis->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis->command_control = 1; //command
	cmd_fis->command = ATA_CMD_READ_DMA_EX;

	cmd_fis->lba0 = (uint8_t)(sector_low);
	cmd_fis->lba1 = (uint8_t)(sector_low >> 8);
	cmd_fis->lba2 = (uint8_t)(sector_low >> 16);
	cmd_fis->lba3 = (uint8_t)(sector_high);
	cmd_fis->lba4 = (uint8_t)(sector_high >> 8);
	cmd_fis->lba5 = (uint8_t)(sector_high >> 16);

	cmd_fis->device_register = 1 << 6; //LBA mode

	cmd_fis->count_low = sector_count & 0xFF;
	cmd_fis->count_high = (sector_count >> 8) & 0xFF;

	hba_port->command_issue = 1; //tell command to run

	while(true) {
		if((hba_port->command_issue == 0)) break; //command is done
		if(hba_port->interrupt_status & HBA_PXIS_TFES) {
			return false; //read unsuccessful due to error
		}
	}

	return true;
}

bool AHCI::Port::write(uint64_t sector, uint32_t sector_count, void *buffer) {
	uint64_t spin = 0; //timeout in case port gets stuck

	while((hba_port->task_file_data & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000) { //make sure we dont issue command while another is running (on another thread)
		spin++;
	}
	if(spin == 1000000) {
		return false; //port is hung!
	}

	uint32_t sector_low = (uint32_t) sector;
	uint32_t sector_high= (uint32_t) (sector >> 32);

	hba_port->interrupt_status = (uint32_t)-1; //clear pending interrupt bits

	HBACommandHeader *cmd_header = (HBACommandHeader *)(uint64_t)hba_port->command_list_base;
	cmd_header->command_fis_length = sizeof(FIS_REG_H2D) / sizeof(uint32_t); //command FIS size
	cmd_header->write = 1; //write
	cmd_header->prdt_length = 1;

	HBACommandTable *command_table = (HBACommandTable *)(uint64_t)(cmd_header->command_table_base_address);
	memset(command_table, 0, sizeof(HBACommandTable) + (cmd_header->prdt_length - 1) * sizeof(HBAPRDTEntry));

	command_table->prdt_entry[0].data_base_address = (uint32_t)(uint64_t)buffer;
	command_table->prdt_entry[0].data_base_address_upper = (uint32_t)((uint64_t)buffer >> 32);
	command_table->prdt_entry[0].byte_count = (sector_count << 9) - 1; //512 bytes per sector
	command_table->prdt_entry[0].interrupt_on_completion = 1;

	FIS_REG_H2D *cmd_fis = (FIS_REG_H2D *)&command_table->command_fis;
	cmd_fis->fis_type = FIS_TYPE_REG_H2D;
	cmd_fis->command_control = 1; //command
	cmd_fis->command = ATA_CMD_WRITE_DMA_EX;

	cmd_fis->lba0 = (uint8_t)(sector_low);
	cmd_fis->lba1 = (uint8_t)(sector_low >> 8);
	cmd_fis->lba2 = (uint8_t)(sector_low >> 16);
	cmd_fis->lba3 = (uint8_t)(sector_high);
	cmd_fis->lba4 = (uint8_t)(sector_high >> 8);
	cmd_fis->lba5 = (uint8_t)(sector_high >> 16);

	cmd_fis->device_register = 1 << 6; //LBA mode

	cmd_fis->count_low = sector_count & 0xFF;
	cmd_fis->count_high = (sector_count >> 8) & 0xFF;

	hba_port->command_issue = 1; //tell command to run

	while(true) {
		if((hba_port->command_issue == 0)) break; //command is done
		if(hba_port->interrupt_status & HBA_PXIS_TFES) {
			return false; //read unsuccessful due to error
		}
	}

	return true;
}

AHCI::AHCIDriver::AHCIDriver(PCI::PCIDeviceHeader *pci_base_address) {
	this->pci_base_address = pci_base_address;

	abar = (HBAMemory *)(uint64_t)((PCI::PCIHeader0 *)pci_base_address)->bar5;

	page_table_manager.map_memory(abar, abar);
	probe_ports();

	for(int i = 0; i < port_count; i++) {
		Port *port = ports[i];
		port->configure();

		if(i == 0) { //this is the disk drive that is attached via qemu -hba ###.img
			disk_drive = port;
		}

		//port->buffer = (uint8_t *)global_allocator.request_page();
		//memset(port->buffer, 64, 0x1000);

		//port->write(12, 4, port->buffer);

		/*port->buffer = (uint8_t *)global_allocator.request_page();
		memset(port->buffer, 0, 0x1000);

		port->read(0, 4, port->buffer);
		for(int t = 0; t < 1024; t++) {
			out::cprint(port->buffer[t]);
		}
		out::newline();*/
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
				if(port_type == PortType::SATA) {
					disk_drive = ports[port_count];
				}
				port_count++;
			}
		}
	}
}

AHCI::AHCIDriver::~AHCIDriver() {

}