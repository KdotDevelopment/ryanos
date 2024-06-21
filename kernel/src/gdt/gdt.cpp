#include "gdt.hpp"

__attribute__((aligned(0x1000)))
GDT default_gdt = {
	{0, 0, 0, 0x00, 0x00, 0}, //null
	{0, 0, 0, 0x9A, 0xA0, 0}, //kernel code segement
	{0, 0, 0, 0x92, 0xA0 ,0}, //kernel data segment
	{0, 0, 0, 0x00, 0x00, 0}, //user null
	{0, 0, 0, 0x9A, 0xA0, 0}, //user code segement
	{0, 0, 0, 0x92, 0xA0 ,0} //user data segment
};