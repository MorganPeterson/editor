/*
 * buffer.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

extern buffer_t *headbuf;

int32_t
grow_gap(buffer_t *b, int32_t n) {
  if (n < (b->gap_end - b->gap_start))
    return 0;

  char_t *new;
  int32_t front = b->gap_start - b->buf_start;
  int32_t aft = b->gap_end - b->buf_start;
  int32_t blen = b->buf_end - b->buf_start;

  n = n < MIN_GAP_SIZE ? DEFAULT_BUFFER_SIZE : n;
  uint64_t newlen = blen + n;

  if (newlen > MAX_SIZE_T) {
    return 0;
  }

  if (blen == 0) {
    new = (char_t*)calloc(newlen, sizeof(char_t));
  } else {
    new = realloc(b->data, newlen * sizeof(char_t));
  }
  if (new == NULL) {
    return 0;
  }
  b->data = new;
  b->buf_start = &b->data[0];
  b->gap_start = b->buf_start + front;
  b->buf_end = b->buf_start + blen;
  b->gap_end = b->buf_start + newlen;
  while (aft < blen--)
    *--b->gap_end = *--b->buf_end;
  b->buf_end = b->buf_start + newlen;
  return 1;
}

int32_t
pos(buffer_t *b, register char_t *c) {
  /* return position in the buffer of the pointer */
  return (c - b->buf_start - (c < b->gap_end ? 0 : b->gap_end - b->gap_start));
}

char_t*
ptr(buffer_t *b, register int32_t offset) {
  /* return pointer to the buffer given the offset */
  if (offset < 0)
    return b->buf_start;
  return (b->buf_start + offset + (b->buf_start + offset < b->gap_start ? 0 : b->gap_end - b->gap_start));
}

int32_t
move_gap(buffer_t *b, int32_t offset) {
  char_t *p = ptr(b, offset);
  while (p < b->gap_start)
    *--b->gap_end = *--b->gap_start;
  while (b->gap_end < p)
    *b->gap_start++ = *b->gap_end++;
  return pos(b, b->gap_end);
}

buffer_t*
init_buffer(void) {
  buffer_t *b = (buffer_t*) malloc(sizeof(buffer_t));
  if (b == NULL)
    return NULL;

  b->data = NULL;
  b->next = NULL;
  b->buf_start = 0;
  b->gap_start = 0;
  b->buf_end = 0;
  b->gap_end = 0;
  b->file_name[0] = '\0';
  b->buf_name[0] = '\0';
  b->point = 0;
  b->cpoint = 0;
  b->psize = 0;
  b->size = 0;
  b->page_start = 0;
  b->page_end = 0;
  b->reframe = 0;
  b->row = 0;
  b->col = 0;
  b->flags = 0x00;

  if (!grow_gap(b, DEFAULT_BUFFER_SIZE))
    return NULL;
  memset(b->data, 0, DEFAULT_BUFFER_SIZE);
  return b;
}

void
make_buffer_name(char_t *bn, char_t *fn) {
  char_t *p = fn;
  /* go to end of file name */
  while (*p != 0)
    ++p;
  /* unwind to seperator */
  while (p != fn && p[-1] != '/' && p[-1] != '\\')
    --p;
  strn_cpy(bn, p, BNAME_MAX);
}

buffer_t*
find_buffer(char_t *n, int32_t flag) {
  buffer_t *x;
  buffer_t *y;

  x = headbuf;
  while (x != NULL) {
    if (strn_cmp(n, x->buf_name, BNAME_MAX) == 0)
      return x;
    x = x->next;
  }

  if (flag) {
    x = init_buffer();
    if (headbuf == NULL) {
      headbuf = x;
    } else if (strn_cmp(headbuf->buf_name, n, BNAME_MAX)) {
      x->next = headbuf;
      headbuf = x;
    } else {
      for (y = headbuf; y->next != NULL; y = y->next) {
        if (strn_cmp(y->next->buf_name, n, BNAME_MAX) > 0)
          break;
      }
      x->next = y->next;
      y->next = x;
    }
    strn_cpy(x->buf_name, n, BNAME_MAX);
  }
  return x;
}

void
insert_string(buffer_t *b, char_t *s, int32_t *len) {
  (void)grow_gap(b, *len);
  int32_t p = pos(b, b->gap_start);

  b->point = move_gap(b, b->point);
  b->gap_start += *len;
  strn_cpy(b->data + p, s, *len);

}

void
attach_buf_win(window_t *w, buffer_t *b) {
  w->buf = b;
}

