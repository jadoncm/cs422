#include <lib/debug.h>
#include <lib/gcc.h>
#include <lib/stdarg.h>
#include <lib/x86.h>

#include <lib/types.h>
#include <lib/spinlock.h>

static spinlock_t debug_lk;

void debug_spinlock_acquire(){
	spinlock_acquire(&debug_lk);
}

void debug_spinlock_release(){
	spinlock_release(&debug_lk);
}

void debug_spinlock_init(){
	spinlock_init(&debug_lk);
}

void
debug_init(void)
{
	debug_spinlock_init();
}

void
debug_info(const char *fmt, ...)
{
#ifdef DEBUG_MSG
	debug_spinlock_acquire();
	va_list ap;
	va_start(ap, fmt);
	vdprintf(fmt, ap);
	va_end(ap);
	debug_spinlock_release();
#endif
}

#ifdef DEBUG_MSG

void
debug_normal(const char *file, int line, const char *fmt, ...)
{
	debug_spinlock_acquire();
	dprintf("[D] %s:%d: ", file, line);

	va_list ap;
	va_start(ap, fmt);
	vdprintf(fmt, ap);
	va_end(ap);
	debug_spinlock_release();
}

#define DEBUG_TRACEFRAMES	10

static void
debug_trace(uintptr_t ebp, uintptr_t *eips)
{
	debug_spinlock_acquire();
	int i;
	uintptr_t *frame = (uintptr_t *) ebp;

	for (i = 0; i < DEBUG_TRACEFRAMES && frame; i++) {
		eips[i] = frame[1];		/* saved %eip */
		frame = (uintptr_t *) frame[0];	/* saved %ebp */
	}
	for (; i < DEBUG_TRACEFRAMES; i++)
		eips[i] = 0;
	debug_spinlock_release();
}

gcc_noinline void
debug_panic(const char *file, int line, const char *fmt,...)
{
	debug_spinlock_acquire();
	int i;
	uintptr_t eips[DEBUG_TRACEFRAMES];
	va_list ap;

	dprintf("[P] %s:%d: ", file, line);

	va_start(ap, fmt);
	vdprintf(fmt, ap);
	va_end(ap);

	debug_trace(read_ebp(), eips);
	for (i = 0; i < DEBUG_TRACEFRAMES && eips[i] != 0; i++)
		dprintf("\tfrom 0x%08x\n", eips[i]);

	dprintf("Kernel Panic !!!\n");
	debug_spinlock_release();
	//intr_local_disable();
	halt();
}

void
debug_warn(const char *file, int line, const char *fmt,...)
{
	debug_spinlock_acquire();
	dprintf("[W] %s:%d: ", file, line);

	va_list ap;
	va_start(ap, fmt);
	vdprintf(fmt, ap);
	va_end(ap);
	debug_spinlock_release();
}

#endif /* DEBUG_MSG */
