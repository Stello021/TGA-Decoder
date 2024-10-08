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
	data.resize(numberOfBytes);
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

int TGAImage::GetWidth() const
{
	return width;
}

int TGAImage::GetHeigth() const
{
	return height;
}

int TGAImage::GetBytesPerPixel() const
{
	return bytesPerPixel;
}

uint8_t* TGAImage::GetBuffer()
{
	return data.data();
}

void TGAImage::Clear()
{
	data.clear();
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
	const uint32_t pixelCount = width * height;
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
	out.write(reinterpret_cast< const char*>(&header), sizeof(header));
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
	const uint32_t pixelCount = width * height;
	uint32_t currentPixel = 0;

	while (currentPixel < pixelCount)
	{
		uint32_t chunkStart = currentPixel * bytesPerPixel;
		uint32_t currentByte = currentPixel * bytesPerPixel;
		uint8_t runLength = 1;
		bool raw = true;

		while (currentPixel + runLength < pixelCount && runLength < MAX_CHUNK_LENGTH)
		{
			//Chunk construction
			bool nextPixelIsEqual = true;
			for (int i = 0; nextPixelIsEqual && i < bytesPerPixel; i++)
			{
				nextPixelIsEqual = data[currentByte + i] == data[currentByte + i + bytesPerPixel];
			}
			currentByte += bytesPerPixel;
			if (runLength == 1)
			{
				raw = !nextPixelIsEqual;
			}
			if (raw && nextPixelIsEqual)
			{
				runLength--;
				break;
			}
			if (!raw && !nextPixelIsEqual)
			{
				break;
			}
			runLength++;
		}
		currentPixel += runLength;
		out.put(raw ? runLength - 1 : runLength + (MAX_CHUNK_LENGTH - 1));
		if (!out.good())
		{
			std::cerr << "Fail to unload RLE data \n";
			return false;
		}
		out.write(reinterpret_cast<char*>(data.data() + chunkStart), raw ? runLength * bytesPerPixel : bytesPerPixel);
		if (!out.good())
		{
			std::cerr << "Fail to unload RLE data \n";
			return false;
		}
	}
	return true;
}

TGAColor TGAImage::GetColor(const int x, const int y) const
{
	if (data.empty() || x < 0 || y < 0 || x >= width || y >= height)
	{
		return TGAColor();
	}
	return TGAColor(data.data() + (x + y * width) * bytesPerPixel, bytesPerPixel);
}

bool TGAImage::SetColor(const int x, const int y, const TGAColor color)
{
	if (data.empty() || x < 0 || y < 0 || x >= width || y >= height)
	{
		return false;
	}
	memcpy(data.data() + (x + y * width) * bytesPerPixel, color.Raw, bytesPerPixel);
	return true;
}

bool TGAImage::FlipHorizontally()
{
	if (data.empty())
	{
		return false;
	}

	for (int i = 0; i < width >> 1; i++)
	{
		for (int j = 0; j < height; j++)
		{
			const TGAColor c1 = GetColor(i, j);
			const TGAColor c2 = GetColor(width - 1 - i, j);
			SetColor(i, j, c2);
			SetColor(width - 1 - i, j, c1);
		}
	}
	return true;
}

bool TGAImage::FlipVertically()
{
	if (data.empty())
	{
		return false;
	}

	const uint32_t bytesPerLine = width * bytesPerPixel;
	std::vector<uint8_t> line(bytesPerLine);

	for (int i = 0; i < width >> 1; i++)
	{
		const unsigned long l1 = i * bytesPerLine;
		const unsigned long l2 = (height - 1 - i) * bytesPerLine;
		memmove(line.data(), data.data() + l1, bytesPerLine);
		memmove(data.data() + l1, (data.data() + l2), bytesPerLine);
		memmove(data.data() + l2, line.data(), bytesPerLine);
	}

	return true;
}

bool TGAImage::Scale(int w, int h)
{
	if (w <= 0 || h <= 0 || data.empty())
	{
		return false;
	}

	std::vector<uint8_t> tempData(w * h * bytesPerPixel);
	int newScanline = 0;
	int originalScanline = 0;
	int verticalErrors = 0; //used to decide when to move to the next scanline in the scaled image
	uint32_t newLineBytes = w * bytesPerPixel;
	uint32_t oldLineBytes = width * bytesPerPixel;

	//Nearest-neighbor interpolation: select the nearest pixel from the original image to fill the new pixel grid of scaled image
	for (int i = 0; i < height; i++)
	{
		int HorizontalErrors = width - w;
		int newCurrentX = -bytesPerPixel;
		int originalCurrentX = -bytesPerPixel;

		for (int j = 0; j < width; j++)
		{
			//HORIZONTAL SCALING
			originalCurrentX += bytesPerPixel;
			HorizontalErrors += w; //Accumulate difference between new width and original

			while (HorizontalErrors >= static_cast<int>(width))
			{
				//When horizontal errors accumulate a value equal or greter than the original width,
				//new pixel grid has moved to a new pixel in the scaled image
				HorizontalErrors -= width;
				newCurrentX += bytesPerPixel;
				memcpy(&tempData[newScanline + newCurrentX], &data[originalScanline + originalCurrentX], bytesPerPixel); //So the original pixel is copied into the new image
			}
		}

		//VERTICAL SCALING
		verticalErrors += h; //Accumulate difference between new height and original
		originalScanline += oldLineBytes;
		while (verticalErrors >= static_cast<int>(height)) 
		{
			if (verticalErrors >= static_cast<int>(height) << 1) //scaling has skippen over one or more scanlines
			{
				memcpy(&tempData[newScanline + newCurrentX], &tempData[newScanline], newLineBytes);
			}
			verticalErrors -= height;
			newScanline += newLineBytes;

		}
	}

	data = std::move(tempData);
	width = w;
	height = h;
	return false;
}
