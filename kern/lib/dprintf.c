#ifdef DEBUG_MSG

#include <dev/console.h>
#include <dev/serial.h>

#include <lib/spinlock.h>
#include <lib/debug.h>
#include <lib/stdarg.h>

static spinlock_t print_lk;

void print_spinlock_init(){
        spinlock_init(&print_lk);
}

void print_spinlock_acquire(){
        spinlock_acquire(&print_lk);
}

void print_spinlock_release(){
        spinlock_release(&print_lk);
}

struct dprintbuf
{
    int idx; /* current buffer index */
    int cnt; /* total bytes printed so far */
    char buf[CONSOLE_BUFFER_SIZE];
};

static void
cputs (const char *str)
{
    while (*str)
    {
        cons_putc (*str);
        str += 1;
    }
}

static void
putch (int ch, struct dprintbuf *b)
{
    b->buf[b->idx++] = ch;
    if (b->idx == CONSOLE_BUFFER_SIZE - 1)
    {
        b->buf[b->idx] = 0;
        cputs (b->buf);
        b->idx = 0;
    }
    b->cnt++;
}

int
vdprintf (const char *fmt, va_list ap)
{

    struct dprintbuf b;

    b.idx = 0;
    b.cnt = 0;
    vprintfmt ((void*) putch, &b, fmt, ap);

    b.buf[b.idx] = 0;
    cputs (b.buf);

    return b.cnt;
}

int
dprintf (const char *fmt, ...)
{
    print_spinlock_acquire();
    va_list ap;
    int cnt;

    va_start(ap, fmt);
    cnt = vdprintf (fmt, ap);
    va_end(ap);
    print_spinlock_release();
    return cnt;
}

#endif /* DEBUG_MSG */
