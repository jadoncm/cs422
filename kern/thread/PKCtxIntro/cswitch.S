/*
 * void cswitch(struct kctx *from, struct kctx *to);
 */

	.globl cswitch
cswitch:
	movl	  4(%esp), %eax	/* %eax <- from */
	movl	  8(%esp), %edx	/* %edx <- to */

	/* TODO: save the old kernel context */
	movl      %edi, 4(%eax)
	movl 	  (%esp), %edi
	movl      %edi, 20(%eax)
        movl      %edi, (%eax)
	movl      %esi, 8(%eax)
	movl      %ebx, 12(%eax)
	movl      %ebp, 16(%eax)

	/* TODO: load the new kernel context */
        movl      (%edx), %edi
	movl      %edi, (%esp)
        movl      20(%edx), %edi
        movl      %edi, (%esp)
        movl      4(%edx), %edi
        movl      8(%edx), %esi
        movl      12(%edx), %ebx
        movl      16(%edx), %ebp	

	xor       %eax, %eax
	ret
