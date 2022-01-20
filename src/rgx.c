/*
 * regex.c, editor, Morgan Peterson, Public Domain, 2021
 */

#include "header.h"
#include "rgx.h"

static char*
parse_until(const char **s, const char *until)
{
  strbuf_t buf;
  buffer_init(&buf);
  size_t len = strlen(until);
  int8_t escaped = 0;

  for (; **s && (!memchr(until, **s, len) || escaped); (*s)++) {
    if (!escaped && **s == '\\') {
      escaped = 1;
      continue;
    }
    char c = **s;
    if (escaped) {
      escaped = 0;

      if (c == '\n')
        continue;

      if (c == 'n') {
        c = '\n';
      } else if (c == 't') {
        c = '\t';
      } else {
        int8_t delim = *(int8_t*)memchr(until, c, len);
        if (!delim)
          buffer_append(&buf, '\\', 1);
      }
    }
    if (!buffer_append(&buf, c, 1)) {
      buffer_release(&buf);
      return NULL;
    }
  }
  buffer_terminate(&buf);
  return buffer_move(&buf);
}

static char*
parse_delimited(const char **s)
{
  char delim[2] = { **s, '\0'};
  if (!delim[0] || isspace((char_t)delim[0]))
    return NULL;
  (*s)++;
  char *chunk = parse_until(s, delim);
  if (**s == delim[0])
    (*s)++;
  return chunk;
}

static Regex*
regex_new(void)
{
  Regex *r = calloc(1, sizeof(Regex));
  if (!r)
    return NULL;
  return r;
}

void
regex_free(Regex *r)
{
  if (!r)
    return;
  regfree(&r->regex);
  free(r);
}

static int32_t
regex_compile(Regex *rx, const char *str) {
  // int32_t flags = REG_EXTENDED|REG_NEWLINE|REG_ICASE;
  int32_t flags = REG_EXTENDED|REG_NEWLINE;
  int32_t r = regcomp(&rx->regex, str, flags);

  if (r || r == -1)
    /* match nothing */
    return -1;
  return r;
}

static Regex*
editor_regex(const char *pattern)
{
  if (!pattern)
    return NULL;
  Regex *r = regex_new();
  if (!r)
    return NULL;
  if (regex_compile(r, pattern) != 0) {
    regex_free(r);
    return NULL;
  }
  return r;
}

Regex*
parse_regex(const char **s)
{
  const char *before = *s;
  char *pattern = parse_delimited(s);
  if (!pattern && *s == before)
    return NULL;
  Regex *r = editor_regex(pattern);
  free(pattern);
  return r;
}

int32_t
regex_match(Regex *r, const char *data, int32_t flags)
{
  return regexec(&r->regex, data, 0, NULL, flags);
}
