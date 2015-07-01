///
///	@file bdf2c.c		@brief BDF Font to C source convertor
///
///	Copyright (c) 2009, 2010 by Lutz Sammer.  All Rights Reserved.
///
///	Contributor(s): 
///
///	License: AGPLv3
///
///	This program is free software: you can redistribute it and/or modify
///	it under the terms of the GNU Affero General Public License as
///	published by the Free Software Foundation, either version 3 of the
///	License.
///
///	This program is distributed in the hope that it will be useful,
///	but WITHOUT ANY WARRANTY; without even the implied warranty of
///	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///	GNU Affero General Public License for more details.
///
///	$Id$
//////////////////////////////////////////////////////////////////////////////

///
///	@mainpage
///		bdf2c - converts bdf font files into C include files.
///
///		The Bitmap Distribution Format (BDF) is a file format for
///		storing bitmap fonts. The content is presented as a text file
///		that is intended to be human and computer readable.
///
///	BDF input:
///	@code
///	STARTCHAR A
///	ENCODING 65
///	SWIDTH 568 0
///	DWIDTH 8 0
///	BBX 8 13 0 -2
///	BITMAP
///	00
///	38
///	7C
///	C6
///	C6
///	C6
///	FE
///	C6
///	C6
///	C6
///	C6
///	00
///	00
///	ENDCHAR
///	@endcode
///
///	The result looks like this:
///	@code
///	//  65 $41 'A'
///	//	width 8, bbx 0, bby -2, bbw 8, bbh 13
///	    ________,
///	    __XXX___,
///	    _XXXXX__,
///	    XX___XX_,
///	    XX___XX_,
///	    XX___XX_,
///	    XXXXXXX_,
///	    XX___XX_,
///	    XX___XX_,
///	    XX___XX_,
///	    XX___XX_,
///	    ________,
///	    ________,
///	@endcode
///

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include <assert.h>

#include "ppmhdr.h"

#define VERSION "4"			///< version of this application

//////////////////////////////////////////////////////////////////////////////

int Outline;				///< true generate outlined font

//////////////////////////////////////////////////////////////////////////////

///
///	Create our header file.
///
///	@param out	file stream for output
///
void CreateFontHeaderFile(FILE * out)
{
    register int i;

    fprintf(out,
	"// (c) 2009, 2010 Lutz Sammer, License: AGPLv3\n\n"
	"\t/// bitmap font structure\n" "struct bitmap_font {\n"
	"\tunsigned char Width;\t\t///< max. character width\n"
	"\tunsigned char Height;\t\t///< character height\n"
	"\tunsigned short Chars;\t\t///< number of characters in font\n"
	"\tconst unsigned char *Widths;\t///< width of each character\n"
	"\tconst unsigned short *Index;\t///< encoding to character index\n"
	"\tconst unsigned char *Bitmap;\t///< bitmap of all characters\n"
	"};\n\n");

    fprintf(out, "\t/// @{ defines to have human readable font files\n");
    for (i = 0; i < 256; ++i) {
	fprintf(out, "#define %c%c%c%c%c%c%c%c 0x%02X\n",
	    (i & 0x80) ? 'X' : '_', (i & 0x40) ? 'X' : '_',
	    (i & 0x20) ? 'X' : '_', (i & 0x10) ? 'X' : '_',
	    (i & 0x08) ? 'X' : '_', (i & 0x04) ? 'X' : '_',
	    (i & 0x02) ? 'X' : '_', (i & 0x01) ? 'X' : '_', i);
    }
    fprintf(out, "\t/// @}\n");
}

//////////////////////////////////////////////////////////////////////////////

///
///	Print header for c file.
///
///	@param out	file stream for output
///	@param name	font variable name in C source file
///
void Header(FILE * out, const char *name)
{
    fprintf(out,
	"// Created from bdf2c Version %s, (c) 2009, 2010 by Lutz Sammer\n"
	"//\tLicense AGPLv3: GNU Affero General Public License version 3\n"
	"\n#include \"font.h\"\n\n", VERSION);

    fprintf(out,
	"\t/// character bitmap for each encoding\n"
	"static const unsigned char __%s_bitmap__[] = {\n", name);
}

