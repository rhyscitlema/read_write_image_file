/*
    rwif_bmp.c

    See: https://en.wikipedia.org/wiki/BMP_file_format
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "rwif.h"

#define BI_RGB 0
#define BI_BITFIELDS 3
#define CT(bpp) (bpp==24 ? BI_RGB : BI_BITFIELDS) /* Compression Type */



typedef struct _Windows_BMP_Header
{
    unsigned short signature;               /* must be 0x4D42 */
    unsigned int filesize;
    unsigned int reserved;
    unsigned int offsetToStartOfImageData;
    unsigned int sizeOfBitmapInfoHeader;    /* must be 40 */
    unsigned int width;
    unsigned int height;
    unsigned short numberOfPlanes;          /* must be 1 */
    unsigned short bitsPerPixel;
    unsigned int compressionType;           /* must be CT(bpp) */
    unsigned int sizeOfImageData;           /* includes padding */
    unsigned int horizontalResolutionPPM;
    unsigned int verticalResolutionPPM;
    unsigned int numberOfImageColours;
    unsigned int numberOfImportantColours;
} BMP_Header;



static void print_bmp_header (BMP_Header* bmp)
{
    #ifdef DEBUG
    printf(".signature = %x\n", bmp->signature);
    printf(".filesize = %u\n", bmp->filesize);
    printf(".reserved = %u\n", bmp->reserved);
    printf(".offsetToStartOfImageData = %u\n", bmp->offsetToStartOfImageData);
    printf(".sizeOfBitmapInfoHeader = %u\n", bmp->sizeOfBitmapInfoHeader);
    printf(".width = %u\n", bmp->width);
    printf(".height = %u\n", bmp->height);
    printf(".numberOfPlanes = %u\n", bmp->numberOfPlanes);
    printf(".bitsPerPixel = %u\n", bmp->bitsPerPixel);
    printf(".compressionType = %u\n", bmp->compressionType);
    printf(".sizeOfImageData = %u\n", bmp->sizeOfImageData);
    printf(".horizontalResolutionPPM = %u\n", bmp->horizontalResolutionPPM);
    printf(".verticalResolutionPPM = %u\n", bmp->verticalResolutionPPM);
    printf(".numberOfImageColours = %u\n", bmp->numberOfImageColours);
    printf(".numberOfImportantColours = %u\n", bmp->numberOfImportantColours);
    #endif
}



/* return -2 if file cannot be opened
 * return -1 if file data is corrupted
 * return  0 if file format is not valid
 * return  1 if file successfully loaded
 */
int read_image_file_bmp (const char* filename, ImageData *imagedata)
{
    unsigned int i, j;
    unsigned char *filecontent=NULL, *content;
    unsigned char *colour, *pixelArray;
    BMP_Header bmpHeader;

    FILE *file = fopen (filename, "rb");
    int ret; /* return value */
    do{
        if(!file) {
            sprintf (rwif_errormessage, "Read Error: file '%s' could not be opened for reading.", filename);
            ret = -2; break;
        }

        if(!fread(&bmpHeader.signature, sizeof(bmpHeader.signature), 1, file)
        || !fread(&bmpHeader.filesize , sizeof(bmpHeader.filesize ), 1, file)
        || bmpHeader.signature != 0x4D42)
        {
            sprintf (rwif_errormessage, "Read Error: file '%s' is not a valid BMP file.", filename);
            ret = 0; break;
        }

        i = sizeof(bmpHeader.signature) + sizeof(bmpHeader.filesize);
        i = bmpHeader.filesize-i;
        filecontent = (unsigned char*) malloc (i);

        if(fread(filecontent, 1, i, file) != i)
        {
            sprintf (rwif_errormessage, "Read Error: file '%s' expected size != actual size.", filename);
            ret = -1; break;
        }

        content = filecontent;
        bmpHeader.reserved                   = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.offsetToStartOfImageData   = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.sizeOfBitmapInfoHeader     = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.width                      = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.height                     = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.numberOfPlanes             = *((unsigned short*)content); content += sizeof(unsigned short);
        bmpHeader.bitsPerPixel               = *((unsigned short*)content); content += sizeof(unsigned short);
        bmpHeader.compressionType            = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.sizeOfImageData            = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.horizontalResolutionPPM    = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.verticalResolutionPPM      = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.numberOfImageColours       = *((unsigned int  *)content); content += sizeof(unsigned int  );
        bmpHeader.numberOfImportantColours   = *((unsigned int  *)content); content += sizeof(unsigned int  );
        print_bmp_header(&bmpHeader);

        if(bmpHeader.numberOfPlanes != 1)
        {
            sprintf (rwif_errormessage, "Read Error: file '%s' is expected to have number of planes = 1 not %d.", filename, bmpHeader.numberOfPlanes);
            ret = -1; break;
        }
        if(bmpHeader.compressionType != BI_RGB && bmpHeader.compressionType != BI_BITFIELDS)
        {
            sprintf (rwif_errormessage, "Read Error: file '%s' is expected to have compression type = %d or %d not %d.", filename, BI_RGB, BI_BITFIELDS, bmpHeader.compressionType);
            ret = -1; break;
        }
        if(bmpHeader.bitsPerPixel != 24 && bmpHeader.bitsPerPixel != 32)
        {
            sprintf (rwif_errormessage, "Read Error: file '%s' is expected to have bits-per-pixel = 24 or 32 not %d.", filename, bmpHeader.bitsPerPixel);
            ret = -1; break;
        }

        content = filecontent-2-4 + bmpHeader.offsetToStartOfImageData; /* filecontent starts after signature and filesize were read */

        i = bmpHeader.height * bmpHeader.width * (bmpHeader.bitsPerPixel/8);
        pixelArray = (unsigned char*)malloc(i);
        colour = pixelArray + i;

        if(bmpHeader.bitsPerPixel == 24)
        {
            for(j = 0; j < bmpHeader.height; j++)
            {
                colour -= bmpHeader.width*3;
                for(i = 0; i < bmpHeader.width; i++)
                {
                    colour[i*3+0] = content[i*3+0];
                    colour[i*3+1] = content[i*3+1];
                    colour[i*3+2] = content[i*3+2];
                }
                content += ((bmpHeader.width*24+31)/32)*4;
            }
        }
        else /* (bmpHeader.bitsPerPixel == 32) */
        {
            for(j = 0; j < bmpHeader.height; j++)
            {
                colour -= bmpHeader.width*4;
                for(i = 0; i < bmpHeader.width; i++)
                {
                    colour[i*4+0] = content[i*4+0];
                    colour[i*4+1] = content[i*4+1];
                    colour[i*4+2] = content[i*4+2];
                    colour[i*4+3] = content[i*4+3];
                }
                content += bmpHeader.width*4;
            }
        }

        /* success */
        imagedata->pixelArray = pixelArray;
        imagedata->height = bmpHeader.height;
        imagedata->width = bmpHeader.width;
        imagedata->bpp = bmpHeader.bitsPerPixel;
        ret = 1;
    }while(0);

    /* free resources */
    free(filecontent);
    if(file) fclose(file);
    return ret;
}



