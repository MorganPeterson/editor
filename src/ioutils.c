/*
 * ioutils.c, editor, Morgan Peterson, Public Domain, 2021
 */

#include "header.h"

char_t*
read_file(char_t* file, int32_t *len)
{
	FILE* f = fopen((char*)file, "rb");

	if (!f) {
		return NULL;
	}

	struct stat st;

  if (fstat(fileno(f), &st) != 0) {
		fclose(f);
		return NULL;
	}

	char_t *data = (char_t*)malloc(st.st_size *sizeof(char_t));

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