///
///	Print width table for c file
///
///	@param out		file stream for output
///	@param name		font variable name in C source file
///	@param width_table	width table read from BDF file
///	@param chars		number of characters in width table
///
void WidthTable(FILE * out, const char *name, const unsigned *width_table,
    int chars)
{
    fprintf(out, "};\n\n");

    fprintf(out,
	"\t/// character width for each encoding\n"
	"static const unsigned char __%s_widths__[] = {\n", name);
    while (chars--) {
	fprintf(out, "\t%u,\n", *width_table++);
    }
}

///
///	Print encoding table for c file
///
///	@param out		file stream for output
///	@param name		font variable name in C source file
///	@param encoding_table	encoding table read from BDF file
///	@param chars		number of characters in encoding table
///
void EncodingTable(FILE * out, const char *name,
    const unsigned *encoding_table, int chars)
{
    fprintf(out, "};\n\n");

    fprintf(out,
	"\t/// character encoding for each index entry\n"
	"static const unsigned short __%s_index__[] = {\n", name);
    while (chars--) {
	fprintf(out, "\t%u,\n", *encoding_table++);
    }
}

///
///	Print footer for c file.
///
///	@param out		file stream for output
///	@param name		font variable name in C source file
///	@param width		character width of font
///	@param height		character height of font
///	@param chars		number of characters in font
///
void Footer(FILE * out, const char *name, int width, int height, int chars)
{
    fprintf(out, "};\n\n");
    fprintf(out,
	"\t/// bitmap font structure\n" "const struct bitmap_font %s = {\n",
	name);
    fprintf(out, "\t.Width = %d, .Height = %d,\n", width, height);
    fprintf(out, "\t.Chars = %d,\n", chars);
    fprintf(out, "\t.Widths = __%s_widths__,\n", name);
    fprintf(out, "\t.Index = __%s_index__,\n", name);
    fprintf(out, "\t.Bitmap = __%s_bitmap__,\n", name);
    fprintf(out, "};\n\n");
}


///
///	Dump character.
///
///	@param out	file stream for output
///	@param bitmap	input bitmap
///	@param width	character width
///	@param height	character height
///
void DumpCharacter(FILE * out, unsigned char *bitmap, int width, int height, int yoffset, char * prefix)
{
    int x;
    int y;
    int c;
    int bmheight = yoffset;
    if (bmheight < 0) {
        bmheight = -yoffset;
    }
    bmheight = height - bmheight;

    for (; yoffset > 0; yoffset --) {
        fprintf (out, "\t%s________,________,\n", prefix);
    }
    for (y = 0; y < bmheight; ++y) {
	fprintf (out, "\t%s", prefix); //fputc('\t', out);
	for (x = 0; x < width; x += 8) {
            c = bitmap[ (y-yoffset) * ((width + 7) / 8) + x / 8];
	    //printf("%d = %d\n", y * ((width+7)/8) + x/8, c);
	    if (c & 0x80) {
		fputc('X', out);
	    } else {
		fputc('_', out);
	    }
	    if (c & 0x40) {
		fputc('X', out);
	    } else {
		fputc('_', out);
	    }
	    if (c & 0x20) {
		fputc('X', out);
	    } else {
		fputc('_', out);
	    }
	    if (c & 0x10) {
		fputc('X', out);
	    } else {
		fputc('_', out);
	    }
	    if (c & 0x08) {
		fputc('X', out);
	    } else {
		fputc('_', out);
	    }
	    if (c & 0x04) {
		fputc('X', out);
	    } else {
		fputc('_', out);
	    }
	    if (c & 0x02) {
		fputc('X', out);
	    } else {
		fputc('_', out);
	    }
	    if (c & 0x01) {
		fputc('X', out);
	    } else {
		fputc('_', out);
	    }
	    fputc(',', out);
	}
	fputc('\n', out);
    }
    for (; yoffset < 0; yoffset ++) {
        fprintf (out, "\t%s________,________,\n", prefix);
    }
}



