# Steganography

This application is written in C code for a Linux operating system. It uses BMP images to hide text data
or files.

The way that the data is stored in the file, is that for each RGBA value in each pixel, the last bit is used
to store the information. This allows for one byte to be stored for every two pixels.


It uses SDL2 to render the image.


## Program Flags
./steganography -i <image> -t -e <filename/text> -d -o <output> -h -r -m

	-i: The image to encode into.
	
	-t: Encodes/Decodes text. You supply a string into the encode flag.
	
	-e: The encode parameter. This will be a filename or text with the -t flag.
	
	-d: Decodes the image.
	
	-o: The output file to save to. If this flag is not set it will save to stego_image.bmp or the encoded filename
	
	-h: Prints this help message.
	
	-r: Creates a random image to encode a message into
	
	-m: Shows the amount of bytes that can fit in the image.



## Examples
Encoding Text:

./steganography -i black.bmp -t -e "This is a test" -o output.bmp


Decoding Text:

./steganography -i output.bmp -t -d


Encoding a File:

./steganography -i black.bmp -e input -o output.bmp


Decoding a File:

./steganography -i output.bmp -d

