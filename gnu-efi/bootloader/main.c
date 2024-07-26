#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <stdlib.h>

#define DESIRED_WINDOW_X 1920
#define DESIRED_WINDOW_Y 1080

typedef struct {
	void* BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanLine;
} Framebuffer;

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} PSF1_HEADER;

typedef struct {
	PSF1_HEADER *psf1_header;
	void *glyph_buffer;
} PSF1_FONT;

Framebuffer framebuffer;

Framebuffer *InitializeGOP() {
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
	if(EFI_ERROR(status)) {
		Print(L"Unable to find GOP\n\r");
		return NULL;
	}else {
		Print(L"GOP located\n\r");
	}

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo = 0, numModes = 0, nativeMode = 0;

	status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode==NULL?0:gop->Mode->Mode, &SizeOfInfo, &info);
	// this is needed to get the current video mode
	if (status == EFI_NOT_STARTED)
		status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
	if(EFI_ERROR(status)) {
		Print(L"Unable to get native mode\n\r");
	} else {
		nativeMode = gop->Mode->Mode;
		numModes = gop->Mode->MaxMode;
	}
	UINTN mode = nativeMode;
	for (UINTN i = 0; i < numModes; i++) {
		status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
		if(info->HorizontalResolution == DESIRED_WINDOW_X && info->VerticalResolution == DESIRED_WINDOW_Y) {
			mode = i;
		}
		Print(L"mode %03d width %d height %d format %x%s \n\r",
			i,
			info->HorizontalResolution,
			info->VerticalResolution,
			info->PixelFormat,
			i == nativeMode ? "(current)" : ""
		);
	}

	status = uefi_call_wrapper(gop->SetMode, 2, gop, mode);
	if(EFI_ERROR(status)) {
		Print(L"Unable to set mode %03d\n\r", mode);
	} else {
		// get framebuffer
		Print(L"Framebuffer address %x size %d, width %d height %d pixelsperline %d",
		gop->Mode->FrameBufferBase,
		gop->Mode->FrameBufferSize,
		gop->Mode->Info->HorizontalResolution,
		gop->Mode->Info->VerticalResolution,
		gop->Mode->Info->PixelsPerScanLine
		);
	}

	framebuffer.BaseAddress = (void*)gop->Mode->FrameBufferBase;
	framebuffer.BufferSize = gop->Mode->FrameBufferSize;
	framebuffer.Width = gop->Mode->Info->HorizontalResolution;
	framebuffer.Height = gop->Mode->Info->VerticalResolution;
	framebuffer.PixelsPerScanLine = gop->Mode->Info->PixelsPerScanLine;

	return &framebuffer;
}

EFI_FILE *LoadFile(EFI_FILE *Directory, CHAR16 *Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	EFI_FILE *LoadedFile;

	EFI_LOADED_IMAGE_PROTOCOL *LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

	if(Directory == NULL) {
		FileSystem->OpenVolume(FileSystem, &Directory);
	}

	EFI_STATUS s = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if(s != EFI_SUCCESS) {
		return NULL;
	}
	return LoadedFile;
}

PSF1_FONT *LoadPSF1Font(EFI_FILE *Directory, CHAR16 *Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	EFI_FILE *font = LoadFile(Directory, Path, ImageHandle, SystemTable);
	if(font == NULL) return NULL;

	PSF1_HEADER *font_header;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_HEADER), (void**)&font_header);
	UINTN size = sizeof(PSF1_HEADER);
	font->Read(font, &size, font_header);

	if(font_header->magic[0] != PSF1_MAGIC0 || font_header->magic[1] != PSF1_MAGIC1) return NULL;

	UINTN glyph_buffer_size = font_header->charsize * 256;
	if(font_header->mode == 1) { //512 glyph mode
		glyph_buffer_size = font_header->charsize * 512;
	}

	void *glyph_buffer;
	{
		font->SetPosition(font, sizeof(PSF1_HEADER));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyph_buffer_size, (void**)&glyph_buffer);
		font->Read(font, &glyph_buffer_size, glyph_buffer);
	}

	PSF1_FONT *finished_font;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finished_font);
	finished_font->psf1_header = font_header;
	finished_font->glyph_buffer = glyph_buffer;
	return finished_font;
}

