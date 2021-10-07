	.file	"hw2-2.c"
	.text
	.globl	cread
	.type	cread, @function
cread:
.LFB11:
	.cfi_startproc
	testq	%rdi, %rdi
	je	.L3
	movq	(%rdi), %rax
	ret
.L3:
	movl	$0, %eax
	ret
	.cfi_endproc
.LFE11:
	.size	cread, .-cread
	.globl	cread_alt
	.type	cread_alt, @function
cread_alt:
.LFB12:
	.cfi_startproc
	movq	$0, -8(%rsp)
	leaq	-8(%rsp), %rax
	testq	%rdi, %rdi
	cmove	%rax, %rdi
	movq	(%rdi), %rax
	ret
	.cfi_endproc
.LFE12:
	.size	cread_alt, .-cread_alt
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"%ld %ld\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB13:
	.cfi_startproc
	subq	$8, %rsp
	.cfi_def_cfa_offset 16
	movl	$1, %edx
	movl	$1, %esi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %edx
	movl	$0, %esi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE13:
	.size	main, .-main
	.ident	"GCC: (GNU) 4.8.5 20150623 (Red Hat 4.8.5-36)"
	.section	.note.GNU-stack,"",@progbits
