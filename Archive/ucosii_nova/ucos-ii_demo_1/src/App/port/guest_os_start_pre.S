/*
 * guest_os_start_pre.S
 *
 *  Created on: 28 ao�t 2014
 *      Author: XIATAN
 */

.extern XSmc_NorInit
.extern XSmc_SramInit

	.file	"xil-crt0.S"
	.section ".got2","aw"
	.align	2

	.text
.Lsbss_start:
	.long	__sbss_start

.Lsbss_end:
	.long	__sbss_end

.Lbss_start:
	.long	__bss_start

.Lbss_end:
	.long	__bss_end

.Lstack:
	.long	__stack


	.globl	_guest_os_start_pre
_guest_os_start_pre:

	mov	r0, #0

	/* clear sbss */
	ldr 	r1,.Lsbss_start		/* calculate beginning of the SBSS */
	ldr	r2,.Lsbss_end		/* calculate end of the SBSS */

.Lloop_sbss:
	cmp	r1,r2
	bge	.Lenclsbss		/* If no SBSS, no clearing required */
	str	r0, [r1], #4
	b	.Lloop_sbss

.Lenclsbss:
	/* clear bss */
	ldr	r1,.Lbss_start		/* calculate beginning of the BSS */
	ldr	r2,.Lbss_end		/* calculate end of the BSS */

.Lloop_bss:
	cmp	r1,r2
	bge	.Lenclbss		/* If no BSS, no clearing required */
	str	r0, [r1], #4
	b	.Lloop_bss

.Lenclbss:

	/* set stack pointer */
	ldr	r13,.Lstack		/* stack address */

   /* run global constructors */
   bl __libc_init_array

	/* make sure argc and argv are valid */
	mov	r0, #0
	mov	r1, #0

	/* Let her rip */
	bl	main

   /* Cleanup global constructors */
   bl __libc_fini_array

        /* All done */
	bl	exit

.Lexit:	/* should never get here */
	b .Lexit

.Lstart:
	.size	_guest_os_start_pre,.Lstart-_guest_os_start_pre
