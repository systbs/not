; ----------------------------------------------------------------------------------------
; Writes "Hello, World" to the console using only system calls. Runs on 64-bit Linux only.
; To assemble and run:
;
;     nasm -felf64 hello.asm && ld hello.o && ./a.out
; ----------------------------------------------------------------------------------------

global    _start

section .text

_start:  
	xor 	rdi, rdi
	mov		rax, 12
	syscall

	push 	rax
	
	mov		rdi, rax
	add		rdi, msglength
	mov		rax, 12				; sys_brk
	syscall

	cmp		rax, 0
	je		error

	mov		rsi, rax

	mov		r8, 1
	mov		r9, 0
line:
	mov		byte [rax], '*'
	inc		rax
	inc		r9
	cmp		r9, r8
	jne		line

	mov		byte [rax], 10
	inc 	rax
	inc		r8
	mov		r9, 0
	cmp		r8, rows
	jng		line

    mov		rax, 0x1                ; system call for write
    mov		rdi, 0x1                ; file handle 1 is stdout
	mov		rdx, 100
    syscall                         ; invoke operating system to do the write

	pop		rdi
	mov		rax, 12
	syscall

    mov		rax, 0x3C               ; system call for exit
    xor		rdi, rdi                ; exit code 0
    syscall                         ; invoke operating system to exit

error:
    mov		rax, 0x1                ; system call for write
    mov		rdi, 0x1                ; file handle 1 is stdout
    mov		rsi, msg                ; address of string to output
    mov		rdx, msglength          ; number of bytes
    syscall                         ; invoke operating system to do the write

	mov		rax, 0x3C               ; system call for exit
    mov		rdi, -1                	; exit code -1
    syscall                         ; invoke operating system to exit

section .data
	msg: db "Unable to allocate memory", 0xA
	msglength: equ $ - msg
	rows: equ 5

section .rodata
section .comment