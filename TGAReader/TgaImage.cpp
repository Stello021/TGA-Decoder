#include "TgaImage.h"
#include <cstdint>
#include <memory>
#include <fstream>
#include<iostream>
#include "TgaData.h"

TGAImage::TGAImage() : width(0), height(0), bytesPerPixel(0)
{
}

TGAImage::TGAImage(int32_t w, int32_t h, int32_t bpp) : width(w), height(h), bytesPerPixel(bpp)
{
	uint64_t numberOfBytes = width * height * bytesPerPixel;
	data.resize(numberOfBytes, 0);
}

TGAImage::TGAImage(const TGAImage& img) : width(img.width), height(img.height), bytesPerPixel(img.bytesPerPixel), data(img.data)
{
}


TGAImage::~TGAImage()
{
}

TGAImage& TGAImage::operator=(const TGAImage& img)
{
	if (this != &img)
	{
		width = img.width;
		height = img.height;
		bytesPerPixel = img.bytesPerPixel;
		data = img.data;
	}
	return *this;
}

bool TGAImage::ReadTGAFile(const std::string filename)
{
	//Input file opening 
	std::ifstream in;
	in.open(filename, std::ios::binary);
	if (!in.is_open())
	{
		std::cerr << "Can't open file" << filename << "\n";
		in.close();
		return false;
	}
	//Read header from input file
	TGAHeader header;
	in.read(reinterpret_cast<char*>(&header), sizeof(header));
	if (!in.good())
	{
		in.close();
		std::cerr << "An error occured while reading the header \n";
		return false;
	}

	width = header.ImageWidth;
	height = header.ImageHeigth;
	bytesPerPixel = header.PixelDepth >> 3; // Pixel Depth is in bits, 1 bytes = 8 bits  ( 2^3 in bin). Shift by 3 to pass from bits to bytes
	if (width <= 0)
	{
		in.close();
		std::cerr << "Fail to read Width value\n";
		return false;
	}
	else if (height <= 0)
	{
		in.close();
		std::cerr << "Fail to read Height value\n";
		return false;
	}
	else if (bytesPerPixel != GRAYSCALE && bytesPerPixel != RGB && bytesPerPixel != RGBA)
	{
		in.close();
		std::cerr << "Fail to read Pixel Depth value\n";
		return false;
	}
	uint64_t numberOfBytes = width * height * bytesPerPixel;
	data.resize(numberOfBytes, 0);
	if (header.ImageType == 2 || header.ImageType == 3)
	{
		in.read(reinterpret_cast<char*>(data.data()), numberOfBytes);
		if (!in.good())
		{
			in.close();
			std::cerr << "An error occured while reading data\n";
			return false;
		}
	}


	return false;
}

