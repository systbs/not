	global	_start
	global	sbrk
	extern	fprintf
section .text
	jmp		L0
	;jammping
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
	ret
	sbrk_error:
	mov		rdi, 0x1
	mov		rsi, sbrk_error_msg
	call    fprintf
	ret
	mov		rax, 0x3C
	mov		rdi, -1
	syscall
	L0:
	_start:
	mov     rdi, rax
	call    __node_make_module__
	mov     rdi, rax
	call    __node_make_module__
	mov     rdi, rax
	mov     rsi, RV0
	mov     rdx, 2
	call    __node_make_class__
	jmp    L1
	;jumpping
	L1:
	mov     rdi, rax
	mov     rsi, RV1
	mov     rdx, 2
	call    __node_make_class__
	jmp    L2
	;jumpping
	L2:
	mov     rdi, rax
	mov     rsi, RV2
	mov     rdx, 2
	call    __node_make_class__
	jmp    L3
	;jumpping
	L3:
	mov     rdi, rax
	mov     rsi, RV3
	mov     rdx, 2
	call    __node_make_class__
	jmp    L4
	;jumpping
	L4:
	mov     rdi, rax
	mov     rsi, RV4
	mov     rdx, 2
	call    __node_make_class__
	jmp    L5
	;jumpping
	L5:
	mov     rdi, rax
	mov     rsi, RV5
	mov     rdx, 2
	call    __node_make_class__
	jmp    L6
	;jumpping
	L6:
	mov     rdi, rax
	mov     rsi, RV6
	mov     rdx, 2
	call    __node_make_class__
	jmp    L7
	;jumpping
	L7:
	mov     rdi, rax
	mov     rsi, RV7
	mov     rdx, 2
	call    __node_make_class__
	jmp    L8
	;jumpping
	L8:
	mov     rdi, rax
	mov     rsi, RV8
	mov     rdx, 2
	call    __node_make_class__
	jmp    L9
	;jumpping
	L9:
	mov     rdi, rax
	mov     rsi, RV9
	mov     rdx, 2
	call    __node_make_class__
	jmp    L10
	;jumpping
	L10:
	mov     rdi, rax
	mov     rsi, RV10
	mov     rdx, 2
	call    __node_make_class__
	jmp    L11
	;jumpping
	L11:
	mov     rdi, rax
	mov     rsi, RV11
	mov     rdx, 2
	call    __node_make_class__
	jmp    L12
	;jumpping
	L12:
	mov     rdi, rax
	mov     rsi, RV12
	mov     rdx, 2
	call    __node_make_class__
	jmp    L13
	;jumpping
	L13:
	mov     rdi, rax
	mov     rsi, RV13
	mov     rdx, 2
	call    __node_make_class__
	jmp    L14
	;jumpping
	L14:
	mov     rdi, rax
	mov     rsi, RV14
	mov     rdx, 2
	call    __node_make_class__
	jmp    L15
	;jumpping
	L15:
	mov     rdi, rax
	mov     rsi, RV15
	mov     rdx, 2
	call    __node_make_class__
	jmp    L16
	;jumpping
	L16:
	mov     rdi, rax
	mov     rsi, RV16
	mov     rdx, 2
	call    __node_make_class__
	jmp    L17
	;jumpping
	L17:
	jmp    L18
	;jumpping
	L18:
	mov     rdi, rax
	mov     rsi, RV17
	mov     rdx, 2
	call    __node_make_class__
	jmp    L19
	;jumpping
	L19:
	jmp    L20
	;jumpping
	L20:
	jmp    L21
	;jumpping
	L21:
	mov     rdi, rax
	mov     rsi, RV18
	mov     rdx, 2
	call    __node_make_class__
	jmp    L22
	;jumpping
	L22:
section .bss
section .data
section .rodata
	sbrk_error_msg: db "Unable to allocate memory", 0xA
	RV0: db "Any", 0
	RV1: db "Null", 0
	RV2: db "Void", 0
	RV3: db "UInt8", 0
	RV4: db "UInt16", 0
	RV5: db "UInt32", 0
	RV6: db "UInt64", 0
	RV7: db "Int8", 0
	RV8: db "Int16", 0
	RV9: db "Int32", 0
	RV10: db "Int64", 0
	RV11: db "Float32", 0
	RV12: db "Float64", 0
	RV13: db "Complex32", 0
	RV14: db "Complex64", 0
	RV15: db "Char", 0
	RV16: db "A", 0
	RV17: db "B", 0
	RV18: db "System", 0
section .comment
