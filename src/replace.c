#include "header.h"

extern buffer_t *curbuf;

void
replace_string(buffer_t *b, char_t *s, char_t *r, int32_t slen, int32_t rlen)
{
	int32_t found = b->point + slen;
	if (rlen > slen) {
		move_gap(b, found);
		if (rlen - slen < b->gap_end - b->gap_start)
			grow_gap(b, rlen - slen);
		b->gap_start = b->gap_start + (rlen - slen);
	} else if (slen > rlen) {
		move_gap(b, found);
		b->gap_start = b->gap_start - (slen - rlen);
	}

	memcpy(ptr(b, b->point), r, rlen * sizeof(char_t));
	add_mode(b, B_MODIFIED);
	add_undo(b, UNDO_REPLACE, curbuf->point, s, r);
	curbuf->point = found - (slen - rlen);
}