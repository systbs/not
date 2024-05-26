; ----------------------------------------------------------------------------------------
; Writes "Hello, World" to the console using only system calls. Runs on 64-bit Linux only.
; To assemble and run:
;
;     nasm -felf64 test3.asm && ld test3.o libc.o && ./a.out
; ----------------------------------------------------------------------------------------

;rdi, rsi, rdx, rcx, r8, r9, qword [rsp + 8], qword [rsp + 16]
;cvtsi2sd, cvtsi2ss, cvtsd2si, cvtsd2ss

extern    fprintf
extern      sy_node_make_class

global    _start

section .text

_start:  
    mov     rdi, rax
    mov     rsi, V1
    mov     rdx, 2
    call    sy_node_make_class


	xor 	rdi, rdi
	mov		rax, 12
	syscall

    cmp		rax, -1
	je		error_occurred
	
	mov		rdi, rax
	add		rdi, 16
	mov		rax, 12				; sys_brk
	syscall

	cmp		rax, -1
	je		error_occurred

    mov     rbp, rax

	xor 	rdi, rdi
	mov		rax, 12
	syscall

    cmp		rax, -1
	je		error_occurred

    mov     qword [rbp - 16], rax

    mov		rdi, rax
	add		rdi, 8
	mov		rax, 12				; sys_brk
	syscall

    cmp		rax, -1
	je		error_occurred

	xor 	rdi, rdi
	mov		rax, 12
	syscall

    cmp		rax, -1
	je		error_occurred

    mov     qword [rbp - 8], rax   

    mov		rdi, rax
	add		rdi, 8
	mov		rax, 12				; sys_brk
	syscall

    cmp		rax, -1
	je		error_occurred

    mov     rax, [rbp - 8]

    movsd    xmm0, [flt2]

    mov     rax, [rbp - 16]
    mov     qword [rax], 8
    cvtsi2sd xmm1, [rax]
    ;movq   xmm1, [rax]

    addsd   xmm0, xmm1
    ;addpd   xmm0, xmm1

    cvtsd2si rax, xmm0
    ;movsd   rax, xmm0

    lea rdi, [error_occurred]
    call rdi

    ;rdi, rsi, rdx, rcx, r8, r9, qword [rsp + 8], qword [rsp + 16]

    ;mov     rdi, number
    ;mov     rsi, 10    
    ;call    convert_to_string    

    mov		rdi, 0x1                ; file handle 1 is stdout
    mov		rsi, format             ; address of string to output
    mov     rdx, rax
    mov     rcx, rax
    call    fprintf                 ; invoke operating system to do the write

    mov		rax, 0x3C               ; system call for exit
    xor		rdi, rdi                ; exit code 0
    syscall                         ; invoke operating system to exit

error_occurred:
    mov		rax, 0x1                ; system call for write
    mov		rdi, 0x1                ; file handle 1 is stdout
    mov		rsi, msg                ; address of string to output
    mov		rdx, msglength          ; number of bytes
    syscall                         ; invoke operating system to do the write
    ret
    
	mov		rax, 0x3C               ; system call for exit
    mov		rdi, -1                	; exit code -1
    syscall                         ; invoke operating system to exit

;rdi, rsi, rdx, rcx, r8, r9, qword [rsp + 8], qword [rsp + 16]

global      fputs
fputs:
    mov		rax, 0x1                ; system call for write
    syscall                         ; invoke operating system to do the write

    mov     rax, 0
    ret

global      sbrk
sbrk:
    push    rdi
    xor 	rdi, rdi
	mov		rax, 12
	syscall

    cmp		rax, -1
	je		sbrk_error
	
	mov		rdi, rax
    pop     rbx
	add		rdi, rbx
	mov		rax, 12
	syscall

	cmp		rax, -1
	je		sbrk_error
    ret;

sbrk_error:
    mov		rdi, 0x1
    mov		rsi, sbrk_error_msg 
    call    fprintf
    ret
    
	mov		rax, 0x3C
    mov		rdi, -1
    syscall

section .bss
    number resb 50

section .data
	msg: db "Unable to allocate memory", 0xA
	msglength: equ $ - msg

    sbrk_error_msg: db "Unable to allocate memory", 0xA
    
	rows: equ 5
    flt2:	dq	4.3211234
    format: db "Hello %f %d World\n", 0
    V1: db "VAR1", 0

section .rodata
section .comment