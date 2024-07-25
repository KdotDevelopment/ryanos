#include "fat32.hpp"
#include "../kernel/kernel.hpp"
#include "../kernel/pci.hpp"
#include "../shell/out.hpp"

FAT32::FAT32(int driver) {
	this->first_data_sector = 0;
	this->root_dir_start = 0;
	this->total_sectors = 0;
	this->total_clusters = 0;
	this->cluster_size = 0;
	this->fat_type = T_FAT32;
	this->partition_start = 0;

	port = ((AHCI::AHCIDriver *)(PCI::get_ahci_driver(0)))->disk_drive;

	//uint64_t *mbr = &_KernelEnd;
	port->buffer = (uint8_t *)global_allocator.request_page();
	memset(port->buffer, 0, 0x1000);

	boot_sector = (Fat32BootSector *)port->buffer;//(Fat32BootSector *)malloc(512);//(Fat32BootSector *)&_KernelEnd;

	if(!port->read(partition_start, 4, port->buffer)) return;
	if(port->buffer[510] != 0x55 || port->buffer[511] != 0xAA) {
		out::set_color(0xFFFF0000);
		out::println("0x55AA Magic not found. Wrong drive device?");
		out::set_color(0xFF000000);
		return;
	}
	port->read(0, 1, (void *)boot_sector);
	first_data_sector = boot_sector->table_count * boot_sector->extended_section.table_size_32 + boot_sector->reserved_sector_count;
	root_dir_start = boot_sector->extended_section.root_cluster;
	
	if(boot_sector->total_sectors_16 == 0) {
		total_sectors = boot_sector->total_sectors_32;
	}else {
		total_sectors = boot_sector->total_sectors_16;
	}

	total_clusters = total_sectors / boot_sector->sectors_per_cluster;
	cluster_size = boot_sector->sectors_per_cluster * boot_sector->bytes_per_sector;

	temp_buffer1 = (uint8_t *)global_allocator.request_page();//(uint8_t *)aligned_alloc(0x1000, cluster_size);
	temp_buffer2 = (uint8_t *)global_allocator.request_page();//(uint8_t *)aligned_alloc(0x1000, cluster_size);

	fat_cache = (uint32_t *)global_allocator.request_page();//(uint32_t *)malloc(boot_sector->extended_section.table_size_32 * boot_sector->bytes_per_sector);
	port->read(partition_start + boot_sector->reserved_sector_count, boot_sector->extended_section.table_size_32, fat_cache);

	if(total_sectors == 0) fat_type = T_ExFAT;
	else if(total_clusters < 4085) fat_type = T_FAT12;
	else if(total_clusters < 65525) fat_type = T_FAT16;
	else fat_type = T_FAT32;
}

uint32_t FAT32::read_fat_entry(uint32_t cluster) {
	if(cluster < 2 || cluster > total_clusters) return 0;
	uint32_t fat_offset = cluster * 4;
	uint32_t fat_sector = partition_start + boot_sector->reserved_sector_count + (fat_offset / boot_sector->bytes_per_sector);
	uint32_t entry_offset = fat_offset % boot_sector->bytes_per_sector;

	port->read(fat_sector, 1, temp_buffer1);
	return *(uint32_t *)&temp_buffer1[entry_offset] & 0x0FFFFFFF;
}

uint32_t FAT32::write_fat_entry(uint32_t cluster, uint32_t entry) {
    if (cluster < 2 || cluster > total_clusters) return false;
    
    // Calculate FAT offset and sector
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = partition_start + boot_sector->reserved_sector_count + (fat_offset / boot_sector->bytes_per_sector);
    uint32_t entry_offset = fat_offset % boot_sector->bytes_per_sector;

    // Read the current FAT sector into a temporary buffer
    port->read(fat_sector, 1, temp_buffer1);
    
    // Update the entry in the buffer
    uint32_t* entry_ptr = (uint32_t*)&temp_buffer1[entry_offset];
    *entry_ptr = entry & 0x0FFFFFFF;

    // Write the updated sector back to disk
    return port->write(fat_sector, 1, temp_buffer1);
}