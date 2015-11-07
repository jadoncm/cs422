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

static spinlock_t getc_lk;
static spinlock_t rl_lk;

void
cons_init()
{
  print_spinlock_init();
  cons_spinlock_init();
  spinlock_init(&getc_lk);
  spinlock_init(&rl_lk);
  cons_spinlock_acquire();
	memset(&cons, 0x0, sizeof(cons));
  cons_spinlock_release();
  ser_spinlock_init();
  ser_spinlock_acquire();
	serial_init();
  ser_spinlock_release();
  vd_spinlock_init();
  vd_spinlock_acquire();
	video_init();
  vd_spinlock_release();
}

void
cons_intr(int (*proc)(void))
{
	int c;
	cons_spinlock_acquire();
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

  // grab the next character from the input buffer.
  cons_spinlock_acquire();
  if (cons.rpos != cons.wpos) {
    c = cons.buf[cons.rpos++];
    if (cons.rpos == CONSOLE_BUFFER_SIZE)
      cons.rpos = 0;
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
  spinlock_acquire(&getc_lk);
  while ((c = cons_getc()) == 0)
    /* do nothing */;
  spinlock_release(&getc_lk);
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
  spinlock_acquire(&rl_lk);
  int i;
  char c;

  print_spinlock_acquire();
  if (prompt != NULL)
    dprintf("%s", prompt);

  i = 0;
  while (1) {
    spinlock_acquire(&getc_lk);
    c = getchar();
    spinlock_release(&getc_lk);
    cons_spinlock_acquire();
    if (c < 0) {
      dprintf("read error: %e\n", c);
      spinlock_release(&rl_lk);
      print_spinlock_release();
      cons_spinlock_release();
      return NULL;
    } else if ((c == '\b' || c == '\x7f') && i > 0) {
      putchar('\b');
      i--;
      cons_spinlock_release();
    } else if (c >= ' ' && i < BUFLEN-1) {
      putchar(c);
      linebuf[i++] = c;
      cons_spinlock_release();
    } else if (c == '\n' || c == '\r') {
      putchar('\n');
      linebuf[i] = 0;
      spinlock_release(&rl_lk);
      print_spinlock_release();
      cons_spinlock_release();
      return linebuf;
    }
  }
}
