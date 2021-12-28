/*
 * strutils.c, editor, Morgan Peterson, Public Domain, 2021
 */

#include "header.h"

#if GCC_VERSION>=5004000 || CLANG_VERSION>=4000000
#define addu __builtin_add_overflow
#else
static inline bool addu(size_t a, size_t b, size_t *c) {
	if (SIZE_MAX - a < b)
		return false;
	*c = a + b;
	return true;
}
#endif

#define BUFFER_SIZE 1024

void
buffer_init(strbuf_t *b)
{
  memset(b, 0, sizeof(*b));
}

static int8_t
buffer_reserve(strbuf_t *b, size_t size)
{
  /* ensure minimal buffer size, to avoid repeated realloc(3) calls */
	if (size < BUFFER_SIZE)
		size = BUFFER_SIZE;
	if (b->size < size) {
		size = MAX(size, b->size*2);
		char *data = realloc(b->data, size);
		if (!data)
			return 0;
		b->size = size;
		b->data = data;
	}
	return 1;
}

static int8_t
buffer_grow(strbuf_t *b, size_t size)
{
  size_t len;
  if (!addu(b->len, size, &len))
    return 0;
  return buffer_reserve(b, len);
}

static int8_t
buffer_insert(strbuf_t *b, size_t pos, const void *c, size_t len)
{
	if (pos > b->len)
		return 0;
	if (len == 0)
		return 1;
	if (!buffer_grow(b, len))
		return 0;
	size_t move = b->len - pos;
	if (move > 0)
		memmove(b->data + pos + len, b->data + pos, move);
	memcpy(b->data + pos, c, len);
	b->len += len;
	return 1;
}

int8_t
buffer_append(strbuf_t *b, const char *c, size_t len)
{
  return buffer_insert(b, b->len, c, len);
}

int8_t
buffer_terminate(strbuf_t *b)
{
  return !b->data || b->len == 0 || b->data[b->len-1] == '\0' || buffer_append(b, "\0", 1);
}

void
buffer_release(strbuf_t *b)
{
  if (!b)
    return;
  free(b->data);
  buffer_init(b);
}

char*
buffer_move(strbuf_t *b)
{
	char *data = b->data;
	buffer_init(b);
	return data;
}

int32_t
strn_cmp(const char_t *s1, const char_t *s2, int32_t n) {
  int32_t i = 0;
  while (s1[i] == s2[i]) {
    if (i < n) {
      if (s1[i] == '\0' || s2[i] == '\0')
        break;
      i++;
    } else {
      break;
    }
  }
  if (s1[i] == '\0' || s2[i] == '\0')
    return 0;
  else
    return -1;
}

void
strn_cpy(void *s1, void *s2, int32_t n) {
  char_t *target = (char_t *)s1;
  char_t *source = (char_t *)s2;

  for (int32_t i = 0; i < n; i++, target++, source++) {
    if (!s2)
      break;
    *target = *source;
  }
  *(--target) = '\0';
}

char_t*
str_dup(const char_t *org)
{
    int org_size;
    static char_t *dup;
    char_t *dup_offset;

    /* Allocate memory for duplicate */
    org_size = strlen((char*)org);
    dup = (char_t *)malloc(sizeof(char_t)*org_size+1);
    if( dup == NULL)
        return( (char_t *)NULL);

    /* Copy string */
    dup_offset = dup;
    while(*org)
    {
        *dup_offset = *org;
        dup_offset++;
        org++;
    }
    *dup_offset = '\0';

    return dup;
}

char_t*
strn_cat(char_t *s1, char_t *s2, uint32_t n)
{
  if((s1 == NULL) && (s2 == NULL))
    return NULL;
  char_t *dest = s1;
  /* find the end of destination */
  while(*dest != '\0')
  {
    dest++;
  }
  while (n--)
  {
    if (!(*dest++ = *s2++))
    {
      return s1;
    }
  }
  *dest = '\0';
  return s1;
}
