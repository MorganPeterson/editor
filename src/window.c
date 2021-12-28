/*
 * window.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern window_t *headwin;
extern window_t *curwin;

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
free_windows(void)
{
  window_t *head = headwin;
  window_t *next;
  while (head != NULL) {
    next = head->next;
    free(head);
    head = next;
  }
}

void
one_window(window_t *w)
{
  w->top = 0;
  w->rows = LINES-2;
  w->next = NULL;
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
  b->cnt++;
}

void
mark_all_windows(void)
{
  window_t *w;
  for (w=headwin; w!=NULL; w=w->next)
    w->update = 1;
}

window_t*
split_current_window(void)
{
  window_t *wp, *wp2;
	int32_t ntru, ntrl;

	if (curwin->rows < 3) {
		msg("Cannot split a %d line window", curwin->rows);
		return NULL;
	}

	wp = new_window();
	associate_buffer_to_win(curwin->buf, wp);
	buf_to_win(wp); /* inherit buffer settings */

	ntru = (curwin->rows - 1) / 2; /* Upper size */
	ntrl = (curwin->rows - 1) - ntru; /* Lower size */

	/* Old is upper window */
	curwin->rows = ntru;
	wp->top = curwin->top + ntru + 1;
	wp->rows = ntrl;

	/* insert it in the list */
	wp2 = curwin->next;
	curwin->next = wp;
	wp->next = wp2;
  clear();
  mark_all_windows();
	update_display(); /* mark the lot for update */
	return curwin;
}

void
free_other_windows(window_t *w)
{
  window_t *wp, *next;
  for (wp=next=headwin; next != NULL; wp=next) {
    next = wp->next;
    if (wp != w) {
      disassociate_buffer(wp);
      free(wp);
    }
  }
  headwin = curwin = w;
  one_window(w);
}

