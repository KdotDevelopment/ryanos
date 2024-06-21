[BITS 64]

load_gdt:
	lgdt [rdi]
	mov ax, 0x10 ; Kernel Data Segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	pop rdi      ; first var in stack into rdi
	mov rax, 0x08
	push rax     ; puts 0x08 (kernel code) into stack
	push rdi
	retfq        ; far 64bit return

GLOBAL load_gdt