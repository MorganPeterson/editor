/*
 * undo.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: femto (Public Domain, 2017 by Hugh Barney)
 */

#include "header.h"

extern buffer_t *curbuf;
extern window_t *curwin;
extern keymap_t key_map;

static undo_t*
new_undo(void)
{
	undo_t *up = (undo_t *)malloc(sizeof(undo_t));

	up->prev = NULL;
	up->point = 0;
	up->str = NULL;
  up->rep = NULL;
	up->type = UNDO_NONE;

	return up;
}

/* append a string to the undo structure member str */
void
append_undo_string(undo_t *up, char_t *str)
{
	char_t *newbuf;
	int newlen, buflen;
	int len = strlen((char *)str);

	/* if undo string is not yet created, malloc enough memory and copy it in */
	if (up->str == NULL) {
    up->str = str_dup(str);
		return;
	}

  /* undo string already exists, adjust memory and append str onto it */
	buflen = strlen((char *)up->str);
	newlen = buflen + len + 1;
  newbuf = realloc(up->str, newlen * sizeof(char_t));
	up->str = newbuf;
  strn_cat(up->str, str, len);
}

void
free_undos(undo_t *up)
{
  undo_t *prev = up;
  undo_t *u;

  while (prev != NULL) {
    if (prev->str != NULL)
      free(prev->str);
    if (prev->rep != NULL)
      free(prev->rep);
    u = prev;
    prev = u->prev;
    free(u);
  }
}

void
add_undo(buffer_t *b, undotype_t type, int32_t p, char_t *s, char_t *r)
{
  int32_t len = 1;

	len = strlen((char *)s);

	/* handle insert, accumulate inserts as long as they are next to the last insert */
	if (b->undo != NULL && b->undo->type == type && type == UNDO_INSERT && (b->undo->point + 1) == p) {
		b->undo->point = p; /* update it */
		append_undo_string(b->undo, s);
	/* handle backspace, accumulate backspaces as long as they are next to the last backspace */
	} else if (b->undo != NULL && b->undo->type == type && type == UNDO_BACKSPACE && (b->undo->point - len) == p) {
		b->undo->point = p; /* update it */
		append_undo_string(b->undo, s);
	/* handle delete-char, accumulate deletes as long as they are at the point of the last delete */
	} else if (b->undo != NULL && b->undo->type == type && type == UNDO_DELETE && (b->undo->point) == p) {
		b->undo->point = p; /* update it */
		append_undo_string(b->undo, s);
	/* handle insert_at(), accumulate insert_at()s as long as they are next to the last insert_at() */
	} else if (b->undo != NULL && b->undo->type == type && type == UNDO_INSAT && (b->undo->point) == p) {
		b->undo->point = p; /* update it */
		append_undo_string(b->undo, s);
	} else {
		undo_t *up = new_undo();
    if (up == NULL)
      return;
		up->prev = b->undo;
		b->undo = up;
		up->type = type;
		up->point = p;
		up->str = str_dup(s);
		if (type == UNDO_REPLACE)
			up->rep = str_dup(r);
	}
}

int32_t
get_undo_again(char_t *input)
{
  keymap_t *key_return;

	input = get_key(&key_map, &key_return);

	if (key_return != NULL) {
		if (key_return->func == undocmd) {
			return 1;
		} else {
			(key_return->func)();
			return 0;
		}
	} else {
		if (*input > 31 || *input == 0x0A || *input == 0x09)
			insert(input);
		else
			msg("key not bound");
	}
	return 0;
}

/*
 * A special insert used as the undo of delete char (C-d or DEL)
 * this is where the char is inserted at the point and the cursor
 * is NOT moved on 1 char.  This MUST be a seperate function so that
 *   INSERT + BACKSPACE are matching undo pairs
 *   INSERT_AT + DELETE are matching undo pairs
 * Note: This function is only ever called by execute_undo to undo a DEL.
 */
