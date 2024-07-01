#pragma once

#include <stdint.h>
#include "../kernel/pci.hpp"

#define HBA_PORT_DEV_PRESENT 0x3
#define HBA_PORT_IPM_ACTIVE 0x1
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_ATA 0x00000101
#define SATA_SIG_SEMB 0xC33C0101
#define SATA_SIG_PM 0x96690101

#define HBA_PXCMD_CR 0x8000
#define HBA_PXCMD_FRE 0x0010
#define HBA_PXCMD_ST 0x0001
#define HBA_PXCMD_FR 0x4000

namespace AHCI {
	enum PortType {
		NONE = 0,
		SATA = 1,
		SEMB = 2,
		PM = 3,
		SATAPI = 4,
	};

	struct HBAPort {
		uint32_t command_list_base;
		uint32_t command_list_base_upper;
		uint32_t fis_base_address;
		uint32_t fis_base_address_upper;
		uint32_t interrupt_status;
		uint32_t interrupt_enable;
		uint32_t command_status;
		uint32_t reserved_0;
		uint32_t task_file_data;
		uint32_t signature;
		uint32_t sata_status;
		uint32_t sata_control;
		uint32_t sata_error;
		uint32_t sata_active;
		uint32_t command_issue;
		uint32_t sata_notification;
		uint32_t fis_switch_control;
		uint32_t reserved_1[11];
		uint32_t vendor[4];
	};

	struct HBAMemory {
		uint32_t host_capability;
		uint32_t global_host_control;
		uint32_t interrupt_status;
		uint32_t ports_implemented;
		uint32_t version;
		uint32_t ccc_control;
		uint32_t ccc_ports;
		uint32_t enclosure_management_location;
		uint32_t enclosure_management_control;
		uint32_t host_capabilities_extended;
		uint32_t bios_handoff_control_status;
		uint8_t reserved_0[0x74];
		uint8_t vendor[0x60];
		HBAPort ports[1];
	};

	struct HBACommandHeader {
		uint8_t command_fis_length:5;
		uint8_t atapi:1;
		uint8_t write:1;
		uint8_t prefetchable:1;

		uint8_t reset:1;
		uint8_t bist:1;
		uint8_t clear_busy:1;
		uint8_t reserved_0:1;
		uint8_t port_multiplier:4;

		uint16_t prdt_length;
		uint32_t prdb_count;
		uint32_t command_table_base_address;
		uint32_t command_table_base_address_upper;
		uint32_t reserved_1[4];
	};

	class Port {
		public:
		HBAPort *hba_port;
		PortType port_type;
		uint8_t *buffer; //direct memory access buffer
		uint8_t port_number;

		void configure();
		void stop_cmd();
		void start_cmd();
	};

	class AHCIDriver {
		public:
		PCI::PCIDeviceHeader *pci_base_address;
		HBAMemory *abar;
		Port *ports[32];
		uint8_t port_count;

		AHCIDriver(PCI::PCIDeviceHeader *pci_base_address);
		void probe_ports();
		~AHCIDriver();
	};
};