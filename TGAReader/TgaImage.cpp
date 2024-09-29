#include "TgaImage.h"
#include <cstdint>
#include <memory>
#include <fstream>
#include <iostream>
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
	//Parsing Header
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
	else if (header.ImageType == 10 || header.ImageType == 11)
	{
		if (!LoadRLEData(in))
		{
			in.close();
			std::cerr << "An error occured loading RLE-compressed data\n";
			return false;
		}
	}
	else
	{
		in.close();
		std::cerr << "Unknown file format: " << static_cast<int32_t>(header.ImageType) << "\n";
		return false;
	}

	if (!(header.ImageDescriptor & 0x20)) //0x20 in binary is 0010 0000, so in this way 6th bit of Image Descriptor is isolated 
	{
		FlipVertically();
	}
	if (!(header.ImageDescriptor & 0x10)) //0x10 in binary is 0001 0000, so in this way 5th bit of Image Descriptor is isolated 
	{
		FlipHorizontally();
	}

	in.close();
	return true;
}


bool TGAImage::LoadRLEData(std::ifstream& in)
{
	uint32_t pixelCount = width * height;
	uint32_t currentPixel = 0;
	uint32_t currentByte = 0;
	TGAColor colorBuffer;
	uint8_t chunkHeader = 0;
	//input file stream exctract header previously, now begin from data bytes
	do
	{
		chunkHeader = in.get(); //get first data byte from input file
		if (!in.good())
		{
			std::cerr << "An error occured while reading data\n";
			return false;
		}
		//chunks can contains raw data or RLE data
		if (chunkHeader < MAX_CHUNK_LENGTH) //chunk headers < 128 contains raw data of chunk header + 1 pixels 
		{
			chunkHeader++;
			for (int i = 0; i < chunkHeader; i++)
			{
				in.read(reinterpret_cast<char*>(colorBuffer.Raw), bytesPerPixel);
				if (!in.good())
				{
					std::cerr << "An error occured while reading data\n";
					return false;
				}

				for (int k = 0; k < bytesPerPixel; k++)
				{
					data[currentByte++] = colorBuffer.Raw[k];
				}
				currentPixel++;
				if (currentPixel > pixelCount)
				{
					std::cerr << "pixels read are out of count\n";
					return false;
				}
			}
		}
		else //chunk headers >= 128 indicates RLE-encoded chunks where the same color is repeated chunk header - 127 times
		{
			chunkHeader -= MAX_CHUNK_LENGTH - 1;
			in.read(reinterpret_cast<char*>(colorBuffer.Raw), bytesPerPixel);
			if (!in.good())
			{
				std::cerr << "An error occured while reading data\n";
				return false;
			}
			for (int i = 0; i < chunkHeader; i++)
			{
				for (int k = 0; k < bytesPerPixel; k++)
				{
					data[currentByte++] = colorBuffer.Raw[k];
				}
				currentPixel++;
				if (currentPixel > pixelCount)
				{
					std::cerr << "pixels read are out of count\n";
					return false;
				}
			}
		}

	} while (currentPixel < pixelCount);
	return true;
}

bool TGAImage::WriteTGAFile(const std::string filename, bool rle)
{
	uint8_t developerArea[4] = { 0, 0, 0, 0 };
	uint8_t extensionArea[4] = { 0, 0, 0, 0 };
	uint8_t footer[18] = { 'T','R','U','E','V','I','S','I','O','N','-','X','F','I','L','E','.','\0' };

	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open())
	{
		std::cerr << "Can't open file " << filename << "\n";
		out.close();
		return false;
	}

	TGAHeader header;
	memset(reinterpret_cast<void*>(&header), 0, sizeof(header));
	header.PixelDepth = bytesPerPixel << 3;
	header.ImageWidth = width;
	header.ImageHeigth = height;
	header.ImageType = (bytesPerPixel == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2));
	header.ImageDescriptor = 0x20;
	out.write(reinterpret_cast<char*>(& header), sizeof(header));
	if (!out.good())
	{
		out.close();
		std::cerr << "Fail to write the TGA file\n";
		return false;
	}

	if (rle)
	{
		if (!CompressRawData(out))
		{
			out.close();
			std::cerr << "Fail to unload raw data\n";
			return false;
		}
	}
	else
	{
		out.write(reinterpret_cast<char*>(data.data()), width * height * bytesPerPixel);
		if (!out.good())
		{
			out.close();
			std::cerr << "Fail to unload raw data\n";
			return false;
		}
	}

	out.write(reinterpret_cast<char*>(developerArea), sizeof(developerArea));
	if (!out.good())
	{
		out.close();
		std::cerr << "Fail to write the TGA file\n";
		return false;
	}
	out.write(reinterpret_cast<char*>(extensionArea), sizeof(extensionArea));
	if (!out.good())
	{
		out.close();
		std::cerr << "Fail to write the TGA file\n";
		return false;
	}
	out.write(reinterpret_cast<char*>(footer), sizeof(footer));
	if (!out.good())
	{
		out.close();
		std::cerr << "Fail to write the TGA file\n";
		return false;
	}
	out.close();
	return true;
}

bool TGAImage::CompressRawData(std::ofstream& out) 
{
	//identifies sequences of consecutive pixels that have same color and encodes them as a "run",
	// storing color once followed by the number of consecutive pixels
	uint32_t pixelCount = width * height;
	uint32_t currentPixel = 0;

	while (currentPixel < pixelCount)
	{
		uint32_t chunkStart = currentPixel * bytesPerPixel;
		uint32_t currentByte = currentPixel * bytesPerPixel;
		uint8_t run_length = 1;
		bool raw = true;

		while (currentPixel + run_length < pixelCount && run_length < MAX_CHUNK_LENGTH)
		{
			//Chunk construction
			bool nextPixelIsEqual = true;
			for (int i = 0; nextPixelIsEqual && i < bytesPerPixel; i++)
			{
				nextPixelIsEqual = data[currentByte + i] == data[currentByte + i + bytesPerPixel];
			}
			currentByte += bytesPerPixel;
			if (run_length == 1)
			{
				raw = !nextPixelIsEqual;
			}
			if (raw && nextPixelIsEqual)
			{
				run_length--;
				break;
			}
			if (!raw && !nextPixelIsEqual)
			{
				break;
			}
			run_length++;
		}
		currentPixel += run_length;
		out.put(raw ? run_length - 1 : run_length + (MAX_CHUNK_LENGTH - 1));
		if (!out.good())
		{
			std::cerr << "Fail to unload RLE data \n";
			return false;
		}
		out.write(reinterpret_cast<char*>(data.data() + chunkStart), raw ? run_length * bytesPerPixel : bytesPerPixel);
		if (!out.good())
		{
			std::cerr << "Fail to unload RLE data \n";
			return false;
		}
	}
	return true;
}