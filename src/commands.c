/*
 * commands.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern buffer_t *curbuf;

void
insert(char_t *c) {
  /* char plus null */
  if (curbuf->gap_start == curbuf->gap_end && !grow_gap(curbuf, 1))
    return;

  curbuf->point = move_gap(curbuf, curbuf->point);
  *curbuf->gap_start = *c;
  curbuf->gap_start++;
  curbuf->point++;
}

