/*
 * strutils.c, editor, Morgan Peterson, Public Domain, 2021
 */

#include "header.h"

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

