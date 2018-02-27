/* ========================================================================
   $SOURCE FILE
   $File: steganography.cpp $
   $Program: steganography $
   $Developer: Jordan Marling $
   $Created On: 2015/09/30 $
   $Functions: 
inline static void SetStegoByte(Image *image, char data, int offset)
inline static void GetStegoByte(Image *image, int offset, char *data)
int StegoMaxBytes(Image *image)
Image *EncodeStegoBuffer(Image *image, const char *buffer, int buffer_length)
Image *EncodeStegoFile(Image *image, const char *filename)
int DecodeStegoBuffer(Image *image, char *buffer, int buffer_len)
int DecodeStegoFile(Image *image, const char *filename)
   $
   $Description: $
   $Revisions: $
   ======================================================================== */

#include "steganography.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image.h"
#include "timer.h"

/* ========================================================================
   $FUNCTION
   $Name: SetStegoByte
   $Prototype: inline static void SetStegoByte(Image *image, char data, int offset)
   $Params: 
       image: The image to set the byte on.
       data: The byte to put into the image.
       offset: The offset to put the data at.
   $
   $Description: This puts a single byte of data into two pixels. $
   ======================================================================== */
inline static void SetStegoByte(Image *image, char data, int offset)
{
    uint8_t red, blue, green, alpha;

    for(int i = 1; i >= 0; i--)
    {

        // Get each of the RGBA values from the pixel
        red = (image->Pixels[offset + i] & image->MaskRed) >> image->ShiftRed;
        green = (image->Pixels[offset + i] & image->MaskGreen) >> image->ShiftGreen;
        blue = (image->Pixels[offset + i] & image->MaskBlue) >> image->ShiftBlue;
        alpha = (image->Pixels[offset + i] & image->MaskAlpha) >> image->ShiftAlpha;

        // Write the data backwards so when we read it, it makes sense.
        alpha &= 0xFE;
        alpha |= data & 0x01;
        data >>= 1;

        blue &= 0xFE;
        blue |= data & 0x01;
        data >>= 1;

        green &= 0xFE;
        green |= data & 0x01;
        data >>= 1;

        red &= 0xFE;
        red |= data & 0x01;
        data >>= 1;

        // Set the pixel data.
        image->Pixels[offset + i] = ((red << image->ShiftRed) |
                                     (green << image->ShiftGreen) |
                                     (blue << image->ShiftBlue) |
                                     (alpha << image->ShiftAlpha));

    }
}

/* ========================================================================
   $FUNCTION
   $Name: GetStegoByte
   $Prototype: inline static void GetStegoByte(Image *image, int offset, char *data)
   $Params: 
       image: The image to set the byte on.
       offset: The offset to get the data at.
       data: The byte of data inside the image
   $
   $Description: This gets a single byte of data from two pixels at the offset. $
   ======================================================================== */
inline static void GetStegoByte(Image *image, int offset, char *data)
{
    uint8_t red, blue, green, alpha;
    *data = 0;

    for(int i = 0; i < 2; i++)
    {

        // Get each of the RGBA values from the pixel
        red = (image->Pixels[offset + i] & image->MaskRed) >> image->ShiftRed;
        green = (image->Pixels[offset + i] & image->MaskGreen) >> image->ShiftGreen;
        blue = (image->Pixels[offset + i] & image->MaskBlue) >> image->ShiftBlue;
        alpha = (image->Pixels[offset + i] & image->MaskAlpha) >> image->ShiftAlpha;

        *data <<= 1;
        *data |= red & 0x1;

        *data <<= 1;
        *data |= green & 0x1;

        *data <<= 1;
        *data |= blue & 0x1;

        *data <<= 1;
        *data |= alpha & 0x1;
    }
}

/* ========================================================================
   $FUNCTION
   $Name: StegoMaxBytes
   $Prototype: int StegoMaxBytes(Image *image)
   $Params: 
       image: The image to calculate how many bytes can fit into.
   $
   $Description: Calculates how many bytes can fit into an image. $
   ======================================================================== */
int StegoMaxBytes(Image *image)
{
    return image->Width * image->Height / 2;
}

/* ========================================================================
   $FUNCTION
   $Name: EncodeStegoBuffer
   $Prototype: Image *EncodeStegoBuffer(Image *image, const char *buffer, int buffer_length)
   $Params: 
       image: The image to encode
       buffer: The buffer of data to put into the image
       buffer_length: The length of the buffer
   $
   $Description: Encodes a buffer of data into an image. $
   ======================================================================== */
