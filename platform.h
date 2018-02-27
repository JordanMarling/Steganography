/* ========================================================================
   $HEADER FILE
   $File: platform.h $
   $Program: $
   $Developer: Jordan Marling $
   $Created On: 2015/09/16 $
   $Description: $
   $Revisions: $
   ======================================================================== */

#if !defined(PLATFORM_H)
#define PLATFORM_H

#include "image.h"

struct Window;

Window *CreateWindow(int width, int height, const char *window_title);
int UpdateWindow(Window *window);
int RenderSurface(Window *window, Image *image);

#endif
