/*
 * keys.c, editor, Morgan Peterson, Public Domain, 2021
 * Derived from: atto, (Public Domain, 2017 by Hugh Barney)
 * Derived from: Anthony's Editor January 93, (Public Domain 1991, 1993 by Anthony Howe)
 */

#include "header.h"

keymap_t key_map[] = {
  {"C-a beginning-of-line", "\x01", linebegin},
  {"C-b backward-char", "\x02", left},
  {"C-d delete", "\x04", delete},
  {"C-e end-of-line", "\x05", lineend},
  {"C-f forward-char", "\x06", right},
  {"C-h backspace", "\x08", backspace},
  {"C-k kill-to-eol", "\x0B", killtoeol},
  {"C-n next-line", "\x0E", down},
  {"C-p previous-line", "\x10", up},
  {"C-r search-backward", "\x12", search},
  {"C-s search-forward", "\x13", search},
  {"C-u undo", "\x15", undocmd},
  {"C-v forward-page", "\x16", pagedown},
  {"C-w kill-region", "\x17", killregion},
  {"C-y yank", "\x19", yank},
  {"C-x 1 delete-other-windows", "\x18\x31", deleteotherwindows},
  {"C-x 2 split-window", "\x18\x32", splitwindow},
  {"C-x i insert-file", "\x18\x69", insertfile},
  {"C-x k kill-buffer", "\x18\x6B", killbuffer},
  {"C-x n next-buffer", "\x18\x6E", nextbuffer},
  {"C-x o other-window", "\x18\x6F", otherwindow},
  {"C-space set-mark", "\x00", setmark},
  {"C-x C-c exit", "\x18\x03", quit},
  {"C-x C-f find-file", "\x18\x06", findfile},
  {"C-x C-n next-buffer", "\x18\x0E", nextbuffer},
  {"C-x C-s save-buffer", "\x18\x13", savebuffer},
  {"C-x C-w write-file", "\x18\x17", writefile},
  {"C-x = curosr-position", "\x18\x3D", cursorpos},
  {"esc-c copy-region", "\x1B\x63", copyregion},
  {"esc-i yank", "\x1B\x69", yank},
  {"esc-k kill-region", "\x1B\x6B", killregion},
  {"esc-m set-mark", "\x1B\x6D", setmark},
  {"esc-n next-buffer", "\x1B\x6E", nextbuffer},
  {"esc-o delete-other-windows", "\x1B\x6F", deleteotherwindows},
  {"esc-v backward-page", "\x1B\x76", pageup},
  {"esc-g goto-line", "\x1B\x67", gotoline},
  {"esc-w copy-region", "\x1B\x77", copyregion},
  {"del forward-delete-char", "\x1B\x5B\x33\x7E", delete}, /* Del key */
  {"backspace delete-left", "\x7f", backspace},            /* backspace key */
  {"up previous-line", "\x1B\x5B\x41", up},                /* up arrow */
  {"down next-line", "\x1B\x5B\x42", down},                /* down arrow */
  {"left backward-character", "\x1B\x5B\x44", left},       /* left arrow */
  {"right forward-character", "\x1B\x5B\x43", right},      /* right arrow */
  {"pgdn forward-page", "\x1B\x5B\x36\x7E", pagedown},     /* PgDn key */
  {"pgup backward-page", "\x1B\x5B\x35\x7E", pageup},      /* PgUp key */
  {NULL, NULL, NULL},
};

char_t
*get_key(keymap_t *keys, keymap_t **key_return) {
	keymap_t *k;
	int submatch;
	static char_t buffer[K_BUFFER_LENGTH];
	static char_t *record = buffer;

	*key_return = NULL;

	/* if recorded bytes remain, return next recorded byte. */
	if (*record != '\0') {
		*key_return = NULL;
		return record++;
	}
	/* reset record buffer. */
	record = buffer;

	do {
		/* read and record one byte. */
		*record++ = (unsigned)getch();
		*record = '\0';

		/* if recorded bytes match any multi-byte sequence... */
		for (k = keys, submatch = 0; k->key_bytes != NULL; ++k) {
			char_t *p, *q;

			for (p = buffer, q = (char_t *)k->key_bytes; *p == *q; ++p, ++q) {
			        /* an exact match */
				if (*q == '\0' && *p == '\0') {
	    				record = buffer;
					*record = '\0';
					*key_return = k;
					return record; /* empty string */
				}
			}
			/* record bytes match part of a command sequence */
			if (*p == '\0' && *q != '\0') {
				submatch = 1;
			}
		}
	} while (submatch);
	/* nothing matched, return recorded bytes. */
	record = buffer;
	return (record++);
}

int32_t
get_input(char *prompt, char_t *buf, int32_t nbuf, int32_t flag)
{
	int32_t cpos = 0;
	int32_t c;
	int32_t start_col = strlen(prompt);

  display_prompt_and_response(prompt, (char*)buf);

	if (flag == 1) buf[0] = '\0';

	/* if we have a default value print it and go to end of it */
	if (buf[0] != '\0') {
		addstr((char*)buf);
		cpos = strlen((char*)buf);
	}

	for (;;) {
		refresh();
		c = getch();
		/* ignore control keys other than backspace, cr, lf */
		if (c < 32 && c != 0x07 && c != 0x08 && c != 0x0a && c != 0x0d)
			continue;

		switch(c) {
		case 0x0a: /* cr, lf */
		case 0x0d:
			buf[cpos] = '\0';
			return (cpos > 0 ? TRUE : FALSE);

		case 0x07: /* ctrl-g */
			return FALSE;

		case 0x7f: /* del, erase */
		case 0x08: /* backspace */
			if (cpos == 0)
				continue;

			move(MSGLINE, start_col + cpos - 1);
			addch(' ');
			move(MSGLINE, start_col + cpos - 1);
			buf[--cpos] = '\0';
			break;

		default:
			if (cpos < nbuf -1) {
				addch(c);
				buf[cpos++] = c;
				buf[cpos] ='\0';
			}
			break;
		}
	}
}
