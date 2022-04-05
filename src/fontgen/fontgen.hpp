#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <fstream>
#include <iostream>
#include <string>

// cross platform bitmap headers
struct BitMapFileHeaderStruct
{
	short type;
	int filesize;
	short reserved1;
	short reserved2;
	int offset;
};

struct BitMapInfoHeaderStruct
{
	int infoheadersize;
	int width;
	int height;
	short planes;
	short bitsperpixel;
	int compression;
	int imagesize;
	int xpixelspermeter;
	int ypixelspermeter;
	int colorsused;
	int importantcolorsused;
};

class BitmapFontGenerator
{
  public:
	static bool generate(const std::string& fontFilename,
	                     const int fontSize,
	                     const std::string& bitmapFilename,
	                     const std::string& widthsFilename);

  private:
};
