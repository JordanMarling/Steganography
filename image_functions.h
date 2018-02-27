/* ========================================================================
   $HEADER FILE
   $File: image_functions.h $
   $Program: $
   $Developer: Jordan Marling $
   $Created On: 2015/09/24 $
   $Description: $
   $Revisions: $
   ======================================================================== */

#if !defined(IMAGE_FUNCTIONS_H)
#define IMAGE_FUNCTIONS_H

#define IMAGE_FUNCTIONS_SSE 1

void NegateImage(Image *image);
Image *Scale(Image *image, float percent_width, float percent_height);
void BasicGrayscale(Image *image);
void LuminanceGrayscale(Image *image);

void FlipVertical(Image *image);
void FlipHorizontal(Image *image);

#endif
