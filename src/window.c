/*
 * window.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern window_t *headwin;

window_t*
new_window(void)
{
  window_t *w = (window_t*)malloc(sizeof(window_t));
  if (w == NULL)
    return NULL;
  w->buf = NULL;
  w->next = NULL;
  w->top = 0;
  w->rows = 0;
  w->point = 0;
  w->page_start = 0;
  w->page_end = 0;
  w->row = 0;
  w->col = 0;
  w->update = 0;
  return w;
}

void
one_window(window_t *w)
{
  w->top = 0;
  w->rows = LINES-2;
}

void
buf_to_win(window_t *w)
{
  w->point = w->buf->point;
	w->page_start = w->buf->page_start;
	w->page_end = w->buf->page_end;
	w->row = w->buf->row;
	w->col = w->buf->col;
	w->buf->size = (w->buf->buf_end - w->buf->buf_start) - (w->buf->gap_end - w->buf->gap_start);
}

void
win_to_buffer(window_t *w)
{
  w->buf->point = w->point;
	w->buf->page_start = w->page_start;
	w->buf->page_end = w->page_end;
	w->buf->row = w->row;
	w->buf->col = w->col;

	/* fixup pointers in other windows of the same buffer, if size of edit text changed */
	if (w->buf->point > w->buf->cpoint) {
		w->buf->point += (w->buf->size - w->buf->psize);
		w->buf->page_start += (w->buf->size - w->buf->psize);
		w->buf->page_end += (w->buf->size - w->buf->psize);
	}
}

void
disassociate_buffer(window_t *w)
{
  w->buf = NULL;
}

void
associate_buffer_to_win(buffer_t *b, window_t *w)
{
  w->buf = b;
}

void
mark_all_windows(void)
{
  window_t *w;
  for (w=headwin; w!=NULL; w=w->next)
    w->update = 1;
}
