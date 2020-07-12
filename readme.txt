
Read and write an image file.

Refer to rwif.h file.


To compile with GCC, execute:
    make

or  make UNZIP=unzip MOVE=mv COPY=cp

or, execute:
    make -f make_libjpeg
    make -f make_libpng CFLAGS="-Izlib"
    make -f make_zlib
    gcc -Ilibjpeg -Ilibpng *.c -Llibjpeg -Llibpng -Lzlib -ljpeg -lpng -lz -lm -o rwif


To compile on Windows with MinGW:

1) First get the folders:
    * libjpeg
    * libpng
    * zlib
   by extracting their corresponding ZIP
   file then renaming accordingly.

2) Install MinGW. Instructions are found at:
   http://www.mingw.org/wiki/Getting_Started
   By default it will install at C:\MinGW\

3) Add the folder C:\MinGW\bin to
   the PATH environment variable.
   (Search online for how to do this!)

4) Finally execute:
    copy libjpeg\jconfig.cfg libjpeg\jconfig.h
    copy libpng\scripts\pnglibconf.h.prebuilt libpng\pnglibconf.h
    mingw32-make.exe


To run, execute:
    ./rwif  input_file_name  output_file_name

Examples:
    ./rwif image.png image.jpg
    ./rwif image.png image.bmp
    ./rwif image.jpg image2.png
    ./rwif image.bmp image2.jpg


Make sure to read the licenses for:
* libjpeg
* libpng
* zlib

USE AT YOUR OWN RISK!
