#include "tgaimage.h"

const TGAColor red = TGAColor(255, 0, 0, 255);

int main(int argc, char** argv) {
	TGAImage image(100, 100, TGAImage::RGB);
	image.SetColor(52, 41, red);
	image.FlipVertically(); // i want to have the origin at the left bottom corner of the image
	image.WriteTGAFile("example.tga");
	return 0;
}