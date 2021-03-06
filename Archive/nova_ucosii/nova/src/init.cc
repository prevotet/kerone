/*
 * init.cc
 *
 *  Created on: 24 f�vr. 2014
 *      Author: XIATAN
 */

#include "bsp/include/xil_printf.h"
#include "bsp/include/xil_exception.h"

#include "cpu/cpu.h"
#include "cpu/tlbflush.h"

#include "bsp/include/xil_cache.h"
#include "bsp/include/performance.h"

#include "include/memory.h"
#include "include/kalloc.h"
#include "include/ptab.h"
#include "include/types.h"
#include "include/vec.h"
#include "include/vgic.h"
#include "include/ec.h"
#include "include/print.h"
#include "include/config.h"


extern void XTime_SetTime(unsigned long long);

//extern void (* CTROS_L)(void); // Define CTORS_L as pointer to function
//extern void (* CTROS_E)(void);
extern void (* init_array_start)(void);
extern void (* init_array_end)(void);
extern mword KERN_PDIR;
extern mword LOAD_E;

char const *version = "NOVA Microhypervisor ARM version";

extern "C"
void init ()
{

	/*
	 * The first thing to to shut down IRQ exception during initialization period
	 * IRQ will be enabled by user code by syscall-irq_enable()
	 */
	Xil_ExceptionDisable();

	/*
	 * This is to run the constructors of static objects, (kalloc::allocator)
	 * This is obliged for initialization, because we use the constructor of
	 * kalloc::allocator to assign values to static data member "begin" and
	 * "end".
	 *
	 * For this stage, print functions are not included, thus we don't realise
	 * the print-related functions.
	 *
	 * This line is to be added when using link script, because the value type
	 * of CTROS_E is not compatible now
	 */
	for (void (**func)() = &init_array_end; func != &init_array_start; (*--func)()) ;

	// Now we're ready to talk to the world
	xil_printf ("\f%s: %s %s [%s]\n\r\n\r", version, __DATE__, __TIME__, COMPILER_STRING);

	/*
	 * Page mapping is set sa following:
	 *
	 *	area	phys				virt					size
	 *	-----------------------------------------------------------
	 *	IOBM	 0x408000-0x409FFF	0xD0000000-0xD0001FFF	0x2000
	 *	KSTCK	 0x40A000-0x40AFFF	0xCFFF0000-0xCFFFFFFF	0x1000
	 *	IRQ_STCK 0x40B000-0x40BFFF	0xCFFFE000-0XCFFFEFFF	0x1000
	 *
	 */
	mword iobm = Kalloc::virt2phys (Kalloc::allocator.alloc_page(2,Kalloc::FILL_1));
	Ptab::insert_mapping (KSTCK_ADDR	, Kalloc::virt2phys (Kalloc::allocator.alloc_page(1)), 1);
	Ptab::insert_mapping (IRQ_STCK_ADDR	, Kalloc::virt2phys (Kalloc::allocator.alloc_page(1)), 1);
	Ptab::insert_mapping (UND_STCK_ADDR	, Kalloc::virt2phys (Kalloc::allocator.alloc_page(1)), 1);
	Ptab::insert_mapping (ABT_STCK_ADDR	, Kalloc::virt2phys (Kalloc::allocator.alloc_page(1)), 1);
	Ptab::insert_mapping (IOBMP_SADDR, iobm, 1);
	Ptab::insert_mapping (IOBMP_SADDR + PAGE_SIZE, iobm + PAGE_SIZE, 1);

	//for (void (**func)() = &CTORS_G; func != &CTORS_L; (*--func)()) ;

	/*
	 * Initialize the exception table at address 0xFFFF0000
	 */
	Vec::vec_init();

	// Initialize IRQ service of kernel
	irq_init();

	// Initialize Performance Measurement
#if IS_PM_MEASURE
	PM_init();
	XTime_SetTime(0);
#endif

#if DEMO_UART
	demo_sender("INIT 4 go 1 go 1 ro 2 sv 3 OVER\n");
#endif
}

extern "C" REGPARM (1) NORETURN
void bootstrap (mword addr)
{
	// unmap everything below 3G : 0 - LOAD_E (clear the init mapping)
    mword* pdir = static_cast<mword*>(Kalloc::phys2virt(reinterpret_cast<mword>(&KERN_PDIR)));
    mword e = reinterpret_cast<mword>(&LOAD_E) >> 20;
    for (mword a = 0; a <= e; pdir[a++] = 0) ;

    //Cpu::flush_cache();
    Xil_DCacheFlush();
    Xil_ICacheInvalidate();

    Cpu::flush_tlb_all();

    print("INIT section has been unmapped \n\r");

    Ec::current = new Ec (Ec::root_invoke, addr);
    Ec::current->make_current();

    UNREACHED;
}
