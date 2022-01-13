/*
 * syntax.c, editor, Morgan Peterson, Public Domain, 2021
 */

#include "header.h"

#define HLDB_ENTRIES (sizeof(HLDB) / sizeof(HLDB[0]))

int32_t state = HL_NORMAL;
int32_t next_state = HL_NORMAL;
int32_t skip_count = 0;
int32_t prev_sep = 1;

char *C_HL_extensions[] = { ".c", ".h", ".cpp", NULL };
char *JS_HL_extensions[] = { ".js", ".jsx", NULL };
char *GO_HL_extensions[] = { ".go", NULL };
char *SH_HL_extensions[] = { ".sh", NULL };

char *C_HL_keywords[] = {
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case", "NULL",
  "int|", "int32_t|", "int8_t|", "int64_t|", "long|", "double|", "float|",
  "char|", "unsigned|", "signed|", "void|", "#include|", "#define|", "auto",
  "const", "short|", "default", "register", "sizeof|", "volatile|", "goto|",
  "do", "extern", NULL };

char *JS_HL_keywords[] = {
  "abstract", "async", "await", "boolean", "break", "byte", "case", "catch",
  "char", "class", "const", "continue", "debugger", "default", "delete",
  "do", "double", "else", "enum", "export", "extends", "false", "final",
  "finally", "float", "for", "function", "get", "goto", "if", "implements",
  "import", "in", "instanceof", "int", "interface", "let", "long", "native",
  "new", "null", "of", "package", "private", "protected", "public", "return",
  "set", "short", "static", "super", "switch", "synchronized", "this",
  "throw", "throws", "transient", "true", "try", "typeof", "var", "void",
  "volatile", "while", "with", "yield", NULL };

char *GO_HL_keywords[] = {"break", "case", "chan", "const", "continue",
  "default", "defer", "else", "fallthrough", "for", "func", "go", "goto", "if",
  "import", "interface", "map", "package", "range", "return", "select", "struct",
  "switch", "type", "var", "true", "false", "iota", "nil", "bool", "byte",
  "complex64", "complex128", "error", "float32", "float64", "int", "int8",
  "int16", "int32", "int64", "rune", "string", "uint", "uint8", "uint16",
  "uint32", "uint64", "uintptr", "append", "cap", "close", "complex", "copy",
  "delete", "imag", "len", "make", "new", "panic", "print", "println", "real",
  "recover", NULL };

char *SH_HL_keywords[] = {"if", "then", "elif", "else", "fi", "case",
  "in", "esac", "while", "for", "do", "done", "continue", "local", "return",
  "select", "-a", "-b", "-c", "-d", "-e", "-f", "-g", "-h", "-k", "-p", "-r",
  "-s", "-t", "-u", "-w", "-x", "-O", "-G", "-L", "-S", "-N", "-nt", "-ot",
  "-ef", "-o", "-z", "-n", "-eq", "-ne", "-lt", "-le", "-gt", "-ge", NULL };

syntax_t HLDB[] = {
  {
    "c",
    C_HL_extensions,
    C_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
    "javascript",
    JS_HL_extensions,
    JS_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
	  "golang",
	  GO_HL_extensions,
	  GO_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  },
  {
    "shell",
    SH_HL_extensions,
    SH_HL_keywords,
    "#", NULL, NULL,
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS
  }
};

static char_t symbols[] = "{}[]()";

static int32_t
is_separator(int c)
{
  return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];:{}", c) != NULL;
}

int32_t
is_symbol(char_t c)
{
	register char_t *p = symbols;

	for (p = symbols; *p != '\0'; p++)
		if (*p == c) return 1;
	return 0;
}

static char_t*
get_char_at(buffer_t *b, int32_t p)
{
  return ptr(b, p);
}

