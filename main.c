/*
    main.c
*/
#include <stdio.h>
#include "rwif.h"

int main (int argc, char** argv)
{
    ImageData imagedata = {0};
    if(argc!=3) printf("Usage: %s  input_file_name  output_file_name\r\n", argv[0]);
    else if(!read_image_file (argv[1], &imagedata)) puts(rwif_errormessage);
    else if(!write_image_file(argv[2], &imagedata)) puts(rwif_errormessage);
    else puts("Success!");
    clear_image_data(&imagedata);
    return 0;
}

