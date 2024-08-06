#include "fat32.hpp"
#include "../kernel/kernel.hpp"
#include "../kernel/pci.hpp"
#include "../shell/out.hpp"
#include "../memory/mem.hpp"
#include "../lib/standard.hpp"

FAT32::FAT32(int driver) {
	this->first_data_sector = 0;
	this->root_dir_start = 0;
	this->total_sectors = 0;
	this->total_clusters = 0;
	this->cluster_size = 0;
	this->fat_type = T_FAT32;
	this->partition_start = 0;

	temp_buffer1 = (uint8_t *)global_allocator.request_page();
	temp_buffer2 = (uint8_t *)global_allocator.request_page();

	port = ((AHCI::AHCIDriver *)(PCI::get_ahci_driver(driver)))->disk_drive;

	//uint64_t *mbr = &_KernelEnd;
	port->buffer = (uint8_t *)global_allocator.request_page();
	memset(port->buffer, 0, 0x1000);

	boot_sector = (Fat32BootSector *)port->buffer;//(Fat32BootSector *)malloc(512);//(Fat32BootSector *)&_KernelEnd;

	if(!port->read(partition_start, 1, boot_sector)) return;
	if(port->buffer[510] != 0x55 || port->buffer[511] != 0xAA) {
		out::set_color(0xFFFF0000);
		out::println("0x55AA Magic not found. Wrong drive device?");
		out::set_color(0xFF000000);
		return;
	}

	root_dir_sector_count = (boot_sector->root_entry_count * 32) + (boot_sector->bytes_per_sector - 1) / boot_sector->bytes_per_sector; //Should be zero in FAT32

	//First sector of cluster 2 (0x5C2 or 0xB8400 in hexeditors)
	first_data_sector = boot_sector->table_count * boot_sector->extended_section.table_size_32 + boot_sector->reserved_sector_count + root_dir_sector_count;
	root_dir_start = boot_sector->extended_section.root_cluster;
	
	if(boot_sector->total_sectors_16 == 0) {
		total_sectors = boot_sector->total_sectors_32;
	}else {
		total_sectors = boot_sector->total_sectors_16;
	}

	total_clusters = total_sectors / boot_sector->sectors_per_cluster;
	cluster_size = boot_sector->sectors_per_cluster * boot_sector->bytes_per_sector;

	fat_cache = (uint32_t *)global_allocator.request_page();//(uint32_t *)malloc(boot_sector->extended_section.table_size_32 * boot_sector->bytes_per_sector);
	port->read(partition_start + boot_sector->reserved_sector_count, boot_sector->extended_section.table_size_32, fat_cache);

	uint8_t *buffer1 = (uint8_t *)global_allocator.request_page();
	memset(buffer1, 0, 0x1000);

	port->read(first_data_sector, 4, buffer1);
	Fat32DirectoryEntry *file = (Fat32DirectoryEntry *)(buffer1 + 32);

	uint8_t *buffer2 = (uint8_t *)global_allocator.request_page();
	memset(buffer2, 0, 0x1000);

	uint32_t sector0 = get_sector_number(get_cluster_number(file));
	port->read(sector0, 1, buffer2);
	for(int t = 0; t < 512; t++) {
		out::cprint(buffer2[t]);
	}

	uint32_t sector = get_sector_number(read_fat_entry(get_cluster_number(file)));//first_data_sector + (read_fat_entry(get_cluster_number(file)) - 2) * boot_sector->sectors_per_cluster;
	port->read(sector, 1, buffer2);
	for(int t = 0; t < 512; t++) {
		out::cprint(buffer2[t]);
	}

	global_allocator.free_page(buffer2);

	tree();

	if(total_sectors == 0) fat_type = T_ExFAT;
	else if(total_clusters < 4085) fat_type = T_FAT12;
	else if(total_clusters < 65525) fat_type = T_FAT16;
	else fat_type = T_FAT32;
}

void FAT32::read_cluster(uint64_t cluster, void *buffer) {
	port->read(get_sector_number(cluster), 1, buffer);
}

uint32_t FAT32::get_sector_number(uint32_t cluster) {
	//uint32_t cluster_number = ((uint32_t)entry->cluster_high << 16) | entry->cluster_low;
	return first_data_sector + (cluster - 2) * boot_sector->sectors_per_cluster;
}

uint32_t FAT32::get_cluster_number(Fat32DirectoryEntry *entry) {
	return ((uint32_t)entry->cluster_high << 16) | entry->cluster_low;
}

//doesnt work yet, itll loop on itself lol
void FAT32::tree() {
	uint8_t *buffer = (uint8_t *)global_allocator.request_page();
	memset(buffer, 0, 0x1000);
	port->read(first_data_sector, 4, buffer);
	Fat32DirectoryEntry *file = (Fat32DirectoryEntry *)buffer;

	int64_t depth = 0;
	uint64_t current_sector = first_data_sector;
	uint64_t last_sector = 0;
	uint64_t last_last_sector = 0;

	out::newline();

	for(int t = 0; t < 24; t++) {
		if(*(uint64_t *)file == 0 && depth >= 0) {
			current_sector = last_sector;
			last_sector = last_last_sector;
			port->read(current_sector, 4, buffer);
			depth--;
			continue;
		}
		for(int i = 0; i < depth; i++) {
			out::cprint('|');
		}
		for(int i = 0; i < 8; i++) {
			out::cprint(file->name[i]);
		}
		for(int i = 0; i < 3; i++) {
			out::cprint(file->extension[i]);
		}

		out::print(" ");
		out::print((uint64_t)(get_creation_date_time(file).month));
		out::print("/");
		out::print((uint64_t)(get_creation_date_time(file).day));
		out::print("/");
		out::print((uint64_t)(get_creation_date_time(file).year));
		out::print(" ");
		out::print((uint64_t)(get_creation_date_time(file).hours));
		out::print(":");
		out::print((uint64_t)(get_creation_date_time(file).minutes));
		out::print(":");
		out::print((uint64_t)(get_creation_date_time(file).hours));
		out::newline();

		if(file->attributes == FAT32_DIRECTORY && file->name[0] != '.') {
			depth++;
			last_last_sector = last_sector;
			last_sector = current_sector;
			current_sector = get_sector_number(get_cluster_number(file));
			port->read(get_sector_number(get_cluster_number(file)), 4, buffer);
		}

		buffer += 32;
		file = (Fat32DirectoryEntry *)buffer;
		//if(*(uint64_t *)file == 0 && depth == 0) return;
	}
}