int32_t
parse_text(buffer_t *b, int32_t p)
{
	if (skip_count-- > 0)
		return state;

	if (b->syntax == NULL)
		return state;

	char_t *now = get_char_at(b, p);
	char_t *next = get_char_at(b, p + 1);
	state = next_state;

	char *mls = b->syntax->multiline_comment_start;
	char *mle = b->syntax->multiline_comment_end;
	char *slc = b->syntax->singleline_comment_start;
	char **kyw = b->syntax->keywords;

  	if (now == NULL)
    	return state;

  	if ((state == HL_KEYWORD || state == HL_KEYWORD2) && is_separator(*now)) {
    	next_state = state = HL_NORMAL;
  	}

  	if (next != NULL) {
    	if (mls != NULL || mle != NULL) {
      		if (state == HL_NORMAL && *now == mls[0] && *next == mls[1]) {
        		skip_count = 1;
        		return (next_state = state = HL_MLCOMMENT);
      		}

      		if (state == HL_MLCOMMENT && *now == mle[0] && *next == mle[1]) {
        		skip_count = 1;
        		next_state = HL_NORMAL;
        		return HL_MLCOMMENT;
      		}
    	}

    	if (slc != NULL) {
      		if (state == HL_NORMAL && *now == '/' && *next == '/') {
        		skip_count = 1;
        		return (next_state = state = HL_COMMENT);
      		}
    	}
  	}

	if (state == HL_COMMENT && *now == '\n')
		return (next_state = HL_NORMAL);

	if (state == HL_NORMAL && *now == '"')
		return (next_state = HL_DOUBLE_QUOTE);

	if (state == HL_DOUBLE_QUOTE && *now == '\\') {
		skip_count = 1;
		return (next_state = HL_DOUBLE_QUOTE);
	}

	if (state == HL_DOUBLE_QUOTE && *now == '"') {
		next_state = HL_NORMAL;
		return HL_DOUBLE_QUOTE;
	}

	if (state == HL_NORMAL && *now == '\'')
		return (next_state = HL_SINGLE_QUOTE);

	if (state == HL_SINGLE_QUOTE && *now == '\\') {
		skip_count = 1;
		return (next_state = HL_SINGLE_QUOTE);
	}

	if (state == HL_SINGLE_QUOTE && *now == '\'') {
		next_state = HL_NORMAL;
		return HL_SINGLE_QUOTE;
	}

	if (state != HL_NORMAL)
		return (next_state = state);

	if (state == HL_NORMAL && *now >= '0' && *now <= '9') {
		next_state = HL_NORMAL;
		return (state = HL_NUMBER);
	}

	if (state == HL_NORMAL && 1 == is_symbol(*now)) {
		next_state = HL_NORMAL;
		prev_sep = 1;
		return (state = HL_SYMBOL);
	}

	if (is_separator(*now)) {
		prev_sep = 1;
		return (next_state = state);
	}

	if (prev_sep) {
		prev_sep = 0;
		int32_t j;
		for (j = 0; kyw[j]; j++) {
			int32_t klen = strlen(kyw[j]);
			int32_t kw2 = kyw[j][klen - 1] == '|';
			if (kw2)
				klen--;
			if (!strncmp((char*)now, kyw[j], klen) && is_separator(*(now + klen))) {
				if (kw2)
					return (next_state = HL_KEYWORD);
				return (next_state = HL_KEYWORD2);
			}
		}
	}
	return (next_state = state);
}

void
set_parse_state(buffer_t * b, int32_t p)
{
  register int32_t po;
  state = HL_NORMAL;
	next_state = HL_NORMAL;
	skip_count = 0;

	for (po=0; po < p; po++)
		parse_text(b, po);
}

void
select_syntax(buffer_t *b)
{
	int32_t j, is_ext;
  	b->syntax = NULL;
  	if (b->file_name == NULL)
    	return;

  	char *ext = strchr((char*)b->file_name, '.');

  	for (unsigned int i=0; i < HLDB_ENTRIES; i++) {
    	syntax_t *s = &HLDB[i];
    	j = 0;
    	while (s->filematch[j]) {
      		is_ext = s->filematch[j][0] == '.';
      		if ((is_ext && ext && !strcmp(ext, s->filematch[j])) ||
				(!is_ext && strstr((char*)b->file_name, s->filematch[j]))) {
        		b->syntax = s;
      			return;
			}
			j++;
    	}
  	}
}
