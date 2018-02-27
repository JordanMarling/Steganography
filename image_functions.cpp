/* ========================================================================
   $SOURCE FILE
   $File: image_functions.cpp $
   $Program: $
   $Developer: Jordan Marling $
   $Created On: 2015/09/24 $
   $Functions: $
   $Description: These are image manipulation functions. $
   $Revisions: $
   ======================================================================== */

#include <mmintrin.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#include "timer.h"

// Remove the TIMED_BLOCK macro because we don't want to ouput the times of these functions
#ifdef TIMED_BLOCK
#undef TIMED_BLOCK
#endif
#define TIMED_BLOCK()

#if IMAGE_FUNCTIONS_SSE
/* ========================================================================
   $FUNCTION
   $Name: NegateImage
   $Prototype: void NegateImage(Image *image)
   $Params: 
       image: The image to negate
   $
   $Description: Turns black to white and white to black. This is the SSE version. $
   ======================================================================== */
void NegateImage(Image *image)
{
    TIMED_BLOCK();

    // Set the mask of the RGB values.
    uint32_t high_value = ((0xFF << image->ShiftRed) |
                           (0xFF << image->ShiftGreen) |
                           (0xFF << image->ShiftBlue) |
                           (0x00 << image->ShiftAlpha));

    // Set the mask of the alpha value
    uint32_t alpha_value = (0xFF << image->ShiftAlpha);

    // Set the values of the 128 bit registers.
    __m128i alpha_mask = _mm_set_epi32(alpha_value, alpha_value, alpha_value, alpha_value);
    __m128i high = _mm_set_epi32(high_value, high_value, high_value, high_value);
    __m128i values;
    __m128i output;

    // Loop through each group of 4 pixels.
    for(uint32_t i = 0; i < image->PixelCount; i += 4)
    {

        // Load group of pixels
        values = _mm_load_si128((__m128i*)(image->Pixels + i));

        // Subtract 255 from each RGB value.
        output = _mm_sub_epi8(high, values);

        // Make sure the alpha is still there.
        output = _mm_or_si128(output, alpha_mask);

        // Store the value back in the pixel array.
        _mm_storeu_si128((__m128i*)(image->Pixels + i), output);
    }
}
#else
/* ========================================================================
   $FUNCTION
   $Name: NegateImage
   $Prototype: void NegateImage(Image *image)
   $Params: 
       image: The image to negate
   $
   $Description: Turns black to white and white to black. $
   ======================================================================== */
void NegateImage(Image *image)
{
    TIMED_BLOCK();

    uint8_t red, green, blue, alpha;

    // Loop through each pixel.
    for(uint32_t i = 0; i < image->PixelCount; i++)
    {

        // Get each of the RGBA values from the pixel
        red = (image->Pixels[i] & image->MaskRed) >> image->ShiftRed;
        green = (image->Pixels[i] & image->MaskGreen) >> image->ShiftGreen;
        blue = (image->Pixels[i] & image->MaskBlue) >> image->ShiftBlue;
        alpha = (image->Pixels[i] & image->MaskAlpha) >> image->ShiftAlpha;

        // Set each of the RGB values to be the negative of itself.
        red = 255 - red;
        green = 255 - green;
        blue = 255 - blue;

        // Set the pixel data.
        image->Pixels[i] = ((red << image->ShiftRed) |
                            (green << image->ShiftGreen) |
                            (blue << image->ShiftBlue) |
                            (alpha << image->ShiftAlpha));

    }
}
#endif


/* ========================================================================
   $FUNCTION
   $Name: Scale
   $Prototype: Image *Scale(Image *image, float percent_width, float percent_height)
   $Params: 
       image: The image to scale
       percent_width: How much to scale horizontally
       percent_height: How much to scale vertically
   $
   $Description: Scales an image. $
   ======================================================================== */
Image *Scale(Image *image, float percent_width, float percent_height)
{
    TIMED_BLOCK();

    int new_width = (int)(percent_width * image->Width);
    int new_height = (int)(percent_height * image->Height);

    // Create an empty image that is the size of the scale.
    Image *new_image = CreateImage(new_width, new_height, image->BitsPerPixel);

    // Set the masks to be the same as the previous image.
    new_image->MaskRed = image->MaskRed;
    new_image->MaskGreen = image->MaskGreen;
    new_image->MaskBlue = image->MaskBlue;
    new_image->MaskAlpha = image->MaskAlpha;

    new_image->ShiftRed = image->ShiftRed;
    new_image->ShiftGreen = image->ShiftGreen;
    new_image->ShiftBlue = image->ShiftBlue;
    new_image->ShiftAlpha = image->ShiftAlpha;

    // Set the new image to be transparent.
    memset(new_image->Pixels, 0, new_image->Width * new_image->Height * new_image->BitsPerPixel / 8);

    // Get the ratio of the new image and the old image.
    // +1 at the end deals with rounding issues.
    // This algorithm is a nearest neighbour scaling algorithm
    // It pretty much truncates the ratio to get the pixel to put in the new image.
    int x_rat = (int)((image->Width << 16) / new_image->Width) + 1;
    int y_rat = (int)((image->Height << 16) / new_image->Height) + 1;

    for(uint32_t y = 0; y < new_image->Height; ++y)
    {
        for(uint32_t x = 0; x < new_image->Width; ++x)
        {
            // Translate the new image coordinates to the old image coordinates.
            int x2 = ((x * x_rat) >> 16);
            int y2 = ((y * y_rat) >> 16);

            // Set the pixel data to be the same as the old pixel.
            SetPixel(new_image, x, y, GetPixel(image, x2, y2));
        }
    }

    return new_image;
}