DateTime FAT32::get_creation_date_time(Fat32DirectoryEntry *entry) {
	DateTime date_time;
	date_time.day = (entry->date_created & 0b0000000000011111);
	date_time.month = (entry->date_created & 0b0000000111100000) >> 5;
	date_time.year = ((entry->date_created & 0b1111111000000000) >> 9) + 1980;
	date_time.seconds = entry->time_created & 0b0000000000011111;
	date_time.minutes = (entry->time_created & 0b0000011111100000) >> 5;
	date_time.hours = (entry->time_created & 0b1111100000000000) >> 11;
	return date_time;
}

//Converts "myfile.txt" to "MYFILE  TXT"
void FAT32::to_dos_filename(char *filename, char *buffer) {
	uint8_t name_count = 0;
	uint8_t extension_count = 0;

	while(*filename != '.') {
		if((*filename>96) && (*filename<123)) *filename ^=0x20;
		*buffer = *filename;

		name_count++;
		filename++;
		buffer++;
	}
	filename++; //skip the '.'
	while(name_count < 8) {
		*buffer = 0x20;
		buffer++;
		name_count++;
	}
	while(*filename != 0) {
		if((*filename>96) && (*filename<123)) *filename ^=0x20;
		*buffer = *filename;

		extension_count++;
		filename++;
		buffer++;
	}
	while(extension_count < 8) {
		*buffer = 0x20;
		buffer++;
		extension_count++;
	}
}

File FAT32::fat_directory(char *directory_name) {
	File file;
	unsigned char *buf;
	Fat32DirectoryEntry *directory;

	char dos_filename[11];
	to_dos_filename(directory_name, dos_filename);
	dos_filename[11] = 0;

	for(int sector = 0; sector < 14; sector++) {
		port->read(root_dir_start + sector, 1, buf);
		directory = (Fat32DirectoryEntry *)buf;

		//16 entries per sector
		for(int i = 0; i < 16; i++) {
			char name[11];
			memcopy((void *)name, directory->name, 8);
			memcopy((void *)name[9], directory->extension, 3);
			name[11] = 0;

			if(compare_string(dos_filename, name) == 0) {
				strcopy(file.name, directory_name);
				file.id = 0;
				file.current_cluster = directory->cluster_low;
				file.eof = 0;
				file.file_length = directory->file_size;

				if(directory->attributes == FAT32_DIRECTORY) file.flags = FS_DIRECTORY;
				else file.flags = FS_FILE;

				return file;
			}

			directory++;
		}
	}

	file.flags = FS_INVALID;
	return file;
}

uint32_t FAT32::read_fat_entry(uint32_t cluster) {
	if(cluster < 2 || cluster > total_clusters) return 0;
	uint8_t *buffer = (uint8_t *)global_allocator.request_pages(128);
	uint32_t fat_offset = cluster * 4;
	uint32_t fat_sector = partition_start + boot_sector->reserved_sector_count + (fat_offset / boot_sector->bytes_per_sector);
	uint32_t entry_offset = fat_offset % boot_sector->bytes_per_sector;

	memset(buffer, 0, 128*4096);

	port->read(fat_sector, 32, buffer);
	uint32_t result = *(uint32_t *)(buffer + entry_offset) & 0x0FFFFFFF;
	global_allocator.free_pages(buffer, 128);
	return result;
}

uint32_t FAT32::find_free_cluster() {
    for (uint32_t cluster = 2; cluster < total_clusters; cluster++) {
        if (read_fat_entry(cluster) == 0) {
            return cluster;
        }
    }
    return 0; // No free cluster found
}

void FAT32::write_fat_entry(uint32_t cluster, uint32_t entry) {
    if (cluster < 2 || cluster > total_clusters) return;

	uint8_t *buffer = (uint8_t *)global_allocator.request_pages(128);

	memset(buffer, 0, 128*4096);
    
    // Calculate FAT offset and sector
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = partition_start + boot_sector->reserved_sector_count + (fat_offset / boot_sector->bytes_per_sector);
    uint32_t entry_offset = fat_offset % boot_sector->bytes_per_sector;

    // Read the current FAT sector into a temporary buffer
    port->read(fat_sector, 1, buffer);
    
    // Update the entry in the buffer
    uint32_t *entry_ptr = (uint32_t *)(buffer + entry_offset);
    *entry_ptr = entry & 0x0FFFFFFF;

    // Write the updated sector back to disk
    port->write(fat_sector, 1, buffer);

	global_allocator.free_pages(buffer, 128);
}