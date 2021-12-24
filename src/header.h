/*
 * header.h, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#ifndef HEADER_H
#define HEADER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <signal.h>
#include <ctype.h>
#include <sys/stat.h>
#include <curses.h>
#include <regex.h>

#define MAX_SIZE_T ((unsigned long) (size_t) ~0)
#define EPOS ((size_t)-1)
#define MIN(x,y)  ((x) < (y) ? (x) : (y))
#define MAX(a, b)  ((a) < (b) ? (b) : (a))

#define DEFAULT_BUFFER_SIZE 512
#define MSGBUF 512
#define MSGLINE (LINES-1)
#define K_BUFFER_LENGTH 256
#define MIN_GAP_SIZE 16
#define FNAME_MAX 256
#define BNAME_MAX 16

#define ID_DEFAULT 1
#define ID_SYMBOL 2
#define ID_MODELINE 3

#define FWD_SEARCH 1
#define BWD_SEARCH 2

typedef enum {
	B_MODIFIED = 0x01,
	B_OVERWRITE = 0x02, /* overwite mode */
} buffer_flags_t;

typedef unsigned char char_t;

typedef struct buffer_t buffer_t;
typedef struct strbuf_t strbuf_t;
typedef struct window_t window_t;
typedef struct keymap_t keymap_t;
typedef struct Regex Regex;
typedef struct filerange_t filerange_t;

struct filerange_t {
	size_t start;  /**< Absolute byte position. */
	size_t end;    /**< Absolute byte position. */
};

struct Regex {
  regex_t regex;
};

struct buffer_t {
  buffer_t *next;
  char_t *data;
  char_t *gap_start;
  char_t *gap_end;
  char_t *buf_start;
  char_t *buf_end;
  char_t file_name[FNAME_MAX];
  char_t buf_name[BNAME_MAX];
  int32_t point;
  int32_t cpoint;
  int32_t size;                 /* size of text minus gap */
  int32_t psize;                /* previous size of text */
  int32_t page_start;
  int32_t page_end;
  int32_t reframe;
  int32_t row;
  int32_t col;
  buffer_flags_t flags;
};

struct strbuf_t {
  char *data;
  size_t len;
  size_t size;
};

struct window_t {
  window_t *next;
  buffer_t *buf;
  int32_t top;
  int32_t rows;
  int32_t point;
  int32_t page_start;
  int32_t page_end;
  int32_t row;
  int32_t col;
  int32_t update;
};

struct keymap_t {
  char *key_desc;
  char *key_bytes;
  void (*func)(void);
};

void term_signal(int32_t n);
void die(const char *s, int32_t code);
void quit(void);
void fatal(char *msg);
window_t *new_window(void);
void buf_to_win(window_t *w);
void win_to_buffer(window_t *w);
int32_t grow_gap(buffer_t *b, int32_t n);
int32_t move_gap(buffer_t *b, int32_t offset);
int32_t pos(buffer_t *b, register char_t *c);
char_t *ptr(buffer_t *b, register int32_t offset);
buffer_t *init_buffer(void);
char_t *read_file(char_t* file, int32_t *len);
void strn_cpy(void *s1, void *s2, int32_t n);
int32_t strn_cmp(const char_t *s1, const char_t *s2, int32_t n);
buffer_t *find_buffer(char_t *n, int32_t flag);
void insert_string(buffer_t *w, char_t * s, int32_t len, int32_t flag);
void make_buffer_name(char_t *bn, char_t *fn);
void one_window(window_t *w);
void attach_buf_win(window_t *w, buffer_t *b);
void free_windows(void);
char_t *get_key(keymap_t *keys, keymap_t **key_return);
int32_t move_gap(buffer_t *b, int32_t offset);
void insert(char_t *c);
void update_display(void);
void display_prompt_and_response(char *prompt, char *response);
int32_t line_down(buffer_t *b, int32_t offset);
int32_t line_up(buffer_t *b, int32_t offset);
int32_t utflen(int32_t s);
int32_t prev_utflen(void);
void display_search_result(int32_t fnd, int32_t point, int8_t dir, char *prompt, char *search);
int32_t display_utf(buffer_t *b, int32_t n);
void msg(char *msg, ...);
void left(void);
void right(void);
void up(void);
void down(void);
void delete(void);
void backspace(void);
void pagedown(void);
void pageup(void);
void savebuffer(void);
void search(void);
int32_t save(char_t *fn);
int32_t get_input(char *prompt, char_t *buf, int32_t nbuf, int32_t flag);
void writefile(void);
int32_t insert_file(char_t *fn, int32_t flag);
void insertfile(void);
void findfile(void);
void beginning_of_buffer(void);
void end_of_buffer(void);
void clear_buffer(void);
buffer_t *find_buffer_fname(char_t *fn);
void disassociate_buffer(window_t *w);
void associate_buffer_to_win(buffer_t *b, window_t *w);
void buffer_init(strbuf_t *b);
int8_t buffer_append(strbuf_t *b, const char *c, size_t len);
int8_t buffer_terminate(strbuf_t *b);
void buffer_release(strbuf_t *b);
char *buffer_move(strbuf_t *b);
Regex* parse_regex(const char **s);
void regex_free(Regex *r);
int32_t search_forward(const char *str, filerange_t pmatch[], size_t mrange);
int32_t search_backward(const char *str, filerange_t pmatch[], size_t mrange);

#endif
