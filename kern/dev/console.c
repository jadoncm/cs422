#include <lib/string.h>
#include <lib/types.h>
#include <lib/debug.h>
#include <lib/spinlock.h>

#include "video.h"
#include "console.h"
#include "serial.h"
#include "keyboard.h"

#define BUFLEN 1024
static char linebuf[BUFLEN];

struct {
	char buf[CONSOLE_BUFFER_SIZE];
	uint32_t rpos, wpos;
} cons;

static spinlock_t cons_lk;

void cons_spinlock_init(){
  spinlock_init(&cons_lk);
}

void cons_spinlock_acquire(){
  spinlock_acquire(&cons_lk);
}

void cons_spinlock_release(){
  spinlock_release(&cons_lk);
}

static spinlock_t vd_lk;

void vd_spinlock_init(){
	spinlock_init(&vd_lk);
}

void vd_spinlock_acquire(){
	spinlock_acquire(&vd_lk);
}

void vd_spinlock_release(){
	spinlock_release(&vd_lk);
}

static spinlock_t ser_lk;

void ser_spinlock_init(){
        spinlock_init(&ser_lk);
}

void ser_spinlock_acquire(){
        spinlock_acquire(&ser_lk);
}

void ser_spinlock_release(){
        spinlock_release(&ser_lk);
}

void
cons_init()
{
	memset(&cons, 0x0, sizeof(cons));
	cons_spinlock_init();
	serial_init();
	ser_spinlock_init();
	vd_spinlock_init();
	video_init();
}

void
cons_intr(int (*proc)(void))
{
	cons_spinlock_acquire();
	int c;

	while ((c = (*proc)()) != -1) {
		if (c == 0)
			continue;
		cons.buf[cons.wpos++] = c;
		if (cons.wpos == CONSOLE_BUFFER_SIZE)
			cons.wpos = 0;
	}
	cons_spinlock_release();

}

char
cons_getc(void)
{
  int c;


  // poll for any pending input characters,
  // so that this function works even when interrupts are disabled
  // (e.g., when called from the kernel monitor).
  serial_intr();
  keyboard_intr();

  cons_spinlock_acquire();
  // grab the next character from the input buffer.
  if (cons.rpos != cons.wpos) {
    c = cons.buf[cons.rpos++];
    if (cons.rpos == CONSOLE_BUFFER_SIZE){
      cons.rpos = 0;
    }
    cons_spinlock_release();
    return c;
  }
  cons_spinlock_release();

  return 0;
}

void
cons_putc(char c)
{
	ser_spinlock_acquire();
	serial_putc(c);
	ser_spinlock_release();
	vd_spinlock_acquire();
        video_putc(c);
	vd_spinlock_release();
}

char
getchar(void)
{
  char c;

  while ((c = cons_getc()) == 0)
    /* do nothing */;
  return c;
}

void
putchar(char c)
{
  cons_putc(c);
}

char *
readline(const char *prompt)
{
  int i;
  char c;

  if (prompt != NULL)
    dprintf("%s", prompt);

  i = 0;
  while (1) {
    c = getchar();
    if (c < 0) {
      dprintf("read error: %e\n", c);
      return NULL;
    } else if ((c == '\b' || c == '\x7f') && i > 0) {
      putchar('\b');
      i--;
    } else if (c >= ' ' && i < BUFLEN-1) {
      putchar(c);
      linebuf[i++] = c;
    } else if (c == '\n' || c == '\r') {
      putchar('\n');
      linebuf[i] = 0;
      return linebuf;
    }
  }
}
