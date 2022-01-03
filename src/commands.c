/*
 * commands.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: femto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern buffer_t *headbuf;
extern buffer_t *curbuf;
extern window_t *headwin;
extern window_t *curwin;
extern char_t *scrap;
extern int32_t nscrap;
extern char_t *scratch_name;

char *str_yes = "y\b";
char *str_no = "n\b";

static
int32_t yesno(int32_t flag)
{
	int32_t ch;

	addstr(flag ? str_yes : str_no);
	refresh();
	ch = getch();
	if (ch == '\r' || ch == '\n')
		return (flag);
	return (tolower(ch) == str_yes[0]);
}

static void
quit_ask(void)
{
  if (modified_buffers() > 0) {
    mvaddstr(MSGLINE, 0, "unsaved changes; quit? ");
    clrtoeol();
    if (!yesno(0))
      return;
  }
  die("QUIT", 0);
}

void
quit(void) {
  quit_ask();
}

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
		c += TABWIDTH(p,c);
		offset += utflen(*ptr(b,offset));
	}
	return (offset);
}

void
insert(char_t *c)
{
  if (curbuf->gap_start == curbuf->gap_end && !grow_gap(curbuf, 1))
    return;

  curbuf->point = move_gap(curbuf, curbuf->point);
  *curbuf->gap_start = *c;
  curbuf->gap_start++;
  curbuf->point++;
  curbuf->flags = B_MODIFIED;
  add_undo(curbuf, UNDO_INSERT, curbuf->point, c, NULL);
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
linebegin(void)
{
  curbuf->point = segment_start(curbuf, line_start(curbuf, curbuf->point), curbuf->point);
}

void
lineend(void)
{
  /* do nothing if EOF */
  if (curbuf->point == pos(curbuf, curbuf->buf_end))
    return;
  curbuf->point = line_down(curbuf, curbuf->point);
	int32_t p = curbuf->point;
	left();
	curbuf->point = (*ptr(curbuf, curbuf->point) == '\n') ? curbuf->point : p;
}

void
backwardword(void)
{
	char_t *p;
	while (!isspace(*(p = ptr(curbuf, curbuf->point))) && curbuf->buf_start < p)
		--curbuf->point;
	while (isspace(*(p = ptr(curbuf, curbuf->point))) && curbuf->buf_start < p)
		--curbuf->point;
}

void
forwardword(void)
{
	char_t *p;
	while (!isspace(*(p = ptr(curbuf, curbuf->point))) && p < curbuf->buf_end)
		++curbuf->point;
	while (isspace(*(p = ptr(curbuf, curbuf->point))) && p < curbuf->buf_start)
		++curbuf->point;
}

void
delete(void)
{
  char_t the_char[7]; /* the deleted char, allow 6 unsigned chars plus a null */
	curbuf->point = move_gap(curbuf, curbuf->point);

	if (curbuf->gap_end < curbuf->buf_end) {
	  int32_t n = utflen(*curbuf->gap_end);
		/* record the deleted chars in the undo structure */
		memcpy(the_char, curbuf->gap_end, n);
		the_char[n] = '\0'; /* null terminate, the deleted char(s) */
		curbuf->gap_end += n;
		curbuf->point = pos(curbuf, curbuf->gap_end);
		add_mode(curbuf, B_MODIFIED);
		add_undo(curbuf, UNDO_DELETE, curbuf->point, the_char, NULL);
	}
}

void
killtoeol(void)
{
  if (curbuf->point == pos(curbuf, curbuf->buf_end))
    return;
  if (*(ptr(curbuf, curbuf->point)) == 0xA) {
    delete();
  } else {
    curbuf->mark = curbuf->point;
    lineend();
    if (curbuf->mark != curbuf->point)
      copy_cut(1);
  }
}

void
backspace(void)
{
  char_t the_char[7]; /* the deleted char, allow 6 unsigned chars plus a null */
	int n = prev_utflen();

	curbuf->point = move_gap(curbuf, curbuf->point);

	if (curbuf->buf_start < (curbuf->gap_start - (n - 1)) ) {
		curbuf->gap_start -= n; /* increase start of gap by size of char */
		add_mode(curbuf, B_MODIFIED);

		/* record the backspaced chars in the undo structure */
		memcpy(the_char, curbuf->gap_start, n);
		the_char[n] = '\0'; /* null terminate, the backspaced char(s) */
		curbuf->point = pos(curbuf, curbuf->gap_end);
		add_undo(curbuf, UNDO_BACKSPACE, curbuf->point, the_char, NULL);
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
	if (get_input("write-file: ", temp, FNAME_MAX, 0)) {
		if (save(temp) != 0) {
			strn_cpy(curbuf->file_name, temp, FNAME_MAX);
      	select_syntax(curbuf);
    	}
  	}
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
    select_syntax(curbuf);
  }
}