///
///	Hex ascii to integer
///
///	@param p	hex input character (0-9a-fA-F)
///
///	@returns converted integer
///
static inline int Hex2Int(const char *p)
{
    if (*p <= '9') {
	return *p - '0';
    } else if (*p <= 'F') {
	return *p - 'A' + 10;
    } else {
	return *p - 'a' + 10;
    }
}

extern void RotateBitmap(uint8_t *bitmap, int shiftx, int shifty, int width, int height, int glywidth, int glyheight);

#define BITSZ_OF(a) (sizeof (a) * 8)
void RotateBitmap(uint8_t *bitmap, int shiftx, int shifty, int width, int height, int glywidth, int glyheight)
{
    int y;
    uint8_t * p0;
    uint8_t * p1;
    uint8_t * p2;
    uint8_t val;
    size_t byteshift;

    if ((shiftx < 0) || (shiftx >= width)) {
        fprintf(stderr, "Waring: This shiftx isn't supported: 1 w=%d,h=%d (max %d,%d), shiftx=%2d, shifty=%2d; ignored!!\n", glywidth, glyheight, width, height, shiftx, shifty);
        return;
    }
    //if ((shifty > 0) || (shifty + height < glyheight)) {
    if (shifty > 0) {
        fprintf(stderr, "Waring: This shifty isn't supported: 2 w=%d,h=%d (max %d,%d), shiftx=%2d, shifty=%2d; ignored!!\n", glywidth, glyheight, width, height, shiftx, shifty);
        return;
    }
#if DEBUG
    // debug
    if ((size_t)shiftx >= BITSZ_OF (uint8_t)) {
        fprintf(stderr, "Waring: This shiftx: w=%d,h=%d, shiftx=%2d\n", width, height, shiftx);
    }
#endif

    byteshift = shiftx % BITSZ_OF (uint8_t);

    for (y = 0; y < height; ++y) {
        p0 = &bitmap[y * ((width + BITSZ_OF (uint8_t) - 1) / BITSZ_OF (uint8_t))];
        p2 = p0 + ((width + BITSZ_OF (uint8_t) - 1) / BITSZ_OF (uint8_t)) - 1;
        p1 = p2 + 1 - ((shiftx + BITSZ_OF (uint8_t) - 1) / BITSZ_OF (uint8_t));
#if DEBUG
if (shiftx < BITSZ_OF (uint8_t)) {
    assert (p1 == p2);
} else if (shiftx < BITSZ_OF (uint8_t) * 2) {
    assert (p1 + 1 == p2);
}
#endif
        for (; p2 >= p0; p2 --) {
            val = 0;
            if (p1 >= p0) {
                val = *p1;
                assert (byteshift < BITSZ_OF (uint8_t));
                if (byteshift > 0) {
                    val >>= byteshift;
                    if (p1 > p0) {
                        val |= (*(p1-1) << (BITSZ_OF (uint8_t) - byteshift));
                    }
                }
                p1 --;
            }
            *p2 = val;
        }
    }
#if 0
// don't offset here, we do it in DumpCharacter()
    if (shifty) {
        byteshift = (width + BITSZ_OF (uint8_t) - 1) / BITSZ_OF (uint8_t);
        if (shifty < 0) {
            p0 = bitmap;
            p1 = bitmap - shifty * byteshift;
            byteshift = byteshift * (height + shifty);
            memset (p0, 0, p1 - p0);
        } else {
            assert (shifty > 0);
            p1 = bitmap;
            p0 = bitmap + shifty * byteshift;
            byteshift = byteshift * (height - shifty);
        }
        fprintf (stderr, "w=%d,h=%d, shifty=%d, move bytes: %lu, p0=%p, p1=%p\n", width, height, shifty, byteshift, p0, p1);
        memmove (p1, p0, byteshift);
    }
#endif
}

