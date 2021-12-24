/*
 * commands.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: femto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern buffer_t *curbuf;
extern window_t *curwin;

void
beginning_of_buffer(void)
{
  curbuf->point = 0;
}

void
end_of_buffer(void) {
  curbuf->point = pos(curbuf, curbuf->buf_end);
  if (curbuf->page_end < pos(curbuf, curbuf->buf_end))
    curbuf->reframe = 1;
}

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
  temp[0] = '\0';

	strn_cpy(temp, curbuf->file_name, FNAME_MAX);
	if (get_input("write-file: ", temp, FNAME_MAX, 0))
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

void
insertfile(void)
{
  char_t temp[FNAME_MAX];
  temp[0] = '\0';
  if (get_input("insert-file: ", temp, FNAME_MAX, 1))
    (void)insert_file(temp, 1);
}

void
findfile(void)
{
  char_t fname[FNAME_MAX];
  char_t bname[BNAME_MAX];

  fname[0] = '\0';

  if (get_input("find-file: ", fname, FNAME_MAX, 1)) {
    buffer_t *b = find_buffer_fname(fname);

    disassociate_buffer(curwin);

    curbuf = b;

    associate_buffer_to_win(curbuf, curwin);

    clear_buffer();
    (void)insert_file(fname, 0);
    beginning_of_buffer();

    make_buffer_name(bname, fname);
    strn_cpy(curbuf->file_name, fname, FNAME_MAX);
    strn_cpy(curbuf->buf_name, bname, BNAME_MAX);
  }
}

void
search(void) {
  int32_t cpos = 0;
  int32_t c, found;
  int32_t old_point = curbuf->point;
  char temp[K_BUFFER_LENGTH];
  filerange_t match[1];
  match[0].start = old_point;
  match[0].end = old_point;
  temp[0] = '\0';
  display_prompt_and_response("search: ", temp);
  cpos = strlen(temp);

  for(;;) {
    c = getch();
    /* ignore control keys other than C-g, backspace, CR,  C-s, C-R, ESC */
		if (c < 32 && c != 07 && c != 0x08 && c != 0x13 && c != 0x12 && c != 0x1b)
			continue;

    switch (c) {
      case 0x1B: /* esc */
        temp[cpos] = '\0';
        flushinp(); /* discard without printing to screen */
        return;
      case 0x07: /* ctrl-g */
        curbuf->point = old_point;
        return;
      case 0x13: /* ctrl-s search forward */
        found = search_forward(temp, match, 1);
        display_search_result(found, match[0].start, FWD_SEARCH, "search: ", temp);
        break;
      case 0x12: /* ctrl-r search backward */
        found = search_backward(temp, match, 1);
        display_search_result(found, match[0].start, BWD_SEARCH, "search: ", temp);
        break;
      case 0x7F: /* fallthrough - del, erase */
      case 0x08: /* backspace */
        if (cpos == 0)
          continue;
        temp[--cpos] = '\0';
        display_prompt_and_response("search: ", temp);
        break;
      default:
        if (cpos < K_BUFFER_LENGTH - 1) {
          temp[cpos++] = c;
          temp[cpos] = '\0';
          display_prompt_and_response("search: ", temp);
        }
        break;
    }
  }
}

void
gotoline(void)
{
  int32_t line;
  char temp[BNAME_MAX];
  temp[0] = '\0';
  if (get_input("goto-line: ", (char_t*)temp, BNAME_MAX, 1)) {
    line = atoi(temp);
    goto_line(line);
  }
}