void
search(void)
{
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

/* scan buffer and fill in curline and lastline */
void
get_line_stats(int32_t *curline, int32_t *lastline)
{
	int32_t endp = pos(curbuf, curbuf->buf_end);

	*curline = -1;
  	*lastline = 0;

	for (int32_t p=0, line=0; p < endp; p++) {
		line += (*(ptr(curbuf, p)) == '\n') ? 1 : 0;
		*lastline = line;

		if (*curline == -1 && p == curbuf->point) {
			*curline = (*(ptr(curbuf, p)) == '\n') ? line : line + 1;
		}
	}

	*lastline = *lastline + 1;

	if (curbuf->point == endp)
		*curline = *lastline;
}

void
cursorpos(void)
{
  int32_t current, last;
  get_line_stats(&current, &last);
  msg("row %d/%d", current, last);
}

void
undocmd(void)
{
  int32_t continue_undo = 1;
	undo_t *up = curbuf->undo;
	curbuf->undo_cnt = -1;
	char_t *input = NULL;

	if (up == NULL) {
		msg("no undo recorded for this buffer");
		return;
	}

	while (continue_undo) {
		up = execute_undo(up, input);

		update_display();
		if (up == NULL) {
			msg("out of undo");
			curbuf->undo_cnt = -1;
			return;
		}
		continue_undo = get_undo_again(input);
	}

  if (input != NULL)
    free(input);

	curbuf->undo_cnt = -1;
}

void
setmark(void)
{
  set_mark();
  msg("mark set to point %d", curbuf->mark);
}

void
killregion(void)
{
  if (check_region() == 0)
    return;
  copy_cut(1);
}

void
copyregion(void)
{
  if (check_region() == 0)
    return;
  copy_cut(0);
}

void
yank(void)
{
  insert_string(curbuf, scrap, nscrap, 1);
}

void
nextbuffer(void) {
  disassociate_buffer(curwin);
  curbuf = curbuf->next != NULL ? curbuf->next : headbuf;
  associate_buffer_to_win(curbuf, curwin);
}

void
killbuffer(void) {
  buffer_t *killbuf = curbuf;
  int32_t bcount = count_buffers();

  /* don't do anything if only the scratch buffer is left */
  if (bcount == 1 && 0 == strn_cmp(curbuf->buf_name, scratch_name, BNAME_MAX))
    return;

  if (!(curbuf->flags & B_SPECIAL) && curbuf->flags & B_MODIFIED) {
    mvaddstr(MSGLINE, 0, "buffer is not saved");
    clrtoeol();
    if (!yesno(0))
      return;
  }

  if (bcount == 1)
    (void)find_buffer(scratch_name, 1);

  nextbuffer();
  delete_buffer(killbuf);
}

void
splitwindow(void)
{
  (void)split_current_window();
}

void
otherwindow(void)
{
  curwin->update = 1;
  curwin = (curwin->next == NULL ? headwin : curwin->next);
  curbuf = curwin->buf;
  if (curbuf->cnt > 1)
    win_to_buffer(curwin);
}

void
deleteotherwindows(void)
{
  if (headwin->next == NULL)
    return;
  free_other_windows(curwin);
}

void
queryreplace(void)
{
	int32_t oldpoint = curbuf->point;
	int32_t lastpoint = -1;
    int32_t found;
	char question[128];
	int32_t slen, rlen;
	int32_t numsub = 0;
    int32_t ask = 1;
	int32_t c;
	filerange_t match[1];
	match[0].start = oldpoint;
	match[0].end = curbuf->size;

	char_t searchtext[64];
	char_t replacetext[64];
	searchtext[0] = '\0';
	replacetext[0] = '\0';
	
	if (!get_input("search: ", searchtext, 64, 1))
		return; 

	if (!get_input("replace: ", replacetext, 64, 1))
		return;

	slen = str_len(searchtext);
	rlen = str_len(replacetext);

	sprintf(question, "replace '%s' with '%s'? ", searchtext, replacetext);

	/* scan through file from point */
	numsub = 0;
	while (1) {
		found = search_forward((char*)searchtext, match, 1);
		if (found == -1) {
			curbuf->point = (lastpoint == -1 ? oldpoint : lastpoint);
			break;
		}
		curbuf->point = found;
		/* search_forward places point at end of search, move to start of search */
		curbuf->point -= slen;

		if (ask) {
			msg("(y)es, (n)o, (!)do the rest, (q)uit");
			clrtoeol();

questionprompt:
			update_display();
			c = getch();
			switch (c) {
			case 'y': /* yes, substitute */
				break;
			case 'n': /* no, don't substitute */
				curbuf->point = found;
				continue;
			case '!': /* yes/no stop asking */
				ask = 0;
				break;
			case 0x1B: /* esc key; fallthrough*/
				flushinp();
			case 'q': /* controlled exit */
				return;
			default:
				msg("command not recognized");
				goto questionprompt;
			}
		}
		lastpoint = curbuf->point;
		match[0].start = lastpoint;
		replace_string(curbuf, searchtext, replacetext, slen, rlen);
		numsub++;
	}
	msg("%d substitutions", numsub);	
}