///
///	Outline character.  Create an outline font from normal fonts.
///
///	@param bitmap	input bitmap
///	@param width	character width
///	@param height	character height
///
void OutlineCharacter(unsigned char *bitmap, int width, int height)
{
    int x;
    int y;
    unsigned char *outline;

    outline = alloca(((width + 7) / 8) * height);
    memset(outline, 0, ((width + 7) / 8) * height);
    for (y = 0; y < height; ++y) {
	for (x = 0; x < width; ++x) {
	    // Bit not set check surroundings
	    if (~bitmap[y * ((width + 7) / 8) + x / 8] & (0x80 >> x % 8)) {
		// Upper row bit was set
		if (y
		    && bitmap[(y - 1) * ((width + 7) / 8) +
			x / 8] & (0x80 >> x % 8)) {
		    outline[y * ((width + 7) / 8) + x / 8] |= (0x80 >> x % 8);
		    // Previous bit was set
		} else if (x
		    && bitmap[y * ((width + 7) / 8) + (x -
			    1) / 8] & (0x80 >> (x - 1) % 8)) {
		    outline[y * ((width + 7) / 8) + x / 8] |= (0x80 >> x % 8);
		    // Next bit was set
		} else if (x < width - 1
		    && bitmap[y * ((width + 7) / 8) + (x +
			    1) / 8] & (0x80 >> (x + 1) % 8)) {
		    outline[y * ((width + 7) / 8) + x / 8] |= (0x80 >> x % 8);
		    // below row was set
		} else if (y < height - 1
		    && bitmap[(y + 1) * ((width + 7) / 8) +
			x / 8] & (0x80 >> x % 8)) {
		    outline[y * ((width + 7) / 8) + x / 8] |= (0x80 >> x % 8);
		}
	    }
	}
    }
    memcpy(bitmap, outline, ((width + 7) / 8) * height);
}

/*
    FONTBOUNDINGBOX 16 18 0 -2
    BBX 16 15 0 -1
In the following diagram, the boundary of the font bounding box is dashed, while the boundary of the bitmap bounding box is dotted:
                |<--- width --->|
                      = 16
                -----------------      -
a blank row-->  |               |      ^
a blank row-->  |               |      |
                |...............| -    |
                |///////////////| ^    |
                |///////////////| |    |
                |// 16x15  /////| |    |
                |// bitmap /////| |15  |18 = FONTBOUNDINGBOX height.
                |///////////////| |    |
                |///////////////| |    |
baseline ---->  O---------------- |    |   point "O" = Origin (0,0)
                |///////////////| v    |
              -1|...............| -    |   -1 = yd of BBX.
a blank row-->  |               |      v
              -2-----------------      -   -2 = yd of FONTBOUNDINGBOX.

*/

