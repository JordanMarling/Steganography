/* ========================================================================
   $HEADER FILE
   $File: image.h $
   $Program: $
   $Developer: Jordan Marling $
   $Created On: 2015/09/16 $
   $Description: $
   $Revisions: $
   ======================================================================== */

#if !defined(IMAGE_H)
#define IMAGE_H

#include <math.h>
#include <stdint.h>

#define SQUARE(a) ((a)*(a))
#define SQUAREROOT(a) (sqrtf(a))
#define GETPIXEL(x,y,width) (((y)*(width))+(x))

// The internal Image structure that our program will know how to
// use. When an image is loaded into the program they will all get
// converted to this.
struct Image
{
    // The size of the image
    uint32_t Width;
    uint32_t Height;

    // Pixel data
    uint32_t *Pixels;
    uint32_t PixelCount;

    uint32_t BitsPerPixel;

    // Masking
    uint32_t MaskRed;
    uint32_t MaskGreen;
    uint32_t MaskBlue;
    uint32_t MaskAlpha;

    // How many bits to shift (just storing for convenience)
    uint8_t ShiftRed;
    uint8_t ShiftGreen;
    uint8_t ShiftBlue;
    uint8_t ShiftAlpha;
};

// Returns the pixel from the image.
inline uint32_t GetPixel(Image *image, int x, int y)
{
    return image->Pixels[(y * image->Width) + x];
}

// Sets the pixel in an image
inline void SetPixel(Image *image, int x, int y, uint32_t value)
{
    image->Pixels[(y * image->Width) + x] = value;
}


Image *CreateImage(const int width, const int height, const int bpp);
Image *CreateRandomImage(const int width, const int height, const int bpp);
Image *CopyImage(Image *image);
Image *LoadImage(const char *filename);
void PrintPixel(Image *image, int x, int y);

int SaveBitmap(const char *filename, const Image *image);

#endif
