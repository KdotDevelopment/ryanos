#pragma once

#include "../graphics/graphics.hpp"
#include "../graphics/framebuffer.hpp"
#include "../graphics/font.hpp"
#include "acpi.hpp"
#include "efi_memory.hpp"

struct boot_info_t {
	Framebuffer *framebuffer;
	psf1_font_t *font;
	EFI_MEMORY_DESCRIPTOR *m_map;
	uint64_t m_map_size;
	uint64_t m_map_descriptor_size;
	ACPI::RSDP2 *rsdp;
};