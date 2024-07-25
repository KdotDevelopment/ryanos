#pragma once

#include <stdint.h>
#include <stddef.h>
#include "ahci.hpp"

#define FAT32_LFN_CHARS_PER 13
#define FAT32_LFN_MAX 20
#define FAT32_LFN_MAX_TOTAL_CHARS 256
#define FAT32_LFN_ORDER_FINAL 0x40

#define FAT32_READ_ONLY 0x01
#define FAT32_HIDDEN 0x02
#define FAT32_SYSTEM 0x04
#define FAT32_VOLUME_ID 0x08
#define FAT32_DIRECTORY 0x10
#define FAT32_ARCHIVE 0x20
#define FAT32_LFN (FAT32_READ_ONLY | FAT32_HIDDEN | FAT32_SYSTEM | FAT32_VOLUME_ID)

#define FAT32_CACHE_MAX 32
#define FAT32_CACHE_BAD 0xFFFFFFFF

typedef struct {
	char name[8];
	char extension[3];
	uint8_t attributes; //READ_ONLY=0x01 HIDDEN=0x02 SYSTEM=0x04 VOLUME_ID=0x08 DIRECTORY=0x10 ARCHIVE=0x20 LFN=READ_ONLY|HIDDEN|SYSTEM|VOLUME_ID 
	uint8_t reserved;
	uint8_t creation_seconds; //0-199
	uint16_t time_created; //5 bits - hour, 6 bits - minutes, 5 bits - seconds
	uint16_t date_created; //7 bits - year, 4 bits - month, 5 bits - day
	uint16_t accessed_date; //same format as creation date
	uint16_t cluster_high;
	uint16_t modified_time; //same format as creation time
	uint16_t modified_date; //same format as creation date
	uint16_t cluster_low; //use to find first cluster of this entry
	uint32_t file_size; //in bytes
}__attribute__((packed)) Fat32DirectoryEntry;

//This entry is placed right before the regular directory entry
typedef struct {
	uint8_t order;
	uint8_t first_five[10]; //5, 2 byte characters
	uint8_t attribute;
	uint8_t type; //leave as zero
	uint8_t checksum;
	uint8_t next_six[12];
	uint16_t zero; //always stays zero
	uint8_t last_two[4];
}__attribute__((packed)) Fat32LFN;

typedef struct {
	uint32_t table_size_32;
	uint16_t extended_flags;
	uint16_t fat_version;
	uint32_t root_cluster;
	uint16_t fat_info;
	uint16_t backup_bs_sector;
	uint8_t reserved_0[12];
	uint8_t drive_number;
	uint8_t reserved_1;
	uint8_t boot_signature;
	uint32_t volume_id;
	uint8_t volume_label[11];
	uint8_t fat_type_label[8];
}__attribute__((packed)) Fat32ExtendedSector;

typedef struct {
	uint8_t boot_jmp[3];
	uint8_t oem_name[8];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sector_count;
	uint8_t table_count;
	uint16_t root_entry_count;
	uint16_t total_sectors_16;
	uint8_t media_type;
	uint16_t table_size_16;
	uint16_t sectors_per_track;
	uint16_t head_side_count;
	uint32_t hidden_sector_count;
	uint32_t total_sectors_32;

	Fat32ExtendedSector extended_section; //54 bytes
}__attribute__((packed)) Fat32BootSector;

typedef struct {
	size_t offset_base;
	size_t offset_fats;
	size_t offset_clusters;

	Fat32BootSector boot_sector;

	//store old FATs for easier lookup
	uint8_t *cache[FAT32_CACHE_MAX];
	uint32_t cache_base[FAT32_CACHE_MAX];
	int current_cache;
} Fat32;

typedef struct {
	int ptr;

	uint8_t index; // x / 32
	uint32_t directory_starting;
	uint32_t directory_current;

	Fat32DirectoryEntry directory_entry;
} Fat32OpenFD;

typedef struct {
	uint8_t attributes;
	uint8_t cylinder_start;
	uint8_t head_start;
	uint8_t sector_start;
	uint8_t filesystem;
	uint8_t cylinder_end;
	uint8_t head_end;
	uint8_t sector_end;
	uint32_t LBA_start;
	uint32_t size;
} Partition;

class FAT32 {
	enum FatType {
		T_ExFAT,
		T_FAT12,
		T_FAT16,
		T_FAT32
	};
	AHCI::Port *port;

	FatType fat_type;

	Fat32BootSector *boot_sector; //see fat32 struct above

	uint8_t *temp_buffer1;
	uint8_t *temp_buffer2;

	uint64_t partition_start;

	uint32_t first_data_sector;
	uint32_t root_dir_start;
	uint32_t total_sectors;
	uint32_t total_clusters;
	uint32_t cluster_size;
	uint32_t *fat_cache; //see fat32 struct above

	uint32_t read_fat_entry(uint32_t cluster);
	uint32_t write_fat_entry(uint32_t cluster, uint32_t value);

	public:
	FAT32(int driver);
};