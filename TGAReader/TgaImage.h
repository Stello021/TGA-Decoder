#pragma once

#include <cstdint>
#include <vector>
#include <fstream>
#include <string>
#include "TgaData.h"

#define MAX_CHUNK_LENGTH 128

class TGAImage
{
protected:
	std::vector<uint8_t> data;
	int32_t width, height, bytesPerPixel;

	/// <summary>
	/// Load image data stored on input file with RLE compression (lossless)
	/// </summary>
	/// <param name="in">
	/// -> Input file
	/// </param>
	/// <returns></returns>
	bool LoadRLEData(std::ifstream& in);

	/// <summary>
	/// Compress the raw image data into RLE format and write it into an output file stream
	/// </summary>
	bool CompressRawData(std::ofstream& out);

public:
	enum Format
	{
		//Numbers associated represent number of channel for each format
		GRAYSCALE = 1, RGB = 3, RGBA = 4
	};

	TGAImage();

	/// <param name="bpp"> Bytes Per Pixel </param>
	TGAImage(int32_t w, int32_t h, int32_t bpp);

	//copy ctor
	TGAImage(const TGAImage& img);

	bool ReadTGAFile(const std::string filename);
	bool WriteTGAFile(const std::string filename, bool rle = true);

	bool FlipHorizontally();
	bool FlipVertically();
	bool Scale(int w, int h);

	TGAColor GetColor(int x, int y);
	bool SetColor(int x, int y, TGAColor color);

	~TGAImage();

	TGAImage& operator=(const TGAImage& img);

	int GetWidth();
	int GetHeigth();
	int GetBytesPerPixel();

	uint8_t* Buffer();
	void Clear();


};