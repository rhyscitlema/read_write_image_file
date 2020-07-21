/*
    rwif_libpng.c

    Link: http://www.libpng.org/pub/png/libpng-1.2.5-manual.html
    Accessed on: 22 November 2014
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "rwif.h"

#ifdef NO_PNG
int read_image_file_png(const char* filename, ImageData *imagedata) { return 0; }
int write_image_file_png(const char* filename, const ImageData *imagedata) { return 0; }
#else

#include <png.h>

#define ABORT(string, argument, rt) \
{   sprintf (rwif_errormessage, string, argument); \
    fclose(fp); \
    return rt; \
}



/* return -2 if file cannot be opened
 * return -1 if file data is corrupted
 * return  0 if file format is not valid
 * return  1 if file successfully loaded
 */
int read_image_file_png (const char* filename, ImageData *imagedata)
{
    int i, width, height, BPP;
    unsigned char* pixelArray = NULL;
    png_bytep* row_pointers;
    png_byte color_type;
    png_infop info_ptr;
    png_structp png_ptr;
    png_byte header[8];
    FILE *fp;


    /* open file for reading-bytes */
    fp = fopen (filename, "rb");

    if(!fp) ABORT("Read Error: file '%s' could not be opened for reading.", filename, -2)

    /* Test if file type is png */
    /* 8 bytes is the maximum size that can be checked */
    if(fread(header, 1, 8, fp) != 8
    || png_sig_cmp(header, 0, 8))
        ABORT("Read Error: file '%s' is not recognized as a PNG file.", filename, 0)


    /* initialise - prepare for parsing */
    png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_ptr)
        ABORT("Read Error: file '%s' is corrupted. png_create_read_struct failed.", filename, -1)

    info_ptr = png_create_info_struct (png_ptr);
    if(!info_ptr)
        ABORT("Read Error: file '%s' is corrupted. png_create_info_struct failed.", filename, -1)

    if(setjmp(png_jmpbuf(png_ptr)))
        ABORT("Read Error: file '%s' is corrupted. setjmp(png_jmpbuf(png_ptr)) failed.", filename, -1)

    png_init_io (png_ptr, fp);
    png_set_sig_bytes (png_ptr, 8);
    png_read_info (png_ptr, info_ptr);


    /* ensure 3-byte colour depth */
    color_type = png_get_color_type (png_ptr, info_ptr);
    if(color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
          png_set_gray_to_rgb (png_ptr);

    /* ensure Alpha channel present */
    png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

    /*png_get_bit_depth(png_ptr, info_ptr) = 8... why so? */
    height = png_get_image_height(png_ptr, info_ptr);
    width  = png_get_image_width (png_ptr, info_ptr);
    BPP    = 4;

    /* ensure BGRA and not ARGB */
    png_set_bgr(png_ptr);

    /* make libpng to handle expansion of the interlaced image */
    png_set_interlace_handling (png_ptr);

    /* reflect requested transformations */
    png_read_update_info (png_ptr, info_ptr);


    /* prepare to read file */
    if(setjmp(png_jmpbuf(png_ptr)))
        ABORT("Read Error: file '%s' is corrupted. setjmp(png_jmpbuf(png_ptr)) failed.", filename, -1)

    /* allocate memory needed to hold the image */
    pixelArray = (unsigned char*) malloc (height*width*BPP);

    row_pointers = (png_bytep*) malloc (height * sizeof(*row_pointers));

    for(i = 0; i < height; i++)
        row_pointers[i] = (png_bytep) (pixelArray + i*width*BPP);

    /* finally... read image file... */
    png_read_image(png_ptr, row_pointers);

    /* free resources */
    free(row_pointers);
    fclose(fp);


    /* success */
    imagedata->pixelArray = pixelArray;
    imagedata->height = height;
    imagedata->width = width;
    imagedata->bpp = BPP*8;
    return 1;
}



/* return success 1 or failure 0 */
int write_image_file_png (const char* filename, const ImageData *imagedata)
{
    int i, width, height, bpp;
    FILE *fp;
    png_bytep* row_pointers;
    png_byte color_type;
    png_byte bit_depth;
    png_infop info_ptr;
    png_structp png_ptr;

    height = imagedata->height;
    width = imagedata->width;
    bpp = imagedata->bpp;
    if(bpp!=24 && bpp!=32)
    { sprintf(rwif_errormessage, "Write Error on '%s': bits-per-pixel must be 24 or 32 not %d.", filename, bpp); return 0; }

    /* open file for writing-bytes */
    fp = fopen(filename, "wb");

    if(!fp) ABORT("Write Error: file '%s' could not be opened for writing.", filename, 0)


    /* initialise - prepare for parsing */
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if(!png_ptr)
        ABORT("Write Error on '%s': png_create_write_struct failed.", filename, 0)

    info_ptr = png_create_info_struct (png_ptr);
    if(!info_ptr)
        ABORT("Write Error on '%s': png_create_info_struct failed.", filename, 0)

    if(setjmp(png_jmpbuf(png_ptr)))
        ABORT("Write Error on '%s': setjmp(png_jmpbuf(png_ptr)) failed.", filename, 0)

    png_init_io (png_ptr, fp);


    /* write png header structure */
    bit_depth = 8;
    if(setjmp(png_jmpbuf(png_ptr)))
        ABORT("Write Error on '%s': error during header writing.", filename, 0)

    if(bpp==3*8)
         color_type = PNG_COLOR_TYPE_RGB;
    else color_type = PNG_COLOR_TYPE_RGBA;

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 bit_depth, color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    /* ensure BGRA and not ARGB */
    png_set_bgr(png_ptr);

    png_write_info(png_ptr, info_ptr);


    /* prepare to write bytes to file */
    if(setjmp(png_jmpbuf(png_ptr)))
        ABORT("Write Error on '%s': error during bytes writing.", filename, 0)

    /* allocate memory needed to point to the image */
    row_pointers = (png_bytep*) malloc (height * sizeof(*row_pointers));

    for(i = 0; i < height; i++)
        row_pointers[i] = (png_bytep) (imagedata->pixelArray + i*width*(bpp/8));

    /* finally... write image file... */
    png_write_image (png_ptr, row_pointers);


    /* end write - finalise */
    if(setjmp(png_jmpbuf(png_ptr))) {
        free(row_pointers); ABORT("Write Error on '%s': error during end of writing.", filename, 0); }

    png_write_end (png_ptr, NULL);


    /* free resources */
    free(row_pointers);
    fclose(fp);

    /* return success */
    return 1;
}

#endif
