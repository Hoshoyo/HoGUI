.data
.code

asm_get_ret_addr proc
    mov rax, [rbp+8]
    ret
asm_get_ret_addr endp

end