/* return success 1 or failure 0 */
int write_image_file_bmp (const char* filename, const ImageData *imagedata)
{
	unsigned int i, j, size, width, height;
	unsigned short bpp;
    unsigned char *filecontent, *content, *colour;
    FILE* file;

    height = imagedata->height;
    width = imagedata->width;
    bpp = imagedata->bpp;

    if(bpp!=24 && bpp!=32)
    { sprintf(rwif_errormessage, "Write Error on '%s': bits-per-pixel must be 24 or 32 not %d.", filename, bpp); return 0; }

    /* open file for writing-bytes */
    file = fopen(filename, "wb");
    if(!file) { sprintf(rwif_errormessage, "Write Error: file '%s' could not be opened for writing.", filename); return 0; }

    i = CT(bpp) == BI_BITFIELDS ? 4*4 : 0;
    size = 54+i + height*((width*bpp+31)/32)*4;
    filecontent = (unsigned char*)malloc(size);
    content = filecontent;

    *((unsigned short*)content) = 0x4D42 ; content += sizeof(unsigned short); /* signature */
    *((unsigned int  *)content) = size   ; content += sizeof(unsigned int  ); /* filesize */
    *((unsigned int  *)content) = 0      ; content += sizeof(unsigned int  ); /* reserved */
    *((unsigned int  *)content) = 54+i   ; content += sizeof(unsigned int  ); /* offsetToStartOfImageData */
    *((unsigned int  *)content) = 40+i   ; content += sizeof(unsigned int  ); /* sizeOfBitmapInfoHeader */
    *((unsigned int  *)content) = width  ; content += sizeof(unsigned int  ); /* width */
    *((unsigned int  *)content) = height ; content += sizeof(unsigned int  ); /* height */
    *((unsigned short*)content) = 1      ; content += sizeof(unsigned short); /* numberOfPlanes */
    *((unsigned short*)content) = bpp    ; content += sizeof(unsigned short); /* bitsPerPixel */
    *((unsigned int  *)content) = CT(bpp); content += sizeof(unsigned int  ); /* compressionType */
    *((unsigned int  *)content) = size-54; content += sizeof(unsigned int  ); /* sizeOfImageData */
    *((unsigned int  *)content) = 3780   ; content += sizeof(unsigned int  ); /* horizontalResolutionPPM */
    *((unsigned int  *)content) = 3780   ; content += sizeof(unsigned int  ); /* verticalResolutionPPM */
    *((unsigned int  *)content) = 0      ; content += sizeof(unsigned int  ); /* numberOfImageColours */
    *((unsigned int  *)content) = 0      ; content += sizeof(unsigned int  ); /* numberOfImportantColours */

    if(i>0) /* if BI_BITFIELDS is specified */
    {
        *((unsigned int*)content) = 0x00FF0000; content += sizeof(unsigned int); /* Red channel bit mask */
        *((unsigned int*)content) = 0x0000FF00; content += sizeof(unsigned int); /* Green channel bit mask */
        *((unsigned int*)content) = 0x000000FF; content += sizeof(unsigned int); /* Blue channel bit mask */
        *((unsigned int*)content) = 0xFF000000; content += sizeof(unsigned int); /* Alpha channel bit mask */
    }

    colour = imagedata->pixelArray + height*width*bpp/8;

    if(bpp==24)
    {
        for(j=0; j < height; j++)
        {
            colour -= width*3;
            for(i=0; i < width; i++)
            {
                content[i*3+0] = colour[i*3+0];
                content[i*3+1] = colour[i*3+1];
                content[i*3+2] = colour[i*3+2];
            }
            content += ((width*24+31)/32)*4;
        }
    }
    else /* bpp==32 */
    {
        for(j=0; j < height; j++)
        {
            colour -= width*4;
            for(i=0; i < width; i++)
            {
                content[i*4+0] = colour[i*4+0];
                content[i*4+1] = colour[i*4+1];
                content[i*4+2] = colour[i*4+2];
                content[i*4+3] = colour[i*4+3];
            }
            content += width*4;
        }
    }

    fwrite(filecontent, 1, size, file);
    free(filecontent);
    fclose(file);
    return 1;
}
