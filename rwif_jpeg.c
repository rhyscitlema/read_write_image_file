/*
    rwif_libjpeg.c

    Link: https://github.com/LuaDist/libjpeg/blob/master/example.c
    Accessed on: 22 November 2014
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "rwif.h"

#include <setjmp.h>
#include <jpeglib.h>



struct my_error_mgr {
    struct jpeg_error_mgr pub;  /* "public" fields */
    jmp_buf setjmp_buffer;      /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}



/* return -2 if file cannot be opened
 * return -1 if file data is corrupted
 * return  0 if file format is not valid
 * return  1 if file successfully loaded
 */
int read_image_file_jpg (const char* filename, ImageData *imagedata)
{
    int i, width, height, BPP;
    unsigned char *row_data, *pixelArray = NULL;

    /* This struct contains the JPEG decompression parameters and pointers to
    * working space (which is allocated as needed by the JPEG library).
    */
    struct jpeg_decompress_struct cinfo;

    /* We use our private extension JPEG error handler.
    * Note that this struct must live as long as the main JPEG parameter
    * struct, to avoid dangling-pointer problems.
    */
    struct my_error_mgr jerr;

    /* open file for reading-bytes */
    FILE *fp = fopen (filename, "rb");
    if (!fp)
    {
        sprintf (rwif_errormessage, "Read Error: file '%s' could not be opened for reading.", filename);
        return -2;
    }

    /* Step 1: allocate and initialize JPEG decompression object */
    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
        * We need to clean up the JPEG object, close the input file, and return.
        */
        jpeg_destroy_decompress(&cinfo);
        sprintf(rwif_errormessage, "Read Error: file '%s' seems to be corrupted.", filename);
        fclose(fp);
        free(pixelArray);
        return 0;
    }

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&cinfo, fp);

    /* Step 3: read file parameters with jpeg_read_header() */
    (void) jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
    * (a) suspension is not possible with the stdio data source, and
    * (b) we passed TRUE to reject a tables-only JPEG file as an error.
    * See libjpeg.txt for more info.
    */

    /* Step 4: set parameters for decompression */
    /* In this example, we don't need to change any of the defaults set by
    * jpeg_read_header(), so we do nothing here.
    */

    /* Step 5: Start decompressor */
    (void) jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

    /* number of bytes-per-pixels should be 3 */
    if(cinfo.output_components==3)
    {
        /* We may need to do some setup of our own at this point before reading
        * the data. After jpeg_start_decompress() we have the correct scaled
        * output image dimensions available, as well as the output colormap
        * if we asked for color quantization.
        * In this example, we need to make an output work buffer of the right size.
        */

        /* JSAMPLEs per row in output buffer */
        height = cinfo.output_height;
        width = cinfo.output_width;
        BPP = cinfo.output_components;

        pixelArray  = (unsigned char*) malloc (height * width * BPP);
        row_data    = (unsigned char*) malloc (width * BPP);

        imagedata->pixelArray = pixelArray;
        imagedata->height = height;
        imagedata->width = width;
        imagedata->bpp = BPP*8;

        /* Step 6: while (scan lines remain to be read) */
        /* jpeg_read_scanlines(...); */
        /* Here we use the library's state variable cinfo.output_scanline as the
        * loop counter, so that we don't have to keep track ourselves.
        */
        while (cinfo.output_scanline < cinfo.output_height)
        {
            /* jpeg_read_scanlines expects an array of pointers to scanlines.
            * Here the array is only one element long, but you could ask for
            * more than one scanline at a time if that's more convenient.
            */
            jpeg_read_scanlines (&cinfo, &row_data, 1);
            for(i=0; i<width; i++)
            {
                /* from (Red, Green, Blue) to (Blue, Green, Red) */
                pixelArray[i*3+0] = row_data[i*3+2];
                pixelArray[i*3+1] = row_data[i*3+1];
                pixelArray[i*3+2] = row_data[i*3+0];
            }
            pixelArray += width*3;
         }
        free(row_data);
    }
    else
    {
        sprintf (rwif_errormessage, "Read Error: file '%s' is expected to have bytes-per-pixel = 3 not %d.", filename, cinfo.output_components);
        width = -1;
    }

    /* Step 7: Finish decompression */
    (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

    /* Step 8: Release JPEG decompression object */
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    /* At this point you may want to check to see whether any corrupt-data
    * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
    */
    if(jerr.pub.num_warnings)
    {
        sprintf (rwif_errormessage, "Read Error on '%s': Loading unsuccessful.", filename);
        width = -1;
    }

    /* After finish_decompress, we can close the input file.
    * Here we postpone it until after no more JPEG errors are possible,
    * so as to simplify the setjmp error logic above. (Actually, I don't
    * think that jpeg_destroy can do an error exit, but why assume anything...)
    */
    fclose(fp);

    /* if a failure */
    if(width < 0)
    {   free (pixelArray);
        return -1;
    }

    /* return success */
    return 1;
}



