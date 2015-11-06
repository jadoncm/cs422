#ifdef DEBUG_MSG

#include <dev/console.h>
#include <dev/serial.h>

#include <lib/debug.h>
#include <lib/stdarg.h>
#include <lib/spinlock.h>

struct dprintbuf
{
    int idx; /* current buffer index */
    int cnt; /* total bytes printed so far */
    char buf[CONSOLE_BUFFER_SIZE];
};

static spinlock_t print_lk;

void print_spinlock_acquire(){
	spinlock_acquire(&print_lk);
}

void print_spinlock_release(){
	spinlock_release(&print_lk);
}

void print_spinlock_init(){
	spinlock_init(&print_lk);
}


static void
cputs (const char *str)
{
    print_spinlock_acquire();
    while (*str)
    {
        cons_putc (*str);
        str += 1;
    }
    print_spinlock_release();
}

static void
putch (int ch, struct dprintbuf *b)
{
    print_spinlock_acquire();
    b->buf[b->idx++] = ch;
    if (b->idx == CONSOLE_BUFFER_SIZE - 1)
    {
        b->buf[b->idx] = 0;
        cputs (b->buf);
        b->idx = 0;
    }
    b->cnt++;
    print_spinlock_release();
}

int
vdprintf (const char *fmt, va_list ap)
{
    print_spinlock_acquire();
    struct dprintbuf b;

    b.idx = 0;
    b.cnt = 0;
    vd_spinlock_acquire();
    vprintfmt ((void*) putch, &b, fmt, ap);
    vd_spinlock_release();

    b.buf[b.idx] = 0;
    cputs (b.buf);
    print_spinlock_release();
    return b.cnt;
}

int
dprintf (const char *fmt, ...)
{
    print_spinlock_acquire();
    va_list ap;
    int cnt;

    vd_spinlock_acquire();
    va_start(ap, fmt);
    cnt = vdprintf (fmt, ap);
    va_end(ap);
    vd_spinlock_release();
    print_spinlock_release();
    return cnt;
}

#endif /* DEBUG_MSG */
