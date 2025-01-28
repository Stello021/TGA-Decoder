# TGA Image Processing Library

Overview
--------

This repository contains a C++ library for reading, writing, and manipulating TGA (Truevision TARGA) image files. The library supports various image formats, including grayscale, RGB, and RGBA, and provides functionalities such as flipping, scaling, and color manipulation.

## Files

* `example.cpp`: A simple example demonstrating how to create a TGA image, set a pixel color, and save the image to a file.

* `tgaData.h`: Contains the data structures for TGA headers and color representation.

* `TgaImage.h`: The header file for the `TGAImage` class, defining the interface for image manipulation.

* `TgaImage.cpp`: The implementation file for the `TGAImage` class, containing the core logic for image processing.

Key Features
------------

* **Image Creation and Manipulation**: Create images with specified dimensions and color depth. Manipulate individual pixels and perform operations like flipping and scaling.

* **File I/O**: Read and write TGA files, with support for both uncompressed and RLE-compressed formats.

* **Color Management**: Represent and manipulate colors using the `TGAColor` struct, which supports grayscale, RGB, and RGBA formats.

Documentation
-------------

For a deeper understanding of the code, especially the more complex parts, please refer to the following sections:

### 1. **TGAHeader Structure (`tgaData.h`)**

* **Description**: Defines the structure of a TGA file header, including image dimensions, color depth, and image type.

* **Key Fields**:
  
  * `ImageWidth`, `ImageHeigth`: Dimensions of the image.
  
  * `PixelDepth`: Number of bits per pixel.
  
  * `ImageDescriptor`: Contains flags for pixel ordering (left-to-right, top-to-bottom).

### 2. **TGAColor Structure (`tgaData.h`)**

* **Description**: Represents a color in the image, supporting different color formats.

* **Key Features**:
  
  * Union for accessing color channels (B, G, R, A) or raw data.
  
  * Constructors for creating colors from individual channel values or raw data.

### 3. **TGAImage Class (`TgaImage.h`, `TgaImage.cpp`)**

* **Description**: The main class for image manipulation, providing methods for reading, writing, and modifying TGA images.

* **Key Methods**:
  
  * `ReadTGAFile`, `WriteTGAFile`: For reading from and writing to TGA files.
  
  * `FlipHorizontally`, `FlipVertically`: For flipping the image along the horizontal or vertical axis.
  
  * `Scale`: For resizing the image using nearest-neighbor interpolation.
  
  * `GetColor`, `SetColor`: For accessing and modifying individual pixels.

### 4. **RLE Compression (`TgaImage.cpp`)**

* **Description**: Implementation of Run-Length Encoding (RLE) for compressing image data.

* **Key Methods**:
  
  * `LoadRLEData`: Reads RLE-compressed data from a file.
  
  * `CompressRawData`: Compresses raw image data into RLE format for writing to a file.

### 5. **Image Scaling (`TgaImage.cpp`)**

* **Description**: Implements nearest-neighbor interpolation for scaling images.

* **Key Logic**:
  
  * Horizontal and vertical scaling are handled separately, with error accumulation to determine when to move to the next pixel or scanline.

## Example

[Go to example file](TGAReader/example.cpp)
