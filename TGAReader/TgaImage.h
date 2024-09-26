#pragma once

#include "Common.h"
#include <fstream>

class TGAImage
{
protected:
	uint8* data;
	int32 width, height, bytesPerPixel;

	/// <summary>
	/// Load image data stored on input file with RLE compression (lossless)
	/// </summary>
	/// <param name="in">
	/// -> Input file
	/// </param>
	/// <returns></returns>
	bool LoadRLEData(std::ifstream& in);

	/// <summary>
	/// Unload image data stored on input file with RLE compression (lossless)
	/// </summary>
	/// <param name="out">
	/// -> Output file
	/// </param>
	/// <returns></returns>
	bool UnloadRLEData(std::ofstream& out);
};