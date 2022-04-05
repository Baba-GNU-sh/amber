#include "fontgen.hpp"
#include <iostream>

int
main(int argc, char *argv[])
{
	const char *filename = "font.png";
	BitmapFontGenerator generator;
	generator.generate(argv[1],
	                   16,
	                   "fontmap.bmp",
	                   "fontmap.txt");
	
	std::cout << "Rendered to " << filename << '\n';
}