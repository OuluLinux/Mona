/*  Font

    Copyright (C) 2003-2004, Alexander Zaprjagaev <frustum@frustum.org>
    Modified by Abdul Bezrati <abezrati@hotmail.com>
*/

#ifndef __FONT_H__
#define __FONT_H__

#include "TextureUtils.h"

class CFont {
public:
	CFont();
	~CFont();

	void printProportional(double widthRatio, double heightRatio,
						   double width,       double height,
						   double r,  double g, double b,
						   const String& string);

	void Print(double x,     double y,
			   double width, double height,
			   double r,     double g, double b,
			   const String& string);

	void printSubString(double x,     double y,
						double width, double height,
						double r,     double g, double b,
						int   start, int end,
						const String& string);


	Tuple2i getStringSizes(const String& string);
	bool    Load(const char* fontPath);
	int     getMaxFittingLength(const String& string, int bounds);
	int     GetHeight();
	const   int* getCharHorizontalGlyphs() const;

private:
	Texture   fontTexture;
	int       spaces[256],
			  fontHeight;
	GLint     fontBaseRange;
};

#endif /* __FONT_H__ */
