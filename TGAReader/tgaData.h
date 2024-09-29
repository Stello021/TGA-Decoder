#pragma once
#include <cstdint>

struct TGAHeader
{
	int8_t IdLength;
	int8_t ColorMapType;
	int8_t ImageType;
	//Color map specification
	int16_t FirstEntryIndex;
	int16_t ColorMapLength;
	int8_t ColorMapDepth; //number of bits per color map entry
	//Image specification
	int16_t OriginX;
	int16_t OriginY;
	int16_t ImageWidth;
	int16_t ImageHeigth;
	int8_t PixelDepth; //in bits
	int8_t ImageDescriptor; 
	//IMAGE DESCRIPTOR BITS DATA
	//5th bit -> pixels ordered from left-to-right (1) or right-to-left (0)
	//6th bit -> pixels ordered from top-to-bottom (1) or bottom-to-top (0)

};

struct TGAColor
{
	union
	{
		struct
		{
			uint8_t R, G, B, A;
		};
		uint8_t Raw[4];
		uint32_t Value;
	};
	int32_t BytesPerPixel;

	TGAColor() : Value(0), BytesPerPixel(1)
	{

	}

	TGAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a), BytesPerPixel(4)
	{

	}

	TGAColor(int value, int bytesPerPixel) : Value(value), BytesPerPixel(bytesPerPixel)
	{

	}

	TGAColor(const uint8_t* p, int bytesPerPixel) : Value(0), BytesPerPixel(bytesPerPixel)
	{
		for (int i = 0; i < bytesPerPixel; i++)
		{
			Raw[i] = p[i];
		}
	}

	//copy ctor
	TGAColor(const TGAColor& color) : Value(color.Value), BytesPerPixel(color.BytesPerPixel)
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