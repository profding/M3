#
# Copyright (C) 2015, Nils Asmussen <nils@os.inf.tu-dresden.de>
# Economic rights: Technische Universitaet Dresden (Germany)
#
# This file is part of M3 (Microkernel-based SysteM for Heterogeneous Manycores).
#
# M3 is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation.
#
# M3 is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License version 2 for more details.
#

#include <m3/Config.h>

.global _start
.extern main
.extern lambda_main
.extern exit
.extern __clibrary_init
.extern __clibrary_init_lambda

_start:
    # set the stack pointer (BOOT_SP is set if we are calling a lambda)
    mov     $BOOT_SP, %rax
    mov     (%rax), %rsp
    test    %rsp, %rsp
    jne     1f
    mov     $STACK_TOP, %rsp
1:
    mov     $0, %rbp

    # check if we should call a lambda
    mov     $BOOT_LAMBDA, %rax
    mov     (%rax), %rax
    test    %rax, %rax
    je      1f

    # __clibrary_init_lambda(argc, argv)
    mov     $ARGC_ADDR, %rdi
    mov     (%rdi), %rdi
    mov     $ARGV_ADDR, %rsi
    mov     (%rsi), %rsi
    call    __clibrary_init_lambda

    # res = lambda_main(lambda)
    mov     $BOOT_LAMBDA, %rdi
    mov     (%rdi), %rdi
    call    lambda_main

    jmp     2f

1:
    # __clibrary_init(argc, argv)
    mov     $ARGC_ADDR, %rdi
    mov     (%rdi), %rdi
    mov     $ARGV_ADDR, %rsi
    mov     (%rsi), %rsi
    call    __clibrary_init

    # res = main(argc, argv)
    mov     $ARGC_ADDR, %rdi
    mov     (%rdi), %rdi
    mov     $ARGV_ADDR, %rsi
    mov     (%rsi), %rsi
    call    main

2:
    # exit(res)
    mov     %rax, %rdi
    call    exit

    # just to be sure
    hlt
