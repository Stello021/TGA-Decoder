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
    
    ```cpp
    #pragma pack(push,1)
    struct TGAHeader
    {
        int8_t IdLength;
        int8_t ColorMapType;
        int8_t ImageType;
        //Color map specification
        int16_t FirstEntryIndex;
        int16_t ColorMapLength;
        int8_t ColorMapDepth; // Number of bits per color map entry
        //Image specs
        int16_t OriginX;
        int16_t OriginY;
        int16_t ImageWidth;
        int16_t ImageHeigth;
        int8_t PixelDepth; // In bits
        int8_t ImageDescriptor; 
        // IMAGE DESCRIPTOR BITS DATA
        // 5th bit -> pixels ordered from left-to-right (1) or right-to-left (0)
        // 6th bit -> pixels ordered from top-to-bottom (1) or bottom-to-top (0)
    };
    #pragma pack(pop)
    ```
    
    

### 2. **TGAColor Structure (`tgaData.h`)**

* **Description**: Represents a color in the image, supporting different color formats.

* **Key Features**:
  
  * Union for accessing color channels (B, G, R, A) or raw data.
  
  * Constructors for creating colors from individual channel values or raw data.
    
    ```cpp
    struct TGAColor
    {
        union
        {
            struct
            {
                uint8_t B, G, R, A; // Little endian: channels are reversed
            };
            uint8_t Raw[4];
            uint32_t Value;
        };
        int32_t BytesPerPixel;
    
        TGAColor() : Value(0), BytesPerPixel(1) {}
    
        TGAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a), BytesPerPixel(4) {}
    
        TGAColor(int value, int bytesPerPixel) : Value(value), BytesPerPixel(bytesPerPixel) {}
    
        TGAColor(const uint8_t* p, int bytesPerPixel) : Value(0), BytesPerPixel(bytesPerPixel)
        {
            for (int i = 0; i < bytesPerPixel; i++)
            {
                Raw[i] = p[i];
            }
        }
    
        // Copy constructor
        TGAColor(const TGAColor& color) : Value(color.Value), BytesPerPixel(color.BytesPerPixel) {}
    
        // Assignment operator
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
    ```
    
    

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
    
    ```cpp
    bool TGAImage::LoadRLEData(std::ifstream& in)
    {
        const uint32_t pixelCount = width * height;
        uint32_t currentPixel = 0;
        uint32_t currentByte = 0;
        TGAColor colorBuffer;
        uint8_t chunkHeader;    
       //input file stream exctract header previously, now begin from data bytes = 0;
        do
        {
            chunkHeader = in.get(); //get first data byte from input file
            if (!in.good())
            {
                std::cerr << "An error occurred while reading data\n";
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
                        std::cerr << "An error occurred while reading data\n";
                        return false;
                    }
    
                    for (int k = 0; k < bytesPerPixel; k++)
                    {
                        data[currentByte++] = colorBuffer.Raw[k];
                    }
                    currentPixel++;
                    if (currentPixel > pixelCount)
                    {
                        std::cerr << "Pixels read are out of count\n";
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
                    std::cerr << "An error occurred while reading data\n";
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
                        std::cerr << "Pixels read are out of count\n";
                        return false;
                    }
                }
            }
    
        } while (currentPixel < pixelCount);
        return true;
    }
    ```

### 5. **Image Scaling (`TgaImage.cpp`)**

* **Description**: Implements nearest-neighbor interpolation for scaling images.

* **Key Logic**:
  
  * Horizontal and vertical scaling are handled separately, with error accumulation to determine when to move to the next pixel or scanline.
    
    ```cpp
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
                HorizontalErrors += w; //Accumulate difference between new height and original
    
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
                if (verticalErrors >= static_cast<int>(height) << 1) 
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
        return true;
    }
    ```

## Example

[Go to example file](TGAReader/example.cpp)
