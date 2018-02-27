/* ========================================================================
   $SOURCE FILE
   $File: image.cpp $
   $Program: steganography $
   $Developer: Jordan Marling $
   $Created On: 2015/09/16 $
   $Functions: 
int SaveBitmap(const char *filename, const Image *image)
static Image *LoadBitmap(const char *filename)
static int FindLeastSignificantBit(uint32_t num)
Image *CopyImage(Image *image)
Image *CreateRandomImage(const int width, const int height, const int bpp)
Image *CreateImage(const int width, const int height, const int bpp)
Image *LoadImage(const char *filename)
   $
   $Description: This file handles everything to do with loading/saving the images. $
   $Revisions: $
   ======================================================================== */


#include "image.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "image_functions.h"

// The header of the bitmap should not have any padding in it.
#pragma pack(push, 1)
struct BitmapHeader
{
    uint16_t FileType;
    uint32_t FileSize;
    uint16_t Reserved1;
    uint16_t Reserved2;
    uint32_t BitmapOffset;
    uint32_t Size;
    int32_t Width;
    int32_t Height;
    uint16_t Planes;
    uint16_t BitsPerPixel;
    uint32_t Compression;
    uint32_t SizeOfBitmap;
    int32_t HorizontalResolution;
    int32_t VerticalResolution;
    uint32_t ColoursUsed;
    uint32_t ColoursImportant;

    uint32_t RedMask;
    uint32_t GreenMask;
    uint32_t BlueMask;
};
#pragma pack(pop)


static int FindLeastSignificantBit(uint32_t num);
static Image *LoadBitmap(const char *filename);


/* ========================================================================
   $FUNCTION
   $Name: LoadImage
   $Prototype: Image *LoadImage(const char *filename)
   $Params: 
       filename: The file to load
   $
   $Description: This function detects which filetype the filename is 
   and loads it properly. $
   ======================================================================== */
Image *LoadImage(const char *filename)
{
    Image *image = 0;

    // Check to see if the bitmap was loaded successfully, if so return it.
    if ((image = LoadBitmap(filename)) != 0)
    {
        return image;
    }

    return image;
}

/* ========================================================================
   $FUNCTION
   $Name: CreateImage
   $Prototype: Image *CreateImage(const int width, const int height, const int bpp)
   $Params: 
       width: The width of the image
       height: The height of the image
       bpp: How many bits are per pixel.
   $
   $Description: This function creates an empty image of the specified params. $
   ======================================================================== */
Image *CreateImage(const int width, const int height, const int bpp)
{
    Image *image = (Image*)malloc(sizeof(Image));
    
    image->Width = width;
    image->Height = height;
    image->PixelCount = width * height;
    image->BitsPerPixel = bpp;

    image->Pixels = (uint32_t*)malloc(bpp / 8 * width * height);

    return image; 
}

/* ========================================================================
   $FUNCTION
   $Name: CreateRandomImage
   $Prototype: Image *CreateRandomImage(const int width, const int height, const int bpp)
   $Params: 
       width: The width of the image
       height: The height of the image
       bpp: How many bits are per pixel.
   $
   $Description: Creates an image with random data with the specified params. $
   ======================================================================== */
Image *CreateRandomImage(const int width, const int height, const int bpp)
{
    Image *image = CreateImage(width, height, bpp);
    FILE *fp;
    int bytes_read = 0;
    int bytes_to_read = image->PixelCount * (bpp / 8);

    if ((fp = fopen("/dev/urandom", "r")) == 0)
    {
        free(image);
        return 0;
    }

    while (bytes_read < bytes_to_read)
    {
        bytes_read += fread(image->Pixels + bytes_read, 1, bytes_to_read - bytes_read, fp);
    }

    return image;
}

/* ========================================================================
   $FUNCTION
   $Name: CopyImage
   $Prototype: Image *CopyImage(Image *image)
   $Params: 
       image: The image to copy
   $
   $Description: Returns an exact copy of the image. $
   ======================================================================== */
Image *CopyImage(Image *image)
{
    Image *new_image = CreateImage(image->Width, image->Height, image->BitsPerPixel);
    uint32_t *pixels = new_image->Pixels;

    // Copy the meta data
    memcpy(new_image, image, sizeof(Image));

    // Copy the pixel data
    new_image->Pixels = pixels;
    memcpy(new_image->Pixels, image->Pixels, image->PixelCount * image->BitsPerPixel / 8);

    return new_image;
}