int memcmp(const void *aptr, const void *bptr, size_t n) {
	const unsigned char *a = aptr, *b = bptr;
	for(size_t i = 0; i < n; i++) {
		if(a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}
typedef struct {
	Framebuffer *framebuffer;
	PSF1_FONT *font;
	EFI_MEMORY_DESCRIPTOR *m_map;
	UINTN m_map_size;
	UINTN m_map_descriptor_size;
	void *rsdp;
} boot_info_t;

UINTN strcmp(CHAR8 *a, CHAR8 *b, UINTN length) {
	for(UINTN i = 0; i < length; i++) {
		if (*a != *b) return 0;
	}
	return 1;
}

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {	
	InitializeLib(ImageHandle, SystemTable); //allows uefi to use certain commands
	Print(L"String guangdong\n\r");

	EFI_FILE *Kernel = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);
	if(Kernel == NULL) {
		Print(L"Could not load kernel!\n\r");
	}else {
		Print(L"Kernel Loaded Successfully\n\r");
	}

	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO* FileInfo;
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo); //malloc ( FileInfoSize )
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);
	}

	UINTN size = sizeof(header);
	Kernel->Read(Kernel, &size, &header);

	if(
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 ||
		header.e_version != EV_CURRENT
	) {
		Print(L"Kernel Format is Bad\r\n");
	}else {
		Print(L"Kernel Header Successfully Verified\n\r");
	}

	Elf64_Phdr *phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	for(
		Elf64_Phdr *phdr = phdrs;
		(char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
	) {
		switch(phdr->p_type) {
			case PT_LOAD:
			{
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

				Kernel->SetPosition(Kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				Kernel->Read(Kernel, &size, (void*)segment);
				break;
			}
		}
	}

	Print(L"Kernel Loaded\n\r");

	PSF1_FONT *new_font = LoadPSF1Font(NULL, L"zap-light16.psf", ImageHandle, SystemTable);
	if(new_font == NULL) {
		Print(L"Font is not valid or not found\n\r");
	}else {
		Print(L"Font found. charsize = %d\n\r", new_font->psf1_header->charsize);
	}

	Framebuffer *new_buffer = InitializeGOP();
	Print(L"Base: 0x%x\n\rSize: 0x%x\n\rWidth: %d \n\rHeight: %d \n\rPPS: %d \n\r", 
	new_buffer->BaseAddress, new_buffer->BufferSize, new_buffer->Width, new_buffer->Height, new_buffer->PixelsPerScanLine);

	EFI_MEMORY_DESCRIPTOR *Map = NULL;
	UINTN MapSize, MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;
	{
		
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
		MapSize += 2*DescriptorSize; // padding because for some ungodly reason I ran out of buffer space at the most random time... like what the heck?
									 // https://stackoverflow.com/questions/72953406/memory-map-and-framebuffer-after-using-exitbootservices
		SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void**)&Map);
		SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);

	}

	EFI_CONFIGURATION_TABLE *config_table = SystemTable->ConfigurationTable;
	void *rsdp = NULL;
	EFI_GUID acpi_2_table_guid = ACPI_20_TABLE_GUID;

	for(UINTN index = 0; index < SystemTable->NumberOfTableEntries; index++) {
		if(CompareGuid(&config_table[index].VendorGuid, &acpi_2_table_guid)) {
			if(strcmp((CHAR8 *)"RSD PTR ", (CHAR8 *)config_table->VendorTable, 8)) {
				rsdp = (void *)config_table->VendorTable;
				//break; //cant break because the first one found is version 1 which is bad
			}
		}
		config_table++;
	}

	void (*KernelStart)(boot_info_t*) = ((__attribute__((sysv_abi)) void (*)(boot_info_t*) ) header.e_entry);

	boot_info_t boot_info;
	boot_info.framebuffer = new_buffer;
	boot_info.font = new_font;
	boot_info.m_map = Map;
	boot_info.m_map_size = MapSize;
	boot_info.m_map_descriptor_size = DescriptorSize;
	boot_info.rsdp = rsdp;

	Print(L"%d", Map->NumberOfPages);

	SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);

	KernelStart(&boot_info);

	//Print(L"%d\r\n", KernelStart());

	return EFI_SUCCESS; // Exit the UEFI application
}
