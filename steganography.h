/* ========================================================================
   $HEADER FILE
   $File: steganography.h $
   $Program: $
   $Developer: Jordan Marling $
   $Created On: 2015/09/30 $
   $Description: $
   $Revisions: $
   ======================================================================== */

#if !defined(STEGANOGRAPHY_H)
#define STEGANOGRAPHY_H

#include "image.h"

int StegoMaxBytes(Image *image);

Image *EncodeStegoBuffer(Image *image, const char *buffer, int buffer_length);
// Image *EncodeStegoBufferEnc(Image *image, const char *buffer, int buffer_length, AESType aes, const char *password);

Image *EncodeStegoFile(Image *image, const char *filename);
// Image *EncodeStegoFileEnc(Image *image, const char *filename, const char *password);

int DecodeStegoBuffer(Image *image, char *buffer, int buffer_len);
// int DecodeStegoBufferEnc(Image *image, char *buffer, int buffer_len, AESType aes, const char *password);

int DecodeStegoFile(Image *image, const char *filename);
// int DecodeStegoFileEnc(Image *image, const char *filename, const char *password);

#endif