/*void PrintPixel(Image *image, int x, int y)
{
    uint8_t red, green, blue, alpha;
    uint32_t pixel = GetPixel(image, x, y);

    red = (pixel & image->MaskRed) >> image->ShiftRed;
    green = (pixel & image->MaskGreen) >> image->ShiftGreen;
    blue = (pixel & image->MaskBlue) >> image->ShiftBlue;
    alpha = (pixel & image->MaskAlpha) >> image->ShiftAlpha;

    printf("{%d,%d,%d,%d}\n", red, blue, green, alpha);

    }*/

/* ========================================================================
   $FUNCTION
   $Name: FindLeastSignificantBit
   $Prototype: static int FindLeastSignificantBit(uint32_t num)
   $Params: 
       num: The number to find the LSB.
   $
   $Description: Finds the index of the LSB. $
   ======================================================================== */
static int FindLeastSignificantBit(uint32_t num)
{
    uint32_t result = -1;

    // Loop through each bit to see if it is a 1
    for(uint32_t i = 0; i < 32; i++)
    {
        if (num & (1 << i))
        {
            result = i;
            break;
        }
    }

    return result;
}

/* ========================================================================
   $FUNCTION
   $Name: LoadBitmap
   $Prototype: static Image *LoadBitmap(const char *filename)
   $Params: 
       filename: The file to load
   $
   $Description: Loads a file into a bitmap type. $
   ======================================================================== */
static Image *LoadBitmap(const char *filename)
{

    char *buffer;
    int fp;
    Image *bitmap = 0;
    BitmapHeader header;

    // Attempt to open the bitmap file in read only mode.
    if ((fp = open(filename, O_RDONLY)) < 0)
    {
        printf("Error opening file.\n");
        return 0;
    }

    // Attempt to read the header.
    if (read(fp, (char*)&header, sizeof(BitmapHeader)) != sizeof(BitmapHeader))
    {
        printf("Error reading bitmap file header.\n");
        return 0;
    }
    
    // Check the header magic number to make sure its a bitmap.
    if (header.FileType != 0x4d42)
    {
        printf("Error loading bitmap magic number.\n");
        return 0;
    }


    // Set the image pixel data location.
    bitmap = (Image*)malloc(sizeof(Image));
    bitmap->Pixels = (uint32_t*)malloc(sizeof(uint32_t) * header.Width * header.Height);
    bitmap->PixelCount = header.Width * header.Height;
    bitmap->Width = header.Width;
    bitmap->Height = header.Height;

    // I don't support flipped images right now.
    if (header.Height < 0)
    {
        printf("This bitmap is upside down. Time to implement it.\n");
        return 0;
    }

    // I don't support bitmaps that don't have 32 bit pixels.
    if (header.BitsPerPixel != 32)
    {
        printf("Cannot open a bitmap without 32 bits per pixel.\n");
        return 0;
    }

    // The compression type needs to be Bit Field.
    if (header.Compression != 3)
    {
        printf("Cannot open a bitmap without a Bit Field compression.\n");
        return 0;
    }

    // Check the header size for different information.
    if (header.Size == 12)
    {
        printf("Need to implement this bitmap header size...\n");
        return 0;
    }
    else
    {
        // Seek to the start of the data array
        lseek(fp, header.BitmapOffset, SEEK_SET);
    }

    // Allocate size for the bitmap.
    buffer = (char*)malloc(header.BitsPerPixel * header.Width * header.Height);

    int bytes_read = 0;
    int n = 1;
    int bytes_left = (header.BitsPerPixel / 8) * header.Width * header.Height;

    // Read the whole file into the buffer.
    while(n > 0)
    {
        n = read(fp, buffer + bytes_read, bytes_left);
        bytes_read += n;
    }

    // printf("bytes read: %d\n", bytes_read);
    // printf("pixel size: %d\n", bitmap->PixelCount * 4);

    // Find the pixel masks from the header.
    uint32_t mask_red = header.RedMask;
    uint32_t mask_green = header.GreenMask;
    uint32_t mask_blue = header.BlueMask;
    uint32_t mask_alpha = ~(mask_red | mask_green | mask_blue);

    // Get the amount of bits to shift each mask. This will be used
    // when loading bitmaps that have different bits per pixel, such
    // as 8, 16, and 24.
    uint32_t shift_red = FindLeastSignificantBit(mask_red);
    uint32_t shift_green = FindLeastSignificantBit(mask_green);
    uint32_t shift_blue = FindLeastSignificantBit(mask_blue);
    uint32_t shift_alpha = FindLeastSignificantBit(mask_alpha);

    // float max = 255.0f;
    // float invmax = 1.0f / max;

    // Calculate the pitch or how many bytes per horizontal line.
    // uint32_t pitch = header.Width * (header.BitsPerPixel / 8);

    // Loop through each pixel and put it into our image.
    uint32_t *source = (uint32_t*)buffer;
    uint32_t *dest = bitmap->Pixels;
    for(int y = 0; y < header.Height; y++)
    {

        // The image is upside down to SDL so we need to start from the bottom and go to the top.
        // uint32_t *dest = (uint32_t*)((uint8_t*)bitmap->Pixels + (pitch * y));

        for(int x = 0; x < header.Width; x++)
        {

            uint32_t C = *source++;

            // Get the RGBA values.
            float red = (C & mask_red) >> shift_red;
            float green = (C & mask_green) >> shift_green;
            float blue = (C & mask_blue) >> shift_blue;
            float alpha = (C & mask_alpha) >> shift_alpha;

/*
// We can use the following code to convert from non-32 bit bitmaps
// to 32 bit.
// Convert to proper ARGB values
red = SQUARE(invmax * red);
green = SQUARE(invmax * green);
blue = SQUARE(invmax * blue);
alpha = invmax * alpha;

red *= alpha;
green *= alpha;
blue *= alpha;

red = max * SQUAREROOT(red);
green = max * SQUAREROOT(green);
blue = max * SQUAREROOT(blue);
alpha = max * alpha;
*/

            // Set the pixel in the correct buffer with the RGBA values rounded up.
            *dest++ = (((uint32_t)(alpha + 0.5f) << 24) |
                       ((uint32_t)(red   + 0.5f) << 16) |
                       ((uint32_t)(green + 0.5f) << 8) |
                       ((uint32_t)(blue  + 0.5f) << 0));

        }

    }

    // Fill out the rest of the data in the image struct.
    bitmap->BitsPerPixel = 32;

    bitmap->MaskRed = mask_red;
    bitmap->MaskGreen = mask_green;
    bitmap->MaskBlue = mask_blue;
    bitmap->MaskAlpha = mask_alpha;

    bitmap->ShiftRed = shift_red;
    bitmap->ShiftGreen = shift_green;
    bitmap->ShiftBlue = shift_blue;
    bitmap->ShiftAlpha = shift_alpha;

    // FlipVertical(bitmap);

    return bitmap;
}

