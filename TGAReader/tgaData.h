#pragma once
#include "Common.h"

struct TGAHeader
{
	int8 IdLength;
	int8 ColorMapType;
	int8 ImageType;
	//Color map specification
	int16 FirstEntryIndex;
	int16 ColorMapLength;
	int8 ColorMapDepth; //number of bits per color map entry
	//Image specification
	int16 OriginX;
	int16 OriginY;
	int16 ImageWidth;
	int16 ImageHeigth;
	int8 PixelDepth;
	int8 ImageDescriptor;
};

struct TGAColor
{
	union
	{
		struct
		{
			uint8 R, G, B, A;
		};
		uint8 Raw[4];
		uint32 Value;
	};
	int32 BytesPerPixel;

	TGAColor() : Value(0), BytesPerPixel(1)
	{

	}

	TGAColor(uint8 r, uint8 g, uint8 b, uint8 a) : R(r), G(g), B(b), A(a), BytesPerPixel(4)
	{

	}

	TGAColor(int value, int bytesPerPixel) : Value(value), BytesPerPixel(bytesPerPixel)
	{

	}

	TGAColor(const uint8* p, int bytesPerPixel) : Value(0), BytesPerPixel(bytesPerPixel)
	{
		for (int i = 0; i < bytesPerPixel; i++)
		{
			Raw[i] = p[i];
		}
	}

	//copy ctor
	TGAColor(TGAColor &color) : Value(color.Value), BytesPerPixel(color.BytesPerPixel)
	{

	}

	//assignment operator
	TGAColor& operator=(const TGAColor& c)
	{
		if (this != &c)
		{
			BytesPerPixel = c.BytesPerPixel;
			Value = c.Value;
		}

		return *this;
	}

};