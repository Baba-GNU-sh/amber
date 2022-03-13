#include "fontgen.hpp"

int
main(void)
{
	BitmapFontGenerator generator;
	generator.generate("/usr/share/fonts/truetype/freefont/FreeMono.ttf",
	                   48,
	                   "font.png",
	                   "font.txt");
}