/* ========================================================================
   $FUNCTION
   $Name: SaveBitmap
   $Prototype: int SaveBitmap(const char *filename, const Image *image)
   $Params: 
       filename: The filename to save to
       image: The image to save
   $
   $Description: Saves the image as a bitmap. $
   ======================================================================== */
int SaveBitmap(const char *filename, const Image *image)
{
    
    BitmapHeader header;
    int fp;
    char *buffer;
    int bytes_written;
    int buffer_len;
    
    // Fill out the bitmap header
    header.FileType = 0x4d42; // The bitmap magic number
    header.FileSize = 0;
    header.Reserved1 = 0;
    header.Reserved2 = 0;
    header.BitmapOffset = sizeof(BitmapHeader);
    header.Size = 40;
    header.Width = image->Width;
    header.Height = image->Height;
    header.Planes = 1;
    header.BitsPerPixel = image->BitsPerPixel;
    header.Compression = 3; // Compression is Bit Field
    header.SizeOfBitmap = image->PixelCount * image->BitsPerPixel / 8;
    header.HorizontalResolution = 0;
    header.VerticalResolution = 0;
    header.ColoursUsed = 0;
    header.ColoursImportant = 0;

    header.RedMask = image->MaskRed;
    header.GreenMask = image->MaskGreen;
    header.BlueMask = image->MaskBlue;
    
    if ((fp = open(filename, O_WRONLY | O_CREAT, 0666)) < 0)
    {
        printf("Error creating save file.\n");
        return 1;
    }

    // Create the buffer to store the data.
    buffer_len = sizeof(BitmapHeader) + (image->PixelCount * image->BitsPerPixel / 8);
    buffer = (char*)malloc(buffer_len);

    // Put the data in the buffer.
    memcpy(buffer, &header, sizeof(BitmapHeader));
    memcpy(buffer + sizeof(BitmapHeader), image->Pixels, image->PixelCount * image->BitsPerPixel / 8);
    
    // Write the buffer to the file.
    bytes_written = 0;
    while (bytes_written < buffer_len)
    {
        bytes_written += write(fp, buffer + bytes_written, buffer_len - bytes_written);
    }
    
    return 0;
}