Image *EncodeStegoBuffer(Image *image, const char *buffer, int buffer_length)
{
    TIMED_BLOCK();

    Image *encoded_image;
    uint32_t current_pixel;

    // Check to see if we can store the buffer in the image.
    // 4 bytes extra for storing the buffer length.
    if (buffer_length + 4 > StegoMaxBytes(image))
    {
        printf("Error: buffer is too long to store.\n");
        return 0;
    }

    // Create a new image to return.
    encoded_image = CopyImage(image);

    // Write the buffer length
    uint8_t *size = (uint8_t*)&buffer_length;
    SetStegoByte(encoded_image, size[3], 0);
    SetStegoByte(encoded_image, size[2], 2);
    SetStegoByte(encoded_image, size[1], 4);
    SetStegoByte(encoded_image, size[0], 6);

    // Write the data
    current_pixel = 8;
    for(int i = 0; i < buffer_length; i++)
    {
        SetStegoByte(encoded_image, buffer[i], current_pixel);
        current_pixel += 2;
    }

    return encoded_image;
}

/* ========================================================================
   $FUNCTION
   $Name: EncodeStegoFile
   $Prototype: Image *EncodeStegoFile(Image *image, const char *filename)
   $Params: 
       image: The image to encode into
       filename: The filename to put into the image
   $
   $Description: Encodes a filename into an image. $
   ======================================================================== */
Image *EncodeStegoFile(Image *image, const char *filename)
{
    TIMED_BLOCK();

    Image *encoded_image = 0;    
    FILE *fp;
    uint32_t file_length;
    uint32_t bytes_read = 0;
    char *buffer;
    uint32_t overhead_size = 0;

    if ((fp = fopen(filename, "r")) == 0)
    {
        printf("Unable to open file: %s\n", filename);
        return 0;
    }

    overhead_size = strlen(filename) + 1;

    // Get the file size.
    fseek(fp, 0, SEEK_END);
    file_length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = (char*)malloc(file_length + overhead_size);

    while (bytes_read < file_length)
    {
        bytes_read += fread(buffer + bytes_read + overhead_size, 1, file_length - bytes_read, fp);
    }

    // Write the filename
    memcpy(buffer, filename, overhead_size - 1);
    buffer[overhead_size - 1] = 0;

    encoded_image = EncodeStegoBuffer(image, buffer, file_length + overhead_size);

    return encoded_image;
}

/* ========================================================================
   $FUNCTION
   $Name: DecodeStegoBuffer
   $Prototype: int DecodeStegoBuffer(Image *image, char *buffer, int buffer_len)
   $Params: 
       image: The image to decode the buffer from
       buffer: The buffer to write into
       buffer_len: The max size of the buffer.
   $
   $Description: Decodes a buffer of data from an image. $
   ======================================================================== */
int DecodeStegoBuffer(Image *image, char *buffer, int buffer_len)
{
    TIMED_BLOCK();

    uint32_t image_buffer_length = 0;
    int offset = 8;

    // Read the buffer length
    uint8_t *size = (uint8_t*)&image_buffer_length;
    GetStegoByte(image, 0, (char*)(size + 3));
    GetStegoByte(image, 2, (char*)(size + 2));
    GetStegoByte(image, 4, (char*)(size + 1));
    GetStegoByte(image, 6, (char*)(size + 0));

    if (buffer_len < (int)image_buffer_length)
    {
        printf("Cannot decode image. Buffer is too small to write to.\n");
        return -1;
    }

    // Read the data
    for(uint32_t i = 0; i < image_buffer_length; i++)
    {
        GetStegoByte(image, offset, buffer + i);
        offset += 2;
    }

    return image_buffer_length;
}

/* ========================================================================
   $FUNCTION
   $Name: DecodeStegoFile
   $Prototype: int DecodeStegoFile(Image *image, const char *filename)
   $Params: 
       image: The image to decode
       filename: The filename to write to, 0 if the original filename is used.
   $
   $Description: Decodes a file that is stored within an image. $
   ======================================================================== */
int DecodeStegoFile(Image *image, const char *filename)
{
    TIMED_BLOCK();

    FILE *fp = 0;
    const char *file = filename;
    int max_size = StegoMaxBytes(image);
    int actual_size;
    int bytes_written = 0;
    char *buffer;
    char *data;

    buffer = (char*)malloc(max_size);
    actual_size = DecodeStegoBuffer(image, buffer, max_size);

    // If there was no filename specified, use the one in the file.
    if (file == 0)
    {
        file = buffer;
    }

    // Strip the filename off the size and buffer
    actual_size -= strlen(buffer) + 1;
    data = buffer + strlen(buffer) + 1;

    if ((fp = fopen(file, "w")) == 0)
    {
        printf("Error writing file: %s\n", file);
        return -1;
    }

    // Write to the file.
    while (bytes_written < actual_size)
    {
        bytes_written += fwrite(data + bytes_written, 1, actual_size - bytes_written, fp);
    }

    return actual_size;
}
