#include <regex.h>

#ifndef RGX_H
#define RGX_H

typedef struct Regex Regex;
struct Regex {
  regex_t regex;
};

Regex* parse_regex(const char**);
void regex_free(Regex*);
#endif

