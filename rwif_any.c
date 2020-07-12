/*
    rwif_any.c
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "rwif.h"

#define toLowerCase(c) (('A'<=(c) && (c)<='Z') ? ((c) + 'a'-'A') : (c))


/* a string containing the latest
 * error message of a failed operation.
 *
 * defined here, but used in here as
 * well as in all the sub-routines.
 */
char rwif_errormessage[1000];

void clear_image_data (ImageData* imagedata)
{
	free(imagedata->pixelArray);
	memset(imagedata, 0, sizeof(*imagedata));
}


/* read_image_file(...)
 *
 * input: filename
 * output: modified ImageData
 * return success 1 or failure 0
 *
 * The caller is responsible to free memory allocated by
 * this function, by calling: free(ImageData.pixelArray);
 * The free() function is available in <malloc.h>
 */
int read_image_file (const char* filename, ImageData *imagedata)
{
    int i;

    i = read_image_file_bmp (filename, imagedata);
    if(i<0) return 0;  /* if file is a corrupted bmp image */
    if(i>0) return 1;  /* if file is loaded successfully */
    /* else i==0, file is not a bmp image */

    i = read_image_file_png (filename, imagedata);
    if(i<0) return 0;  /* if file is a corrupted png image */
    if(i>0) return 1;  /* if file is loaded successfully */
    /* else i==0, file is not a png image */

    i = read_image_file_jpg (filename, imagedata);
    if(i<0) return 0;  /* if file is a corrupted jpg image */
    if(i>0) return 1;  /* if file is loaded successfully */
    /* else i==0, file is not a jpg image */

    sprintf (rwif_errormessage, "Read Error: file '%s' is not a BMP nor PNG nor JPEG image file.", filename);
    return 0;
}


/* write_image_file(...)
 *
 * inputs: filename, pointer to ImageData
 * return success 1 or failure 0
 *
 * The image file type is specified by
 * the extension part of the file name.
 */
int write_image_file (const char* filename, const ImageData *imagedata)
{
    int i, j, k;
    char extension[100];

    if(filename==NULL) return 0;

    /* get i = length of file name, and
       get j = position of last '.' character */
    j=0;
    for(i=0; filename[i] != 0; i++)
        if(filename[i]=='.') j=i;

    if(j>0)
    {
        /* get the file extension in lower-case */
        for(k=0, j++; j<i; j++)
            extension[k++] = toLowerCase(filename[j]);
        extension[k] = 0;

        if(0==strcmp(extension, "bmp")) return write_image_file_bmp (filename, imagedata);
        if(0==strcmp(extension, "png")) return write_image_file_png (filename, imagedata);
        if(0==strcmp(extension, "jpg")
        || 0==strcmp(extension, "jpeg")) return write_image_file_jpg (filename, imagedata, 96);
    }

    sprintf (rwif_errormessage, "Write Error on '%s':\r\nfile extension is not one of: bmp, png, jpg, jpeg.", filename);
    return 0;
}
