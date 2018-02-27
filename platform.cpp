/* ========================================================================
   $SOURCE FILE
   $File: platform.cpp $
   $Program: steganography $
   $Developer: Jordan Marling $
   $Created On: 2015/09/16 $
   $Functions: 
struct Window *CreateWindow(int width, int height, const char *window_title)
int RenderSurface(Window *window, Image *image)
int UpdateWindow(Window *window)
   $
   $Description: This file contains all of the window functions. $
   $Revisions: $
   ======================================================================== */

#include <SDL2/SDL.h>

#include "image.h"

struct Window
{
    SDL_Window *Window;
    SDL_Renderer *Renderer;
};

/* ========================================================================
   $FUNCTION
   $Name: CreateWindow
   $Prototype: struct Window *CreateWindow(int width, int height, const char *window_title)
   $Params: 
       width: The width of the window
       height: The height of the window
       window_title: The name of the window that is displayed.
   $
   $Description: Creates a window and returns a window struct. $
   ======================================================================== */
struct Window *CreateWindow(int width, int height, const char *window_title)
{
    SDL_Window *window = 0;
    SDL_Renderer *renderer = 0;
    Window *out_window = 0;

    // Load the SDL library for video only.
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        printf("Error initializing video: %s\n", SDL_GetError());
        return 0;
    }

    // Create the window with the specified size
    window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED, width, height,
                              SDL_WINDOW_SHOWN);

    if (window == 0)
    {
        return 0;
    }

    // Get the renderer for the window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == 0)
    {
        return 0;
    }

    out_window = (Window*)malloc(sizeof(Window));
    out_window->Window = window;
    out_window->Renderer = renderer;

    return out_window;
}


/* ========================================================================
   $FUNCTION
   $Name: RenderSurface
   $Prototype: int RenderSurface(Window *window, Image *image)
   $Params: 
       window: The window to render to
       image: The image to render on the window.
   $
   $Description: Draws an image in the window $
   ======================================================================== */
int RenderSurface(Window *window, Image *image)
{
    // Create the render surface.
    SDL_Surface *surface = SDL_CreateRGBSurface(0, image->Width, image->Height,
                                                image->BitsPerPixel, image->MaskRed,
                                                image->MaskGreen, image->MaskBlue,
                                                image->MaskAlpha);

    // Set the pixels of the surface to be the pixels of the image.
    surface->pixels = image->Pixels;

    // Create a texture from the surface
    SDL_Texture *tex = SDL_CreateTextureFromSurface(window->Renderer, surface);

    // Put the texture on the renderer
    SDL_RenderCopy(window->Renderer, tex, 0, 0);

    // Tell SDL to render the current frame.
    SDL_RenderPresent(window->Renderer);

    return 0;
}

/* ========================================================================
   $FUNCTION
   $Name: UpdateWindow
   $Prototype: int UpdateWindow(Window *window)
   $Params: 
       window: The window to update.
   $
   $Description: Returns 1 on exit. Otherwise returns 0. It handles window events. $
   ======================================================================== */
int UpdateWindow(Window *window)
{
    SDL_Event event;
    int has_exited = 0;

    // Loop through each of the window events
    // TODO(jordan): Possibly make the window resize and keep the image centered.
    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            // Handle the window events
            case SDL_WINDOWEVENT:
            {
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                {
                    has_exited = 1;
                }
            } break;

            // If the close button was pressed
            case SDL_QUIT:
            {
                has_exited = 1;
            } break;

            // If a key on the keyboard is pressed down
            case SDL_KEYDOWN:
            {
                // If the key is escape
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    has_exited = 1;
                }
            } break;

        }
    }

    return has_exited;
}
