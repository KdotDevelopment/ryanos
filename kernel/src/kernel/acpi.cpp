#include "acpi.hpp"

void *ACPI::find_table(SDTHeader *sdt_header, char *signature) {
	int entries = (sdt_header->length - sizeof(ACPI::SDTHeader)) / 8;
	for(int t = 0; t < entries; t++) {
		ACPI::SDTHeader *new_sdt_header = (ACPI::SDTHeader *)*(uint64_t *)((uint64_t)sdt_header + sizeof(ACPI::SDTHeader)+(t*8));
		for(int i = 0; i < 4; i++) {
			if(new_sdt_header->signature[i] != signature[i]) break;
			if(i == 3) return new_sdt_header;
		}
	}
	return 0;
}