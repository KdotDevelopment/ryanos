#include "efi_memory.hpp"

const char *EFI_MEMORY_TYPE_STRINGS[] {
	"EfiReservedMemoryType",
	"EfiLoaderCode",
	"EfiLoaderData",
	"EfiBootServicesCode",
	"EfiBootServicesData",
	"EfiRuntimeServicesCode",
	"EfiRuntimeServicesData",
	"EfiConventionalMemory", //memory we can use in kernel
	"EfiUnusableMemory",
	"EfiACPIReclaimMemory", //memory we can use after getting ACPI info
	"EfiACPIMemoryNVS",
	"EfiMemoryMappedIO",
	"EfiMemoryMappedIOPortSpace",
	"EfiPalCode"
};