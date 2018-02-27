/* ========================================================================
   $SOURCE FILE
   $File: main.cpp $
   $Program: steganograph $
   $Developer: Jordan Marling $
   $Created On: 2015/09/14 $
   $Functions: 
        void Usage(const char *program)
        int main(int argc, char **argv)
   $
   $Description: This program uses steganography to hide data inside of
                 bitmap images. $
   $Revisions: $
   $NOTES: To convert an image to a 32 bit bitmap, use ImageMagick:
   "convert <source bmp> -type truecolormatte <output bmp>"
   ======================================================================== */

#include <SDL2/SDL.h>

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>

#include "image.h"
#include "image_functions.h"
#include "platform.h"
#include "steganography.h"
#include "timer.h"

/* ========================================================================
   $FUNCTION
   $Name: usage
   $Prototype: void usage(const char *name)
   $Params: 
       name: The name of the program
   $
   $Description: outputs how to use the program. $
   ======================================================================== */
void Usage(const char *program)
{
    printf("%s -i <image> -t -e <filename/text> -d -o <output> -h -r -m\n", program);
    printf("\t-i: The image to encode into.\n");
    printf("\t-t: Encodes/Decodes text. You supply a string into the encode flag.\n");
    printf("\t-e: The encode parameter. This will be a filename or text with the -t flag.\n");
    printf("\t-d: Decodes the image.\n");
    printf("\t-o: The output file to save to. If this flag is not set it will save to stego_image.bmp or the encoded filename\n");
    printf("\t-h: Prints this help message.\n");
    printf("\t-r: Creates a random image to encode a message into\n");
    printf("\t-m: Shows the amount of bytes that can fit in the image.\n");
}

/* ========================================================================
   $FUNCTION
   $Name: main
   $Prototype: int main(int argc, char **argv)
   $Params: 
       Do I really need to say these?
   $
   $Description: The main entrypoint to the program. Gathers all inputs. $
   ======================================================================== */
int main(int argc, char **argv)
{
    Window *window_input = 0;
    Window *window_output = 0;

    Image *image_input = 0;
    Image *image_output = 0;

    // Command line arguments.
    char text_mode = 0;
    char *input_file = 0;
    char *encode = 0;
    char decode = 0;
    char *output = 0;
    char random = 0;

    char *output_buffer;

    struct option long_options[] = {
        { "image", required_argument, 0, 'i' },
        { "encode", required_argument, 0, 'e' },
        { "decode", required_argument, 0, 'd' },
        { "text", no_argument, 0, 't' },
        { "output", required_argument, 0, 'o' },
        { "help", no_argument, 0, 'h' },
        { "random", no_argument, 0, 'r' },
        { "max", required_argument, 0, 'm' },
    };
    
    const char *short_options = "i:e:dto:hrm:";
    int option_index = 0;
    char opt = 0; 
    
    while ((opt = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {
 
        switch (opt)
        {
            case 'h':
            {
                Usage(argv[0]);
                return 0;
            } break;

            case 'i':
            {
                input_file = optarg;
            } break;

            case 't':
            {
                text_mode = 1;
            } break;

            case 'e':
            {
                encode = optarg;
            } break;

            case 'd':
            {
                decode = 1;
            } break;

            case 'o':
            {
                output = optarg;
            } break;

            case 'r':
            {
                random = 1;
            } break;

            case 'm':
            {
                if (optarg)
                {
                    image_input = LoadImage(optarg);
                    if (image_input)
                    {
                        printf("You can fit %d bytes of data in this image.\n", StegoMaxBytes(image_input));
                        return 0;
                    }
                }
            } break;

            default:
                Usage(argv[0]);
                return 1;

        }
    }

    if (!input_file && !random)
    {
        Usage(argv[0]);
        return -1;
    }

    if (!decode && !encode)
    {
        printf("You must have either the encode flag or the decode flag.\n");
        Usage(argv[0]);
        return -1;
    }

    // Load the image
    if (input_file)
    {
        image_input = LoadImage(input_file);
    }
    else if (random)
    {
        image_input = CreateRandomImage(300, 300, 32);
    }

    // Check to see if the image was successfully loaded.
    if (image_input == 0)
    {
        printf("The image %s failed to load.\n", input_file);
        return -1;
    }


    // Check for encoding.
    if (encode)
    {
        if (text_mode)
        {
            image_output = EncodeStegoBuffer(image_input, encode, strlen(encode));
        }
        else
        {
            image_output = EncodeStegoFile(image_input, encode);
        }

        if (image_output == 0)
        {
            return -1;
        }

        if (output)
        {
            SaveBitmap(output, image_output);
        }
        else
        {
            SaveBitmap("stego_output.bmp", image_output);
        }
    }
    else if (decode)
    {
        if (text_mode)
        {
            int size = StegoMaxBytes(image_input);
            int bytes_used;

            output_buffer = (char*)malloc(sizeof(char) * size);
            bytes_used = DecodeStegoBuffer(image_input, output_buffer, size);

            if (output)
            {
                FILE *fp;
                int bytes_written = 0;

                if ((fp = fopen(output, "w")) == 0)
                {
                    printf("Error writing to file: %s\n", output);
                    return -1;
                }

                while (bytes_written < bytes_used)
                {
                    bytes_written += fwrite(output_buffer + bytes_written, 1, bytes_used - bytes_written, fp);
                }
                fclose(fp);
            }
            else
            {
                printf("Decoded Text:\n%.*s\n", bytes_used, output_buffer);
            }
        }
        else
        {
            if (output)
            {
                DecodeStegoFile(image_input, output);
            }
            else
            {
                DecodeStegoFile(image_input, 0);
            }
        }

    }

    
    // Create a window that is the same size as the image.
    if ((window_input = CreateWindow(image_input->Width, image_input->Height, "Input")) == 0)
    {
        printf("Error creating window.\n");
        return -1;
    }

    if (image_output && (window_output = CreateWindow(image_output->Width, image_output->Height, "Output")) == 0)
    {
        printf("Error creating window.\n");
        return -1;
    }

    // Render the images. We need to flip them because of the how the
    // pixel buffers are interpreted.
    if (window_input && image_input)
    {
        FlipVertical(image_input);
        RenderSurface(window_input, image_input);
    }
    if (window_output && image_output)
    {
        FlipVertical(image_output);
        RenderSurface(window_output, image_output);
    }

    // Handle window update loop
    while (UpdateWindow(window_input) == 0 && UpdateWindow(window_output) == 0);
    
    return 0;
}
