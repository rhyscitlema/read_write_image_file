#ifndef _RWIF_H
#define _RWIF_H
/*
    rwif.h

    Use at your own risk!
*/

typedef struct _ImageData
{
    unsigned char* pixelArray;
    unsigned int height;
    unsigned int width;
    unsigned int bpp; /* Bits Per Pixel */
    /* Colour format for bpp=3*8 is {Blue, Green, Red} */
    /* Colour format for bpp=4*8 is {Blue, Green, Red, Alpha} */
} ImageData;


/* The string that contains the latest
 * error message of a failed operation.
 */
extern char rwif_errormessage[];


/* read_image_file(..)
 *
 * input: filename of image
 * input: pointer to valid ImageData
 * (if first time then clear struct to all 0)
 *
 * output: modified ImageData
 * return success 1 or failure 0
 */
extern int read_image_file (const char* filename, ImageData *imagedata);

extern void clear_image_data (ImageData* imagedata);


/* write_image_file(..)
 *
 * input: filename of image
 * input: pointer to const ImageData
 * return success 1 or failure 0
 *
 * The image file type is specified by
 * the extension part of the file name.
 */
extern int write_image_file (const char* filename, const ImageData *imagedata);


/* return -2 if file cannot be opened
 * return -1 if file data is corrupted
 * return  0 if file format is not valid
 * return  1 if file successfully loaded
 */
extern int read_image_file_bmp (const char* filename, ImageData *imagedata);
extern int read_image_file_png (const char* filename, ImageData *imagedata);
extern int read_image_file_jpg (const char* filename, ImageData *imagedata);


/* return success 1 or failure 0.
 * 'quality' must be in range [1, 100], typical value is 96.
 */
extern int write_image_file_bmp (const char* filename, const ImageData *imagedata);
extern int write_image_file_png (const char* filename, const ImageData *imagedata);
extern int write_image_file_jpg (const char* filename, const ImageData *imagedata, int quality);


#endif
