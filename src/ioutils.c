/*
 * ioutils.c, editor, Morgan Peterson, Public Domain, 2021
 */

#include "header.h"

extern buffer_t *curbuf;

int32_t
posix_file(char_t *fn)
{
  if (fn[0] == '_')
    return 0;
  for (; *fn != '\0'; ++fn) {
    if (!isalnum(*fn) && *fn != '.' && *fn != '_' && *fn != '-' && *fn != '/')
      return 0;
  }
  return 1;
}

int32_t
save(char_t *fn)
{
  FILE *fp;
	size_t length;

	if (!posix_file(fn)) {
		msg("Not a portable POSIX file name.");
		return 0;
	}
	fp = fopen((char*)fn, "w");
	if (fp == NULL) {
		msg("Failed to open file \"%s\".", fn);
		return 0;
	}

	(void) move_gap(curbuf, 0);

  length = (size_t)(curbuf->buf_end - curbuf->gap_end);

  if (fwrite(curbuf->gap_end, sizeof(char_t), length, fp) != length) {
		msg("Failed to write file \"%s\".", fn);
		return 0;
	}

  if (fclose(fp) != 0) {
		msg("Failed to close file \"%s\".", fn);
		return 0;
	}

  curbuf->flags &= ~B_MODIFIED;

  msg("File \"%s\" %ld bytes saved.", fn, pos(curbuf, curbuf->buf_end));
	return (TRUE);
}

char_t*
read_file(char_t* file, int32_t *len)
{
	struct stat st;
  char_t *data = NULL;

  if (stat((char*)file, &st) < 0)
    return NULL;

  if (MAX_SIZE_T < (size_t)st.st_size)
    return NULL;

	FILE* f = fopen((char*)file, "rb");

	if (!f) {
		return NULL;
	}

  if (fstat(fileno(f), &st) != 0) {
		fclose(f);
		return NULL;
	}

	data = (char_t*)malloc(st.st_size * sizeof(char_t));

	if (!data) {
		fclose(f);
		return NULL;
	}

	if (fread(data, 1, st.st_size, f) != (size_t)st.st_size) {
		fclose(f);
		free(data);
		return NULL;
	}

	fclose(f);
  *len = st.st_size;
	return data;
}