///
///	Read BDF font file.
///
///	@param bdf	file stream for input (bdf file)
///	@param out	file stream for output (C source file)
///	@param name	font variable name in C source file
///
///	@todo bbx isn't used to correct character position in bitmap
///
void ReadBdf(FILE * bdf, FILE * out, const char *name, const char * fnppm)
{
    char linebuf[1024];
    char *s;
    char *p;
    int fontboundingbox_width = 0;
    int fontboundingbox_height = 0;
    int fontboundingbox_xoff = 0;
    int fontboundingbox_yoff = 0;
    int chars = 0;
    int i;
    int j;
    int n;
    int scanline;
    char charname[1024];
    char fontname[1024];
    int encoding;
    int bbx;
    int bby;
    int bbw;
    int bbh;
    int width;
    unsigned *width_table;
    unsigned *encoding_table;
    unsigned char *bitmap;

    for (;;) {
	if (!fgets(linebuf, sizeof(linebuf), bdf)) {	// EOF
	    break;
	}
	if (!(s = strtok(linebuf, " \t\n\r"))) {	// empty line
	    if (chars > 0) break; else continue;
	}
	// printf("token:%s\n", s);
	if (!strcasecmp(s, "FONTBOUNDINGBOX")) {
	    p = strtok(NULL, " \t\n\r");
	    fontboundingbox_width = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    fontboundingbox_height = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    fontboundingbox_xoff = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    fontboundingbox_yoff = atoi(p);
	} else if (!strcasecmp(s, "FONT")) {
	    p = strtok(NULL, " \t\n\r");
	    strcpy (fontname, p);
	} else if (!strcasecmp(s, "CHARS")) {
	    p = strtok(NULL, " \t\n\r");
	    chars = atoi(p);
	    break;
	}
    }
    /*
       printf("%d * %dx%d\n", chars, fontboundingbox_width,
       fontboundingbox_height);
     */
    bdf2c_fontpic_init (fnppm, chars, fontboundingbox_width, fontboundingbox_height, fontname);
    //
    //	Some checks.
    //
    if (fontboundingbox_width <= 0 || fontboundingbox_height <= 0) {
	fprintf(stderr, "Need to know the character size\n");
	exit(-1);
    }
    if (chars <= 0) {
	fprintf(stderr, "Need to know the number of characters\n");
	exit(-1);
    }
    if (Outline) {			// Reserve space for outline border
	fontboundingbox_width++;
	fontboundingbox_height++;
    }
    //
    //	Allocate tables
    //
    width_table = malloc(chars * sizeof(*width_table));
    if (!width_table) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
    }
    encoding_table = malloc(chars * sizeof(*encoding_table));
    if (!encoding_table) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
    }
    /*	FIXME: needed for proportional fonts.
       offset_table = malloc(chars * sizeof(*offset_table));
       if (!offset_table) {
       fprintf(stderr, "Out of memory\n");
       exit(-1);
       }
     */
    bitmap =
	malloc(((fontboundingbox_width + 7) / 8) * fontboundingbox_height * 2); // double space
    if (!bitmap) {
	fprintf(stderr, "Out of memory\n");
	exit(-1);
    }

    Header(out, name);
    fprintf(out, "// FONTBOUNDINGBOX %d %d %d %d\n", fontboundingbox_width, fontboundingbox_height, fontboundingbox_xoff, fontboundingbox_yoff);

    //int fseek(FILE *stream, long offset, int whence);
    fseek (bdf, 0, SEEK_SET);
    int curline = 0;

    scanline = -1;
    n = 0;
    encoding = -1;
    bbx = 0;
    bby = 0;
    bbw = 0;
    bbh = 0;
    width = INT_MIN;
    strcpy(charname, "unknown character");
    for (;;) {
	if (!fgets(linebuf, sizeof(linebuf), bdf)) {	// EOF
	    break;
	}
	curline ++;
	if (!(s = strtok(linebuf, " \t\n\r"))) {	// empty line
	    continue; //break;
	}
	// printf("token:%s\n", s);
	if (!strcasecmp(s, "STARTCHAR")) {
	    p = strtok(NULL, " \t\n\r");
	    strcpy(charname, p);
	    width = fontboundingbox_width;
	} else if (!strcasecmp(s, "ENCODING")) {
	    p = strtok(NULL, " \t\n\r");
	    encoding = atoi(p);
	} else if (!strcasecmp(s, "DWIDTH")) {
	    p = strtok(NULL, " \t\n\r");
	    width = atoi(p);
	} else if (!strcasecmp(s, "BBX")) {
	    p = strtok(NULL, " \t\n\r");
	    bbw = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    bbh = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    bbx = atoi(p);
	    p = strtok(NULL, " \t\n\r");
	    bby = atoi(p);
	} else if (!strcasecmp(s, "BITMAP")) {
	    fprintf(out, "// %3d $%02x '%s'\n", encoding, encoding, charname);
	    fprintf(out, "//\twidth %d, bbx %d, bby %d, bbw %d, bbh %d\n",
		width, bbx, bby, bbw, bbh);

	    if (n == chars) {
		fprintf(stderr, "Warning: Too many bitmaps for characters, chars=%d, line=%d\n", chars, curline);
		//exit(-1);
	    }
	    if (width == INT_MIN) {
		fprintf(stderr, "character width not specified\n");
		exit(-1);
	    }
	    //
	    //	Adjust width based on bounding box
	    //
	    if (bbx < 0) {
		width -= bbx;
		bbx = 0;
	    }
	    if (bbx + bbw > width) {
		width = bbx + bbw;
	    }
	    if (Outline) {		// Reserve space for outline border
		++width;
	    }
	    width_table[n] = width;
	    encoding_table[n] = encoding;
	    ++n;
	    if (Outline) {		// Leave first row empty
		scanline = 1;
	    } else {
		scanline = 0;
	    }
	    memset(bitmap, 0,
		((fontboundingbox_width + 7) / 8) * fontboundingbox_height);
	} else if (!strcasecmp(s, "ENDCHAR")) {
            char flag_shifted = 0;
            char flag_overflow = 0;
            if (bbx - fontboundingbox_xoff) {
                flag_shifted = 1;
                if (bbx < fontboundingbox_xoff) { // - fontboundingbox_xoff + bbx < 0
                    flag_overflow = 1;
                } else if (bbw + bbx > fontboundingbox_xoff + fontboundingbox_width) { // - fontboundingbox_xoff + bbx + bbw > fontboundingbox_width
                    flag_overflow = 1;
                }
            }
            if (bby + bbh - fontboundingbox_yoff - fontboundingbox_height) {
                flag_shifted = 1;
                if (bby < fontboundingbox_yoff) {
                    flag_overflow = 1;
                } else if ( bby + bbh > fontboundingbox_yoff + fontboundingbox_height) { // fontboundingbox_height - (bby - fontboundingbox_yoff + bbh) < 0
                    flag_overflow = 1;
                }
            }
#if 0
            if (bby + bbh - fontboundingbox_yoff - fontboundingbox_height) {
                flag_shifted = 1;
                RotateBitmap(bitmap, 0, fontboundingbox_height + fontboundingbox_yoff - bbh - bby, fontboundingbox_width,
                    fontboundingbox_height, width, bbh);
            }
#endif
	    if (bbx - fontboundingbox_xoff) {
                flag_shifted = 1;
		RotateBitmap(bitmap, bbx - fontboundingbox_xoff, 0, fontboundingbox_width,
		    fontboundingbox_height, bbw, bbh);
	    }
	    if (Outline) {
		RotateBitmap(bitmap, 1, 0, fontboundingbox_width,
		    fontboundingbox_height, bbw, bbh);
		OutlineCharacter(bitmap, fontboundingbox_width,
		    fontboundingbox_height);
	    }

            bdf2c_fontpic_add (bitmap, fontboundingbox_width, fontboundingbox_height
                , 0, fontboundingbox_height - (bby - fontboundingbox_yoff + bbh)
                , encoding
                , flag_shifted, flag_overflow);
            //fprintf (out, "// fw=%d,fh=%d,yoff=%d\n", fontboundingbox_width, fontboundingbox_height, fontboundingbox_height - (bby - fontboundingbox_yoff + bbh));
            if (flag_overflow) {
                DumpCharacter(out, bitmap, fontboundingbox_width, fontboundingbox_height, 0, "//");
            }
	    DumpCharacter(out, bitmap, fontboundingbox_width, fontboundingbox_height, fontboundingbox_height - (bby - fontboundingbox_yoff + bbh), "");
	    scanline = -1;
	    width = INT_MIN;
	} else {
	    if (scanline >= 0) {
		p = s;
		j = 0;
		while (*p) {
		    i = Hex2Int(p);
		    ++p;
		    if (*p) {
			i = Hex2Int(p) | i * 16;
		    } else {
			bitmap[j + scanline * ((fontboundingbox_width +
				    7) / 8)] = i;
			break;
		    }
		    /* printf("%d = %d\n", 
		       j + scanline * ((fontboundingbox_width + 7)/8), i); */
		    bitmap[j + scanline * ((fontboundingbox_width + 7) / 8)] =
			i;
		    ++j;
		    ++p;
		}
		++scanline;
	    }
	}
    }

    // Output width table for proportional font.
    WidthTable(out, name, width_table, chars);
    // FIXME: Output offset table for proportional font.
    // OffsetTable(out, name, offset_table, chars);
    // Output encoding table for utf-8 support
    EncodingTable(out, name, encoding_table, chars);

    Footer(out, name, fontboundingbox_width, fontboundingbox_height, chars);
    bdf2c_fontpic_clear ();
}