static void
insert_at(char_t *input)
{
	char_t the_char[2]; /* the inserted char plus a null */

	if (curbuf->gap_start == curbuf->gap_end && !grow_gap(curbuf, 2 * sizeof(char_t)))
		return;
	curbuf->point = move_gap(curbuf, curbuf->point);


	/* overwrite if mid line, not EOL or EOF, CR will insert as normal */
	if ((curbuf->flags & B_OVERWRITE) && *input != '\r' && *(ptr(curbuf, curbuf->point)) != '\n' && curbuf->point < pos(curbuf,curbuf->buf_end) ) {
		*(ptr(curbuf, curbuf->point)) = *input;
		if (curbuf->point < pos(curbuf, curbuf->buf_end))
			++curbuf->point;
		/* FIXME - overwite mode not handled properly for undo yet */
	} else {
		the_char[0] = *input == '\r' ? '\n' : *input;
		the_char[1] = '\0'; /* null terminate */
		*curbuf->gap_start++ = the_char[0];
		curbuf->point = pos(curbuf, curbuf->gap_end);
		curbuf->point--; /* move point back to where it was before, should always be safe */
		/* the point is set so that and undo will DELETE the char */
		add_undo(curbuf, UNDO_INSAT, curbuf->point, the_char, NULL);
	}

	add_mode(curbuf, B_MODIFIED);
}

undo_t*
execute_undo(undo_t *up, char_t *input)
{
  int len;
	int sz;
	int i;
	int before, after;
	char_t the_char[7]; /* the deleted char, allow 6 unsigned chars plus a null */

	len = strlen((char *)up->str);

	if (curbuf->undo_cnt == -1) curbuf->undo_cnt = len;

  switch(up->type) {
    case UNDO_INSERT:
      curbuf->point = up->point - (len - curbuf->undo_cnt);
      before = curbuf->point;
      backspace();
      after = curbuf->point;
      /*
       * we could have backspaced over a multibyte UTF8 char so we need
       * to calculate the delta based on point which is set inside the backspace
       * function.
       */
      sz = before - after;
      curbuf->undo_cnt -= sz;
      if (curbuf->undo_cnt > 0)
        return up; /* more left to undo on this undo string */
      break;

    case UNDO_BACKSPACE:
      /* load up insert with char at str[undo_cnt-1] */
      curbuf->point = up->point + (len - curbuf->undo_cnt);
      sz = prev_utflen_n(up->str, curbuf->undo_cnt - 1);

      /* inserting back, highest byte first, for UTF8 char (as if entered at the keyboard) */
      for (i = 0; i < sz; i++) {
        the_char[0] = up->str[curbuf->undo_cnt - sz + i];
        the_char[1] = '\0';
        input = the_char;
        insert(input);
      }
      curbuf->undo_cnt -= sz;
      if (curbuf->undo_cnt > 0)
        return up; /* more left to undo on this undo string */
      break;


    case UNDO_DELETE:
      /* load up insert_at() with char at str[undo_cnt-1] */
      curbuf->point = up->point; /* point should always be the same */
      sz = prev_utflen_n(up->str, curbuf->undo_cnt - 1);

      /*
       * As we are using insert_at we must insert lowest byte first
       * (the opposite of insert) for a multibyte UTF8 char.
       * This has a strange effect on the list-undos report as the
       * bytes are backwards and so dont get recognised as the UTF8 chars they represent
       */
      for (i = 1; i <= sz; i++) {
        the_char[0] = up->str[curbuf->undo_cnt - i];
        the_char[1] = '\0';
        input = the_char;
        insert_at(input);
      }

      curbuf->undo_cnt -= sz;
      if (curbuf->undo_cnt > 0)
        return up; /* more left to undo on this undo string */
      break;

    case UNDO_INSAT:
      curbuf->point = up->point; /* point should always be the same */
      before = (curbuf->buf_end - curbuf->buf_start) - (curbuf->gap_end - curbuf->gap_start);
      delete();
      /*
       * we could have deleted a multibyte UTF8 char so we need to calculate the delta.
       * this can not be done using point as it does not changes for DEL and INSAT.
       * hence why we use document_size() to get the before and after sizes.
       */
      after = (curbuf->buf_end - curbuf->buf_start) - (curbuf->gap_end - curbuf->gap_start);
      sz = before - after;
      curbuf->undo_cnt -= sz;

      if (curbuf->undo_cnt > 0)
        return up; /* more left to undo on this undo string */
      break;

      /* opposite of a kill-region is a yank (paste) */
    case UNDO_KILL:
      curbuf->point = up->point;
      insert_string(curbuf, up->str, strlen((char*)up->str), 1);
      break;

    case UNDO_YANK:
      curbuf->point = up->point;
      curbuf->mark = up->point + strlen((char*)up->str);
      killregion();
      break;

    case UNDO_REPLACE:
      curbuf->point = up->point;
      replace_string(curbuf, up->rep, up->str, str_len(up->rep), str_len(up->str));
      break;
  }

	curbuf->undo_cnt = -1;
	return up->prev;
}