/* ========================================================================
   $FUNCTION
   $Name: BasicGrayscale
   $Prototype: void BasicGrayscale(Image *image)
   $Params: 
       image: The image to grayscale
   $
   $Description: Does a simple grayscale to the image. $
   ======================================================================== */
void BasicGrayscale(Image *image)
{
    TIMED_BLOCK();

    uint8_t red, green, blue, alpha, average;

    // Loop through each pixel.
    for(uint32_t i = 0; i < image->PixelCount; i++)
    {

        // Get each of the RGBA values from the pixel
        red = (image->Pixels[i] & image->MaskRed) >> image->ShiftRed;
        green = (image->Pixels[i] & image->MaskGreen) >> image->ShiftGreen;
        blue = (image->Pixels[i] & image->MaskBlue) >> image->ShiftBlue;
        alpha = (image->Pixels[i] & image->MaskAlpha) >> image->ShiftAlpha;

        // Calculate the average of the RGB values.
        average = (uint8_t)((float)((short)red + green + blue) / 3);

        // Set the average value to each of the RGB values
        red = green = blue = average;

        // Set the pixel data.
        image->Pixels[i] = ((red << image->ShiftRed) |
                            (green << image->ShiftGreen) |
                            (blue << image->ShiftBlue) |
                            (alpha << image->ShiftAlpha));

    }
}

/* ========================================================================
   $FUNCTION
   $Name: LuminanceGrayscale
   $Prototype: void LuminanceGrayscale(Image *image)
   $Params: 
       image: The image to grayscale
   $
   $Description: Does a complex grayscale algorithm on the image. $
   ======================================================================== */
void LuminanceGrayscale(Image *image)
{
    TIMED_BLOCK();

    uint8_t red, green, blue, alpha, average;

    // Loop through each pixel.
    for(uint32_t i = 0; i < image->PixelCount; i++)
    {

        // Get each of the RGBA values from the pixel.
        red = (image->Pixels[i] & image->MaskRed) >> image->ShiftRed;
        green = (image->Pixels[i] & image->MaskGreen) >> image->ShiftGreen;
        blue = (image->Pixels[i] & image->MaskBlue) >> image->ShiftBlue;
        alpha = (image->Pixels[i] & image->MaskAlpha) >> image->ShiftAlpha;

        // Calculate the weighted average of the red/green/blue colours.
        average = (uint8_t)((0.299f * red) + (0.587f * green) + (0.114f * blue));

        // Set the average value to each of the RGB values.
        red = green = blue = average;

        // Set the pixel data.
        image->Pixels[i] = ((red << image->ShiftRed) |
                            (green << image->ShiftGreen) |
                            (blue << image->ShiftBlue) |
                            (alpha << image->ShiftAlpha));

    }
}

/* ========================================================================
   $FUNCTION
   $Name: FlipVertical
   $Prototype: void FlipVertical(Image *image)
   $Params: 
       image: The image to flip
   $
   $Description: Flips an image vertically. $
   ======================================================================== */
void FlipVertical(Image *image)
{
    TIMED_BLOCK();

    uint32_t top, bottom;

    for(uint32_t y = 0; y < image->Height/2; y++)
    {
        for(uint32_t x = 0; x < image->Width; x++)
        {
            
            top = GetPixel(image, x, y);
            bottom = GetPixel(image, x, image->Height - y - 1);

            SetPixel(image, x, y, bottom);
            SetPixel(image, x, image->Height - y - 1, top);

        }
    }
}

// TODO(jordan): Finish this someday...
/* ========================================================================
   $FUNCTION
   $Name: FlipHorizontal
   $Prototype: void FlipHorizontal(Image *image)
   $Params: 
       image: The image to flip.
   $
   $Description: Flips an image horizontally. $
   ======================================================================== */
void FlipHorizontal(Image *image)
{
}
