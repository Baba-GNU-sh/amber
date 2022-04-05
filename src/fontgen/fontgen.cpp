#include "fontgen.hpp"

// helper function to convert a normal buffer to a bitmap buffer
unsigned char*
toBitmapBuffer(int* bufferSize,
               const int imageWidth,
               const int imageHeight,
               unsigned char* buffer)
{
	int rowSize = ((24 * imageWidth + 31) / 32) * 4;
	int padding = 0;
	while ((imageWidth * 3 + padding) % 4 != 0)
		padding++;
	*bufferSize = rowSize * imageHeight;
	unsigned char* bitmapBuffer = new unsigned char[*bufferSize];
	memset(bitmapBuffer, 0, *bufferSize);

	for (int y = 0; y < imageHeight; ++y) {
		for (int x = 0; x < imageWidth; ++x) {
			// position in buffer
			int pos = y * imageWidth * 4 + x * 4;
			// position in padded bitmap buffer
			int newpos =
			  (imageHeight - y - 1) * (imageWidth * 3 + padding) + x * 3;
			bitmapBuffer[newpos + 0] = buffer[pos + 2]; // swap r and b
			bitmapBuffer[newpos + 1] = buffer[pos + 1]; // g stays
			bitmapBuffer[newpos + 2] = buffer[pos + 0]; // swap b and r
		}
	}
	return bitmapBuffer;
}

