#include "header.h"

enum {
  WHITE,
  OFFWHITE,
  DARKRED,
  GREEN,
  OLIVE,
  BRIGHTBLUE,
  DARKGREY,
  NAVY,
  BLACK,
  LIGHTGREY,
  BRIGHTRED,
  PINK,
  PURPLE,
  ORANGE,
  DARKBLUE
};

color_t colors[] = {
	{"white", 255, "#EEEEEEE"},
  {"offwhite", 252, "#d0d0d0"},
	{"darkred", 124, "#AF0000"},
  {"green", 28, "#008700"},
	{"olive", 64, "#5F8700"},
	{"brightblue", 31, "#0087AF"},
	{"darkgrey", 102, "#878787"},
	{"navy", 24, "#005F87"},
	{"black", 238, "#444444"},
	{"lightgrey", 250, "#BCBCBC"},
	{"brightred", 160, "#D70000"},
	{"pink", 162, "#D70087"},
	{"purple", 91, "#8700AF"},
	{"orange", 166, "#D75F00"},
	{"darkblue", 25, "#005FAF"},
};

void
init_colors(void)
{
  init_pair(HL_BACKGROUND, colors[BRIGHTBLUE].eightbit, colors[WHITE].eightbit);
  init_pair(HL_NORMAL, colors[BLACK].eightbit, colors[WHITE].eightbit);
  init_pair(HL_SYMBOL, colors[DARKBLUE].eightbit, colors[WHITE].eightbit);
  init_pair(HL_NUMBER, colors[ORANGE].eightbit, colors[WHITE].eightbit);
  init_pair(HL_DOUBLE_QUOTE, colors[OLIVE].eightbit, colors[WHITE].eightbit);
  init_pair(HL_SINGLE_QUOTE, colors[GREEN].eightbit, colors[WHITE].eightbit);
  init_pair(HL_COMMENT, colors[DARKGREY].eightbit, colors[WHITE].eightbit);
  init_pair(HL_MLCOMMENT, colors[DARKGREY].eightbit, colors[WHITE].eightbit);
  init_pair(HL_MODELINE, colors[NAVY].eightbit, colors[OFFWHITE].eightbit);
  init_pair(HL_KEYWORD, colors[PINK].eightbit, colors[WHITE].eightbit);
}

