#include "header.h"

extern buffer_t *curbuf;

#define STRBUF_L 256
#define STRBUF_M 64
#define STRBUF_S 16

extern window_t *curwin;

char *m_qreplace = "Replace '%s' with '%s' ? ";
char *m_rephelp = "(y)es, (n)o, (!)do the rest, (q)uit";

void
query_replace(void)
{
  int32_t c, rlen, slen;
  int32_t found = REG_NOMATCH;
  int32_t last_point = -1;
  int32_t numsub = 0;
  int32_t orig_point = curbuf->point;
  int32_t ask = TRUE;

	filerange_t match[1];
	match[0].start = curbuf->point;
	match[0].end = curbuf->point;

  char_t searchtext[STRBUF_M];
  char_t replace[STRBUF_M];
  char question[STRBUF_L];
  searchtext[0] = '\0';
  replace[0] = '\0';

	if (!get_input("replace: ", searchtext, STRBUF_M, 1))
		return;

	if (!get_input("with: ", replace, STRBUF_M, 1))
		return;

  slen = str_len(searchtext);
  rlen = str_len(replace);

  sprintf(question, m_qreplace, searchtext, replace);

  while (TRUE) {
    found = search_forward((char*)searchtext, match, 1);
    if (found != 0) {
      curbuf->point = (last_point == -1 ? orig_point : last_point);
      break;
    }
    curbuf->point = match[0].start;
    if (ask == TRUE) {
      msg(question);
      clrtoeol();

    qprompt:
      display(curwin, TRUE);
      c = getch();

      switch (c) {
      case 'y':
        break;
      case 'n':
        curbuf->point = match[0].end;
        continue;
      case '!':
        ask = FALSE;
        break;
      case 0x1B:
        flushinp();
      case 'q':
        return;
      default:
        msg(m_rephelp);
        goto qprompt;
      }
    }
    last_point = curbuf->point;
    replace_string(curbuf, searchtext, replace, slen, rlen);
    numsub++;
  }
	msg("%d substitutions", numsub);
}

void
replace_string(buffer_t *bp, char_t *s, char_t *r, int32_t slen, int32_t rlen)
{
	/*
	 * we call this function with the point set at the start of the search string
	 * search places the point at the end of the search
	 * to claculate the value of found we add on the length of the search string
	 */
	int32_t found = bp->point + slen;

	if (rlen > slen) {
		move_gap(bp, found);
		/*check enough space in gap left */
		if (rlen - slen < bp->gap_end - bp->gap_start)
			grow_gap(bp, rlen - slen);
		/* shrink gap right by r - s */
		bp->gap_start = bp->gap_start + (rlen - slen);
	} else if (slen > rlen) {
		move_gap(bp, found);
		/* stretch gap left by s - r, no need to worry about space */
		bp->gap_start = bp->gap_start - (slen - rlen);
	}

	/* now just overwrite the chars at point in the buffer */
	memcpy(ptr(bp, bp->point), r, rlen * sizeof (char_t));
	add_mode(bp, B_MODIFIED);

	add_undo(curbuf, UNDO_REPLACE, curbuf->point, s, r);
	curbuf->point = found - (slen - rlen); /* set point to end of replacement */
}
