/*
 * main.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, femto (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern keymap_t key_map;

volatile sig_atomic_t done = 0;
window_t *headwin;
window_t *curwin;
buffer_t *headbuf;
buffer_t *curbuf;
char_t msgline[MSGBUF];
int32_t msgflag;
char_t *scratch_name = (char_t*)"*scratch*";
keymap_t *key_return;

int32_t
main(int argc, char **argv) {
  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler=term_signal;
  action.sa_flags=0;
  sigaction(SIGTERM, &action, 0);
  sigaction(SIGSEGV, &action, 0);

  setlocale(LC_ALL, "");

  if (initscr() == NULL)
    fatal("%s: failed to initialize the screen\n");
  raw();
  noecho();
  idlok(stdscr, 1);

  start_color();
  init_pair(ID_DEFAULT, COLOR_CYAN, COLOR_BLACK);
  init_pair(ID_SYMBOL, COLOR_WHITE, COLOR_BLACK);
  init_pair(ID_MODELINE, COLOR_BLACK, COLOR_WHITE);

  TABSIZE=4;
  if (argc > 1) {
    char_t bname[BNAME_MAX];
    char_t fname[FNAME_MAX];

    strn_cpy(fname, (char_t*)argv[1], FNAME_MAX);
    make_buffer_name(bname, fname);

    curbuf = find_buffer(bname, 1);

    (void)insert_file(fname, 0);
    strn_cpy(curbuf->file_name, fname, FNAME_MAX);
    strn_cpy(curbuf->buf_name, bname, BNAME_MAX);
  } else {
    curbuf = find_buffer(scratch_name, 1);
    strn_cpy(curbuf->buf_name, scratch_name, BNAME_MAX);
  }

  headwin = curwin = new_window();
  if (headwin == NULL)
    die("window malloc failed", 1);

  one_window(headwin);

  attach_buf_win(headwin, curbuf);
  beginning_of_buffer();

  char_t *input;
  while (!done) {
    update_display();
    input = get_key(&key_map, &key_return);
    if (key_return != NULL) {
      (key_return->func)();
    } else {
      if (*input > 31 || *input == 0x0A || *input == 0x09) {
        insert(input);
      } else {
        flushinp();
        msg("key not bound");
      }
    }
  }
  quit();
}

void
term_signal(int32_t n) {
  switch (n) {
    case SIGTERM:
      die("SIGTERM", n);
      break;
    case SIGSEGV:
      die("SIGSEGV", n);
      break;
    case SIGINT:   /* fallthrough */
    case SIGTSTP:  /* fallthrough */
    case SIGWINCH: /* fallthrough */
      break;
    default:
      die("UNKNOWN", n);
  }
}

void
free_windows(void) {
  window_t *w;
  w = headwin;
  while (w != NULL) {
    headwin = w->next;
    free(w->buf->data);
    free(w->buf);
    free(w);
    w = headwin;
  }
}

void
die(const char *s, int32_t code) {
  switch (code) {
    case 0:
    case 1:
    case 2: /* fallthrough */
    default:
      free_windows();
      if (curscr != NULL) {
		    move(LINES-1, 0);
		    refresh();
		    noraw();
		    endwin();
      }
      break;
  }
  perror(s);
  done = 1;
  exit(code);
}

void
fatal(char *msg) {
	if (curscr != NULL) {
		move(LINES-1, 0);
		refresh();
		noraw();
		endwin();
		putchar('\n');
	}
	fprintf(stderr, msg, "editor");
	exit(1);
}

void
quit(void) {
  die("QUIT", 0);
}
