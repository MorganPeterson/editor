/*
 * utf.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern buffer_t *curbuf;

enum {
  UTFmax = 6,
	UTFerror = 0xFFFD,  /* decoding error in utf */
  Chartmax = 0x10FFFF
};

#define BADUTF(x) ((x) < 0 || (x) > Chartmax \
                || ((x) & 0xFFFE) == 0xFFFE \
                || ((x) >= 0xD800 && (x) <= 0xDFFF) \
                || ((x) >= 0xFDD0 && (x) <= 0xFDEF))

int32_t
utflen(int32_t c) {
	if(BADUTF(c))
		return 0; /* error */
	else if(c <= 0x7F)
		return 1;
	else if(c <= 0x07FF)
		return 2;
	else if(c <= 0xFFFF)
		return 3;
	else
		return 4;
}

int32_t
prev_utflen(void) {
  int32_t m;
  for (int32_t n=2; n < 5; n++) {
    m = utflen(*(ptr(curbuf, curbuf->point - n)));
    if (m == 0)
      return 1;
    if (-1 < curbuf->point - n &&  m == n) {
      return n;
    }
  }
  return 1;
}

int32_t
prev_utflen_n(char_t *b, int32_t pos)
{
  for (int32_t n=1; n < 4; n++) {
    if (-1 < pos - n &&  utflen(b[pos-n]) == n + 1) {
      return n + 1;
    }
  }
  return 1;
}

int32_t
display_utf(buffer_t *b, int32_t n) {
  int32_t i;
  char buf[UTFmax];

  for (i=0; i < n; i++) {
    buf[i] = *ptr(b, b->page_end + i);
  }
  buf[i] = '\0';
  addstr(buf);
  return n;
}
