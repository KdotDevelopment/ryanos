#include "vfs.hpp"

Filesystem *filesystems[DEVICE_MAX];

File open_file(const char *filename) {
	if(filename) {
		unsigned char device = 'a';
		char *filename = (char *)filename;

		//This means it contains the "A:" bit and we can remove that
		if(filename[1] == ':') {
			device = filename[0]; //switches the device to (for example) "A" from "A:"
			filename += 2;
		}

		if(filesystems[device - 'a']) {
			File file = filesystems[device - 'a']->open(filename);
			file.device = device;
			return file;
		}
	}
	File file;
	file.flags = FS_INVALID;
	return file;
}

void register_filesystem(Filesystem *fs, uint32_t device_id) {
	if(device_id < DEVICE_MAX) {
		if(fs) {
			filesystems[device_id] = fs;
		}
	}
}