//////////////////////////////////////////////////////////////////////////////

///
///	Print version
///
void PrintVersion(void)
{
    printf("bdf2c Version %s, (c) 2009, 2010 by Lutz Sammer\n"
	"\tLicense AGPLv3: GNU Affero General Public License version 3\n",
	VERSION);
}

///
///	Print usage
///
void PrintUsage(void)
{
    printf("Usage: bdf2c [OPTIONs]\n"
	"\t-h or -?\tPrints this short page on stdout\n"
	"\t-b\tRead bdf file from stdin, write to stdout\n"
	"\t-c\tCreate font header on stdout\n"
	"\t-C file\tCreate font header file\n"
	"\t-n name\tName of c font variable (place it before -b)\n"
	"\t-O\tCreate outline for the font.\n");
    printf("\n\tOnly idiots print usage on stderr\n");
}

///
///	Main test program for bdf2c.
///
///
///	@param argc	number of arguments
///	@param argv	arguments vector
///
int main(int argc, char *const argv[])
{
    const char *name;
    const char *fnppm = "out.ppm";
    FILE * fout = stdout;
    FILE * fin = stdin;

    name = "font";			// default variable name
    //
    //	Parse arguments.
    //
    for (;;) {
	switch (getopt(argc, argv, "bcC:n:i:o:p:hO?-")) {
	    case 'b':			// bdf file name
		ReadBdf(stdin, stdout, name, fnppm);
		continue;
	    case 'i':
		fin = fopen(optarg, "rb");
                if (NULL == fin) {
                    fprintf (stderr, "input file error: %s\n", optarg);
                }
		continue;
	    case 'o':
		fout = fopen(optarg, "wb");
                if (NULL == fin) {
                    fprintf (stderr, "output file error: %s\n", optarg);
                }
		continue;
	    case 'p':
		fnppm = optarg;
		continue;
	    case 'c':			// create header file
		CreateFontHeaderFile(stdout);
		break;
	    case 'C':			// create header file
	    {
		FILE *out;

		if (!(out = fopen(optarg, "w"))) {
		    fprintf(stderr, "Can't open file '%s': %s\n", optarg,
			strerror(errno));
		    exit(-1);
		}
		CreateFontHeaderFile(out);
		fclose(out);
	    }
		continue;
	    case 'n':
		name = optarg;
		continue;
	    case 'O':
		Outline = 1;
		continue;

	    case EOF:
		break;
	    case '?':
	    case 'h':			// help usage
		PrintVersion();
		PrintUsage();
		exit(0);
	    case '-':
		fprintf(stderr, "We need no long options\n");
		PrintUsage();
		exit(-1);
	    case ':':
		PrintVersion();
		fprintf(stderr, "Missing argument for option '%c'\n", optopt);
		exit(-1);
	    default:
		PrintVersion();
		fprintf(stderr, "Unkown option '%c'\n", optopt);
		exit(-1);
	}
	break;
    }
    while (optind < argc) {
	fprintf(stderr, "Unhandled argument '%s'\n", argv[optind++]);
    }

    if (NULL == fin) {
        fprintf (stderr, "input file error\n");
        return -1;
    }
    if (NULL == fout) {
        fprintf (stderr, "output file error\n");
        return -1;
    }
    ReadBdf(fin, fout, name, fnppm);
    return 0;
}
