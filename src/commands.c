/*
 * commands.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: femto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern buffer_t *curbuf;
extern window_t *curwin;

static int32_t
line_column(buffer_t *b, int32_t offset, int32_t col)
{
  int c = 0;
	char_t *p;
	while ((p = ptr(b, offset)) < b->buf_end && *p != '\n' && c < col) {
		c += *p == '\t' ? 8 - (c & 7) : 1;
		offset += utflen(*ptr(b,offset));
	}
	return (offset);
}

void
insert(char_t *c)
{
  /* char plus null */
  if (curbuf->gap_start == curbuf->gap_end && !grow_gap(curbuf, 1))
    return;

  curbuf->point = move_gap(curbuf, curbuf->point);
  *curbuf->gap_start = *c;
  curbuf->gap_start++;
  curbuf->point++;
}

void
left(void)
{
  int32_t p = prev_utflen();
  while (0 < curbuf->point && p-- > 0)
    --curbuf->point;
}

void
right(void)
{
  if (curbuf->point < curbuf->size) {
    int32_t s = utflen(*ptr(curbuf, curbuf->point));
    while ((curbuf->point < pos(curbuf, curbuf->buf_end)) && s-- > 0)
      ++curbuf->point;
  }
}

void
up(void)
{
  curbuf->point = line_column(curbuf, line_up(curbuf, curbuf->point),curbuf->col);
}

void
down(void)
{
  curbuf->point = line_column(curbuf, line_down(curbuf, curbuf->point), curbuf->col);
}

void
delete(void)
{
	curbuf->point = move_gap(curbuf, curbuf->point);
	if (curbuf->gap_end < curbuf->buf_end) {
		curbuf->gap_end += utflen(*curbuf->gap_end);
		curbuf->point = pos(curbuf, curbuf->gap_end);
		curbuf->flags |= B_MODIFIED;
	}
}

void
backspace(void)
{
	curbuf->point = move_gap(curbuf, curbuf->point);
	if (curbuf->buf_start < curbuf->gap_start) {
		curbuf->gap_start -= prev_utflen();
		curbuf->flags |= B_MODIFIED;
	}
	curbuf->point = pos(curbuf, curbuf->gap_end);
}

void
pagedown(void)
{
	curbuf->page_start = curbuf->point = line_up(curbuf, curbuf->page_end);
	while (0 < curbuf->row--)
		down();
	curbuf->page_end = pos(curbuf, curbuf->buf_end);
}

void
pageup(void)
{
	int i = curwin->rows;
	while (0 < --i) {
		curbuf->page_start = line_up(curbuf, curbuf->page_start);
		up();
	}
}

void
writefile(void)
{
  char_t temp[FNAME_MAX];

	strn_cpy(temp, curbuf->file_name, FNAME_MAX);
	if (get_input("Write file: ", temp, FNAME_MAX, 0))
		if (save(temp) != 0)
			strn_cpy(curbuf->file_name, temp, FNAME_MAX);
}

void
savebuffer(void)
{
  if (curbuf->file_name[0] != '\0') {
    save(curbuf->file_name);
    return;
  } else {
    writefile();
  }
  refresh();
}
