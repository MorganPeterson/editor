/*
 * strutils.c, editor, Morgan Peterson, Public Domain, 2021
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <signal.h>
#include <ctype.h>
#include <curses.h>

#include "header.h"

#define BUFFER_SIZE 1024

void
buffer_init(strbuf_t *b)
{
  (void)memset(b, 0, sizeof(*b));
}

static bool
buffer_reserve(strbuf_t *b, size_t size)
{
	char *data = NULL;

	size_t sze = MAX(size, (b->size*2)*sizeof(char));

	/* ensure minimal buffer size, to avoid repeated realloc(3) calls */
	if (sze < BUFFER_SIZE)
		sze = BUFFER_SIZE;

	if (b->size >= sze)
		return false;

	if ((data = (char*)realloc(b->data, sze)) == NULL)
		return false;

	b->size = sze;
	b->data = data;

	return true;
}

static bool
buffer_grow(strbuf_t *b, size_t size)
{
  return buffer_reserve(b, size);
}

static bool
buffer_insert(strbuf_t *b, size_t pos, char c, size_t len)
{
	if (pos > b->len)
		return false;
	if (len == 0)
		return true;
	if (!buffer_grow(b, len))
		return false;
	size_t move = b->len - pos;
	if (move > 0)
		memmove(b->data + pos + len, b->data + pos, move);
	memcpy(b->data + pos, &c, len);
	b->len += len;
	return true;
}

bool
buffer_append(strbuf_t *b, char c, size_t len)
{
  return buffer_insert(b, b->len, c, len);
}

bool
buffer_terminate(strbuf_t *b)
{
  return !b->data || b->len == 0 || b->data[b->len-1] == '\0' || buffer_append(b, '\0', 1);
}

void
buffer_release(strbuf_t *b)
{
  if (!b)
    return;
  free(b->data);
  (void)memset(b, 0, sizeof(*b));
}

char*
buffer_move(strbuf_t *b)
{
	char *data = b->data;
	(void)memset(b, 0, sizeof(*b));
	return data;
}

int32_t
str_len(const char_t *s)
{
    int32_t count = 0;
    while (*s != (char_t)'\0') {
        count++;
        s++;
    }
    return count;
}

int32_t
strn_cmp(const char_t *s1, const char_t *s2, int32_t n)
{
	int32_t i = 0;
	char_t eol = (char_t)'\0';

	while (s1[i] == s2[i]) {
		if (i < n) {
			if (s1[i] == eol || s2[i] == eol)
				break;
			i++;
		} else {
			break;
		}
	}
	if (s1[i] == eol || s2[i] == eol)
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
  *(--target) = (char_t)'\0';
}

/*@null@*/
char_t*
str_dup(const char_t *org)
{
  if (org == NULL)
    return NULL;

  int32_t org_size;
  char_t *dup;
  char_t *dup_offset;

  /* Allocate memory for duplicate */
  org_size = str_len(org);
  dup = (char_t *)malloc(sizeof(char_t)*org_size+1);
  if( dup == NULL)
    return NULL;

  /* Copy string */
  dup_offset = dup;
  while(*org != (char_t)'\0')
  {
    *dup_offset = *org;
    dup_offset++;
    org++;
  }
  *dup_offset = (char_t)'\0';

  return dup;
}

void
strn_cat(char_t *s1, const char_t *s2, uint32_t n)
{
	char_t eol = (char_t)'\0';

	if((s1 == NULL) && (s2 == NULL))
		return;
	char_t *dest = s1;
	/* find the end of destination */
	while(*dest != eol) {
    	dest++;
	}
	while (n-- > 0) {
		if ((*dest++ = *s2++) == eol) {
			return;
		}
	}
	*dest = eol;
}

int32_t
str_str(const char_t *haystack, const char_t *needle)
{
    int32_t nTemp = 0;
    int32_t nStrLen = str_len(haystack);
    int32_t nStrSubLen = str_len(needle);
    for(int32_t i=0; i<nStrLen-nStrSubLen; i++)
    {
        nTemp = i;
        for(int32_t j=0; j<nStrSubLen; j++)
        {

            if(haystack[nTemp]==needle[j])
            {
                if(j==nStrSubLen-1)
                    return 1;
                nTemp++;
            }
            else
                break;
        }
    }
    return 0;
}
