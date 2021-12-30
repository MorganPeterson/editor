/*
 * display.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern char_t msgline[MSGBUF];
extern int32_t msgflag;
extern window_t *curwin;
extern window_t *headwin;
extern buffer_t *curbuf;

int32_t
line_start(buffer_t *b, int32_t offset)
{
  char_t *p;
  do {
    p = ptr(b, --offset);
  } while (b->buf_start < p && *p != '\n');
  return (b->buf_start < p ? ++offset : 0);
}

int32_t
segment_start(buffer_t *b, int32_t start, int32_t finish)
{
  char_t *p;
  int32_t c = 0;
  int32_t scan = start;

  while (scan < finish) {
    p = ptr(b, scan);
    if (*p == '\n') {
      c = 0;
      start = scan + 1;
    } else if (COLS <= c) {
      c = 0;
      start = scan;
    }
    scan += utflen(*ptr(b, scan));
    c += TABWIDTH(p, c);
  }
  return c < COLS ? start : finish;
}

static int32_t
segment_next(buffer_t *b, int32_t start, int32_t finish) {
  char_t *p;
  int32_t c = 0;
  int32_t scan = segment_start(b, start, finish);

  for(;;) {
    p = ptr(b, scan);
    if (b->buf_end <= p || COLS <= c)
      break;
    scan += utflen(*ptr(b, scan));
    if (*p == '\n')
      break;
    c += TABWIDTH(p, c);
  }
  return (p < b->buf_end ? scan : pos(b, b->buf_end));
}

int32_t
line_down(buffer_t *b, int32_t offset)
{
  return segment_next(b, line_start(b, offset), offset);
}

int32_t
line_up(buffer_t *b, int32_t offset)
{
  int32_t curr = line_start(b, offset);
  int32_t seg = segment_start(b, curr, offset);
  if (curr < seg)
    offset = segment_start(b, curr, seg-1);
  else
    offset = segment_start(b, line_start(b, curr-1), curr-1);
  return offset;
}

static void
modeline(window_t *w)
{
  int i;
	char lch, mch;
	static char modeline[256];

  attron(COLOR_PAIR(HL_MODELINE));
	move(w->top + w->rows, 0);
	lch = (w == curwin ? '=' : '-');
	mch = ((w->buf->flags & B_MODIFIED) ? '*' : lch);

	sprintf(modeline, "%c%c mpe: %c%c %s ",  lch,mch,lch,lch, w->buf->buf_name);
	addstr(modeline);
	for (i = strlen(modeline) + 1; i <= COLS; i++)
		addch(lch);
  attron(COLOR_PAIR(HL_NORMAL));
}

static void
display_message(void)
{
  move(MSGLINE, 0);
  if (msgflag) {
    addstr((char*)msgline);
    msgflag = 0;
  }
  clrtoeol();
}

void
display_prompt_and_response(char *prompt, char *response)
{
  mvaddstr(MSGLINE, 0, prompt);
  if (response[0] != '\0')
    addstr((char*)response);
  clrtoeol();
}

static void
display_character(buffer_t *b, char_t *p) {
  int32_t token_type = parse_text(b, b->page_end);
	attron(COLOR_PAIR(token_type));
  addch(*p);
}

static void
display(window_t *w, int32_t flag)
{
  buffer_t *b = w->buf;
  int32_t i, j, nch;
  char_t *p;

  if (b->point < b->page_start)
    b->page_start = segment_start(b, line_start(b, b->point), b->point);

  if (b->reframe == 1 || (b->page_end <= b->point && curbuf->point != pos(curbuf, curbuf->buf_end))) {
    b->reframe = 0;
    b->page_start = line_down(b, b->point);
    if (pos(b, b->buf_end) <= b->page_start) {
      b->page_start = pos(b, b->buf_end);
      i = w->rows - 1;
    } else {
      i = w->rows;
    }

    while (0 < i--)
      b->page_start = line_up(b, b->page_start);
  }

  move(w->top, 0);
  i = w->top;
  j = 0;
  b->page_end = b->page_start;
  set_parse_state(b, b->page_end);

  while (1) {
    if (b->point == b->page_end) {
      b->row = i;
      b->col = j;
    }
    p = ptr(b, b->page_end);
    nch = 1;
    if (w->top + w->rows <= i || b->buf_end <= p) /* max line */
      break;
    if (*p != '\r') {
      nch = utflen(*p);
      if (nch > 1) {
        j += display_utf(b, nch);
      } else if (isprint(*p) || *p == '\t' || *p == '\n') {
        j += TABWIDTH(p,j);
        display_character(b, p);
      } else {
        const char *ctrl = unctrl(*p);
        j += (int32_t)strlen(ctrl);
        addstr(ctrl);
      }
    }
    if (*p == '\n' || COLS <= j) {
      j -= COLS;
      if (j < 0)
        j = 0;
      ++i;
    }
    b->page_end += nch;
  }
  /* replacement for clrtobot() to bottom of window */
	for (int32_t k=i; k < w->top + w->rows; k++) {
		move(k, j); /* clear from very last char not start of line */
		clrtoeol();
		j = 0; /* thereafter start of line */
	}

	buf_to_win(w); /* save buffer to window */
	modeline(w);
	if (w == curwin && flag) {
		display_message();
		move(b->row, b->col); /* set cursor */
		refresh();
	}
	w->update = 0;
}

void
update_display(void)
{
  window_t *w;
  buffer_t *b;

  b = curwin->buf;
  b->cpoint = b->point;

  if (headwin->next == NULL) {
    display(curwin, 1);
    refresh();
    b->psize = b->size;
    return;
  }

  display(curwin, 0);

  for (w=headwin; w!=NULL; w=w->next) {
    if (w != curwin && (w->buf == b || w->update)) {
      win_to_buffer(w);
      display(w, 0);
    }
  }

  win_to_buffer(curwin);
  display_message();
  move(curwin->row, curwin->col);
  refresh();
  b->psize = b->size;
}

void
msg(char *msg, ...)
{
  va_list args;
  va_start(args, msg);
  (void)vsprintf((char*)msgline, msg, args);
  va_end(args);
  msgflag = 1;
}

void
display_search_result(int32_t fnd, int32_t point, int8_t dir, char *prompt, char *search)
{
  if (fnd == 0) {
    curwin->buf->point = point;
    msg("%s/%s", prompt, search);
    display(curwin, 1);
  } else {
    msg("Failing %s%s", prompt, search);
    display_message();
    curwin->buf->point = dir == FWD_SEARCH ? 0 : pos(curwin->buf, curwin->buf->buf_end);
  }
}

