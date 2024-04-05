; ----------------------------------------------------------------------------------------
; Writes "Hello, World" to the console using only system calls. Runs on 64-bit Linux only.
; To assemble and run:
;
;     nasm -felf64 hello.asm && ld hello.o && ./a.out
; ----------------------------------------------------------------------------------------

global    _start

section .text

_start:   
    mov       rax, 0x1                ; system call for write
    mov       rdi, 0x1                ; file handle 1 is stdout
    mov       rsi, msg                ; address of string to output
    mov       rdx, msglength          ; number of bytes
    syscall                           ; invoke operating system to do the write

    mov       rax, 0x3C               ; system call for exit
    xor       rdi, rdi                ; exit code 0
    syscall                           ; invoke operating system to exit

section .data
    msg: db "Hello, World", 0xA      ; note the newline at the end
    msglength: equ $ - msg

section .rodata
section .comment