/* return success 1 or failure 0.
 * 'quality' must be in range [1, 100], typical value is 96.
 */
int write_image_file_jpg (const char* filename, const ImageData *imagedata, int quality)
{
    int i, width, height, bpp, BPP;
    FILE *outfile;
    unsigned char *row_data, *pixelArray;

    /* This struct contains the JPEG compression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     * It is possible to have several such structures, representing multiple
     * compression/decompression processes, in existence at once.  We refer
     * to any one struct (and its associated working data) as a "JPEG object".
     */
    struct jpeg_compress_struct cinfo;
    /* This struct represents a JPEG error handler.  It is declared separately
     * because applications often want to supply a specialized error handler
     * (see the second half of this file for an example).  But here we just
     * take the easy way out and use the standard error handler, which will
     * print a message on stderr and call exit() if compression fails.
     * Note that this struct must live as long as the main JPEG parameter
     * struct, to avoid dangling-pointer problems.
     */
    struct jpeg_error_mgr jerr;

    height = imagedata->height;
    width = imagedata->width;
    bpp = imagedata->bpp;
    BPP = bpp/8;
    if(bpp!=24 && bpp!=32)
    { sprintf(rwif_errormessage, "Write Error on '%s': bits-per-pixel must be 24 or 32 not %d.", filename, bpp); return 0; }


    /* Step 1: allocate and initialize JPEG compression object */

    /* We have to set up the error handler first, in case the initialization
     * step fails.  (Unlikely, but it could happen if you are out of memory.)
     * This routine fills in the contents of struct jerr, and returns jerr's
     * address which we place into the link field in cinfo.
     */
    cinfo.err = jpeg_std_error(&jerr);
    /* Now we can initialize the JPEG compression object. */
    jpeg_create_compress(&cinfo);

    /* Step 2: specify data destination (eg, a file) */
    /* Note: steps 2 and 3 can be done in either order. */

    /* Here we use the library-supplied code to send compressed data to a
     * stdio stream.  You can also write your own code to do something else.
     * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
     * requires it in order to write binary files.
     */
    outfile = fopen(filename, "wb");

    if(!outfile) { sprintf(rwif_errormessage, "Write Error: file '%s' could not be opened for writing.", filename); return 0; }

    jpeg_stdio_dest(&cinfo, outfile);

    /* Step 3: set parameters for compression */

    /* First we supply a description of the input image.
     * Four fields of the cinfo struct must be filled in:
     */
    cinfo.image_width = width; 	/* image width and height, in pixels */
    cinfo.image_height = height;
    cinfo.input_components = 3;		/* # of color components per pixel */
    cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */

    /* Now use the library's routine to set default compression parameters.
     * (You must set at least cinfo.in_color_space before calling this,
     * since the defaults depend on the source color space.)
     */
    jpeg_set_defaults(&cinfo);

    /* Now you can set any non-default parameters you wish to.
     * Here we just illustrate the use of quality (quantization table) scaling:
     */
	if(quality<1) quality=1;
	if(quality>100) quality=100;
    jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);


    /* Step 4: Start compressor */

    /* TRUE ensures that we will write a complete interchange-JPEG file.
     * Pass TRUE unless you are very sure of what you're doing.
     */
    jpeg_start_compress(&cinfo, TRUE);

    row_data = (unsigned char*) malloc (width*3);
    pixelArray = imagedata->pixelArray;

    /* Step 5: while (scan lines remain to be written) */
    /*           jpeg_write_scanlines(...); */

    /* Here we use the library's state variable cinfo.next_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     * To keep things simple, we pass one scanline per call; you can pass
     * more if you wish, though.
     */
    while (cinfo.next_scanline < cinfo.image_height)
    {
        /* jpeg_write_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could pass
         * more than one scanline at a time if that's more convenient.
         */
        for(i=0; i<width; i++)
        {
            /* from (Blue, Green, Red) to (Red, Green, Blue) */
            row_data[i*3+0] = pixelArray[i*BPP+2];
            row_data[i*3+1] = pixelArray[i*BPP+1];
            row_data[i*3+2] = pixelArray[i*BPP+0];
        }
        pixelArray += width*BPP;
        (void) jpeg_write_scanlines(&cinfo, &row_data, 1);
    }

    /* Step 6: Finish compression */
    jpeg_finish_compress(&cinfo);


    /* free resources */
    free(row_data);
    fclose(outfile);

    /* Step 7: release JPEG compression object */
    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_compress(&cinfo);

    /* return success */
    return 1;
}