bool
BitmapFontGenerator::generate(const std::string& fontFilename,
                              const int fontSize,
                              const std::string& bitmapFilename,
                              const std::string& widthsFilename)
{
	FT_Library lib;
	FT_Error error;
	FT_Face face;
	FT_UInt glyphIndex;

	// init freetype
	error = FT_Init_FreeType(&lib);
	if (error != FT_Err_Ok) {
		std::cout << "BitmapFontGenerator > ERROR: FT_Init_FreeType failed, "
		             "error code: "
		          << error << std::endl;
		return false;
	}

	// load font
	error = FT_New_Face(lib, fontFilename.c_str(), 0, &face);
	if (error == FT_Err_Unknown_File_Format) {
		std::cout << "BitmapFontGenerator > ERROR: failed to open file \""
		          << fontFilename << "\", unknown file format" << std::endl;
		return false;
	} else if (error) {
		std::cout << "BitmapFontGenerator > ERROR: failed to open file \""
		          << fontFilename << "\", error code: " << error << std::endl;
		return false;
	}

	// set font size
	error = FT_Set_Pixel_Sizes(face, 0, fontSize);
	if (error) {
		std::cout
		  << "BitmapFontGenerator > failed to set font size, error code: "
		  << error << std::endl;
	}

	// create bitmap font
	int imageWidth = (fontSize + 2) * 16;
	int imageHeight = (fontSize + 2) * 8;

	// create a buffer for the bitmap
	unsigned char* buffer = new unsigned char[imageWidth * imageHeight * 4];
	memset(buffer, 0, imageWidth * imageHeight * 4);

	// create an array to save the character widths
	int* widths = new int[128];

	// we need to find the character that goes below the baseline by the biggest
	// value
	int maxUnderBaseline = 0;
	for (int i = 32; i < 127; ++i) {
		// get the glyph index from character code
		glyphIndex = FT_Get_Char_Index(face, i);

		// load the glyph image into the slot
		error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if (error) {
			std::cout
			  << "BitmapFontGenerator > failed to load glyph, error code: "
			  << error << std::endl;
		}

		// get the glyph metrics
		const FT_Glyph_Metrics& glyphMetrics = face->glyph->metrics;

		// find the character that reaches below the baseline by the biggest
		// value
		int glyphHang = (glyphMetrics.horiBearingY - glyphMetrics.height) / 64;
		if (glyphHang < maxUnderBaseline) {
			maxUnderBaseline = glyphHang;
		}
	}

	// draw all characters
	for (int i = 0; i < 128; ++i) {
		// get the glyph index from character code
		glyphIndex = FT_Get_Char_Index(face, i);

		// load the glyph image into the slot
		error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
		if (error) {
			std::cout
			  << "BitmapFontGenerator > failed to load glyph, error code: "
			  << error << std::endl;
		}

		// convert to an anti-aliased bitmap
		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		if (error) {
			std::cout
			  << "BitmapFontGenerator > failed to render glyph, error code: "
			  << error << std::endl;
		}

		// save the character width
		widths[i] = face->glyph->metrics.width / 64;

		// find the tile position where we have to draw the character
		int x = (i % 16) * (fontSize + 2);
		int y = (i / 16) * (fontSize + 2);
		x += 1; // 1 pixel padding from the left side of the tile
		y += (fontSize + 2) - face->glyph->bitmap_top + maxUnderBaseline - 1;

		// draw the character
		const FT_Bitmap& bitmap = face->glyph->bitmap;
		for (int xx = 0; xx < bitmap.width; ++xx) {
			for (int yy = 0; yy < bitmap.rows; ++yy) {
				unsigned char r = bitmap.buffer[(yy * (bitmap.width) + xx)];
				buffer[(y + yy) * imageWidth * 4 + (x + xx) * 4 + 0] = r;
				buffer[(y + yy) * imageWidth * 4 + (x + xx) * 4 + 1] = r;
				buffer[(y + yy) * imageWidth * 4 + (x + xx) * 4 + 2] = r;
				buffer[(y + yy) * imageWidth * 4 + (x + xx) * 4 + 3] = 255;
			}
		}
	}

	// convert the buffer to the bitmap format
	int paddedSize = 0;
	unsigned char* bitmapBuffer =
	  toBitmapBuffer(&paddedSize, imageWidth, imageHeight, buffer);

	BitMapFileHeaderStruct bmfh;
	bmfh.type = 0x4d42; // 0x4d42 = 'BM'
	bmfh.filesize = 14 + sizeof(BitMapInfoHeaderStruct) + paddedSize;
	bmfh.reserved1 = 0; // not used
	bmfh.reserved2 = 0; // not used
	bmfh.offset = 54;

	BitMapInfoHeaderStruct bmih;
	bmih.infoheadersize = sizeof(BitMapInfoHeaderStruct);
	bmih.width = imageWidth;
	bmih.height = imageHeight;
	bmih.planes = 1;
	bmih.bitsperpixel = 24;
	bmih.compression = 0;
	bmih.imagesize = 0;
	bmih.xpixelspermeter = 0;
	bmih.ypixelspermeter = 0;
	bmih.colorsused = 0;
	bmih.importantcolorsused = 0;

	FILE* file;
	if ((file = fopen(bitmapFilename.c_str(), "wb")) == NULL) {
		std::cout << "BitmapFontGenerator > failed to save bitmap file \""
		          << bitmapFilename << "\"" << std::endl;
		return false;
	}

	// write file header
	fwrite(&bmfh.type, sizeof(short), 1, file);
	fwrite(&bmfh.filesize, sizeof(int), 1, file);
	fwrite(&bmfh.reserved1, sizeof(short), 1, file);
	fwrite(&bmfh.reserved2, sizeof(short), 1, file);
	fwrite(&bmfh.offset, sizeof(int), 1, file);

	// write info header
	fwrite(&bmih, sizeof(BitMapInfoHeaderStruct), 1, file);

	// write data
	fwrite(bitmapBuffer, sizeof(unsigned char), paddedSize, file);

	fclose(file);
	delete[] bitmapBuffer;
	delete[] buffer;

	// save the widths file
	std::ofstream ofs;
	ofs.open(widthsFilename);
	if (ofs.is_open()) {
		for (int i = 0; i < 128; ++i) {
			ofs << widths[i] << std::endl;
		}
		ofs.close();
	} else {
		std::cout << "BitmapFontGenerator > failed to save widths file \""
		          << bitmapFilename << "\"" << std::endl;
		return false;
	}

	delete[] widths;

	// shutdown freetype
	error = FT_Done_FreeType(lib);
	if (error != FT_Err_Ok) {
		std::cout << "BitmapFontGenerator > ERROR: FT_Done_FreeType failed, "
		             "error code: "
		          << error << std::endl;
		return false;
	}
	return true;
}
