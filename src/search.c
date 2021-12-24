#include "header.h"

extern buffer_t *curbuf;

static char*
alloc_buf(int32_t len)
{
  char *buf = calloc(1,  len);
	if (!buf)
		return NULL;
  memcpy(buf, curbuf->gap_end, len);
  buf[len] = '\0';
  return buf;
}

int32_t
search_forward(const char *str, filerange_t pmatch[], size_t mrange)
{
  Regex *r = parse_regex(&str);
  if (r == NULL)
    return REG_NOMATCH;
  int32_t pos = move_gap(curbuf, pmatch[0].end);
  int32_t len = curbuf->buf_end - curbuf->gap_end;
  char *buf = calloc(1,  len+1);
	if (!buf)
		return REG_NOMATCH;
  memcpy(buf, curbuf->gap_end, len);
  buf[len] = '\0';

	char *cur = buf, *end = buf + len;
	int ret = REG_NOMATCH;
	regmatch_t match[1];

	for (size_t junk = len; len > 0; len -= junk, pos += junk) {
		ret = regexec(&r->regex, cur, mrange, match, REG_EXTENDED);
		if (!ret) {
			for (size_t i = 0; i < mrange; i++) {
        pmatch[i].start = match[i].rm_so == -1 ? EPOS : pos + match[i].rm_so - 1;
				pmatch[i].end = match[i].rm_eo == -1 ? EPOS : pos + match[i].rm_eo;
			}
			break;
		}
		char *next = memchr(cur, 0, len);

		if (!next)
			break;

    while (!*next && next != end)
			next++;

    junk = next - cur;
		cur = next;
	}
	free(buf);
  regex_free(r);
	return ret;
}

int32_t
search_backward(const char *str, filerange_t pmatch[], size_t mrange)
{
  int32_t eflags = REG_EXTENDED;
  Regex *r = parse_regex(&str);
  (void)move_gap(curbuf, curbuf->point);
  int32_t pos = 0;
  int32_t len = curbuf->gap_start - curbuf->buf_start;
  char *buf = calloc(1,  len+1);
	if (!buf)
		return REG_NOMATCH;
  memcpy(buf, curbuf->buf_start, len);
  buf[len] = '\0';
	char *cur = buf, *end = buf + len;
	int ret = REG_NOMATCH;
	regmatch_t match[1];
	for (size_t junk = len; len > 0; len -= junk, pos += junk) {
		char *next;
		if (!regexec(&r->regex, cur, mrange, match, REG_EXTENDED)) {
			ret = 0;
			for (size_t i = 0; i < mrange; i++) {
				pmatch[i].start = match[i].rm_so == -1 ? EPOS : pos + match[i].rm_so - 1;
				pmatch[i].end = match[i].rm_eo == -1 ? EPOS : pos + match[i].rm_eo;
			}

			if (match[0].rm_so == 0 && match[0].rm_eo == 0) {
				/* empty match at the beginning of cur, advance to next line */
				next = strchr(cur, '\n');
				if (!next)
					break;
				next++;
			} else {
				next = cur + match[0].rm_eo;
			}
		} else {
			next = memchr(cur, 0, len);
			if (!next)
				break;
			while (!*next && next != end)
				next++;
		}
		junk = next - cur;
		cur = next;
		if (cur[-1] == '\n')
			eflags &= ~REG_NOTBOL;
		else
			eflags |= REG_NOTBOL;
	}
	free(buf);
  regex_free(r);
	return ret;
}

