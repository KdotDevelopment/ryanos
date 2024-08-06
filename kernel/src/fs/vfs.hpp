#include <stdint.h>
#include <stddef.h>

#define FS_FILE 0
#define FS_DIRECTORY 1
#define FS_INVALID 2

// Accordingly to fatfs
#define FS_MODE_READ 0x01
#define FS_MODE_WRITE 0x02
#define FS_MODE_OPEN_EXISTING 0x00
#define FS_MODE_CREATE_NEW 0x04
#define FS_MODE_CREATE_ALWAYS 0x08
#define FS_MODE_OPEN_ALWAYS 0x10
#define FS_MODE_OPEN_APPEND 0x30

// https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/fcntl.h
#define O_ACCMODE 00000003
#define O_RDONLY 00000000
#define O_WRONLY 00000001
#define O_RDWR 00000002
#define O_CREAT 00000100  /* not fcntl */
#define O_EXCL 00000200   /* not fcntl */
#define O_NOCTTY 00000400 /* not fcntl */
#define O_TRUNC 00001000  /* not fcntl */
#define O_APPEND 00002000
#define O_NONBLOCK 00004000
#define O_DSYNC 00010000  /* used to be O_SYNC, see below */
#define FASYNC 00020000   /* fcntl, for BSD compatibility */
#define O_DIRECT 00040000 /* direct disk access hint */
#define O_LARGEFILE 00100000
#define O_DIRECTORY 00200000 /* must be a directory */
#define O_NOFOLLOW 00400000  /* don't follow links */
#define O_NOATIME 01000000
#define O_CLOEXEC 02000000 /* set close_on_exec */
#define __O_SYNC 04000000
#define O_SYNC (__O_SYNC | O_DSYNC)
#define O_PATH 010000000
#define __O_TMPFILE 020000000
#define O_TMPFILE (__O_TMPFILE | O_DIRECTORY)
#define O_NDELAY O_NONBLOCK

//YES im writing this like its C, I can imagine myself reusing this code at some point and C is about as portable as it gets

typedef struct {
	char name[32];
	uint32_t flags;
	uint32_t file_length;
	uint32_t id;
	uint32_t eof;
	uint32_t position;
	uint32_t current_cluster;
	uint32_t device;
} File;

typedef struct {
	char name[18];
	File (*directory)(const char *directory_name);
	void (*mount)();
	void (*read)(File *file, unsigned char *buffer, uint32_t length);
	void (*close)(File *file);
	File (*open)(const char *filename);
} Filesystem;

#define DEVICE_MAX 26 //letters a-z

File open_file(const char *filename);
void read_file(File *file, unsigned char *buffer, uint32_t length);
void close_file(File *file);
void register_file_system(Filesystem *fs, uint32_t device_id);
void unregister_filesystem(Filesystem *fs);
void unregister_filesystem_by_id(uint32_t device_id);