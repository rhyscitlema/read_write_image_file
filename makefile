# Makefile created by Rhyscitlema
# Explanation of file structure available at:
# http://rhyscitlema.com/applications/makefile.html

# executable output file
EXE_OUT_FILE = rwif

# library output file
LIB_OUT_FILE = librwif.a

OBJECT_FILES = rwif_any.o \
               rwif_bmp.o \
               rwif_png.o \
               rwif_jpeg.o

LIBJPEG = ./libjpeg
LIBPNG  = ./libpng
ZLIB    = ./zlib

#-------------------------------------------------

# C compiler
CC = gcc

# archiver
AR = ar

# Linker
LD = gcc

# compiler flags
CC_FLAGS = -Wall -ansi -pedantic $(CFLAGS)

# archiver flags
AR_FLAGS = -crs #$(ARFLAGS)

# needed linker libs
LD_LIBS = -L. -lrwif -lm $(LDLIBS)

#-------------------------------------------------

make: main.o lib
	$(LD) main.o $(LD_LIBS) -o $(EXE_OUT_FILE)

use_os_png: main.o lib_use_os_png
	$(LD) main.o $(LD_LIBS) -lpng -o $(EXE_OUT_FILE)

use_os_png_jpeg: main.o lib_use_os_png_jpeg
	$(LD) main.o $(LD_LIBS) -lpng -ljpeg -o $(EXE_OUT_FILE)


lib:
	$(MAKE) -f make_zlib
	$(MAKE) -f make_libpng CFLAGS+="-I$(ZLIB)"
	$(MAKE) -f make_libjpeg
	$(MAKE) objects CFLAGS+="-I$(LIBJPEG) -I$(LIBPNG)"
	$(AR) $(AR_FLAGS) $(LIB_OUT_FILE) rwif_*.o $(LIBJPEG)/*.o $(LIBPNG)/*.o $(ZLIB)/*.o
# TODO: instead merge rwif_*.o $(LIBJPEG)/libjpeg.a $(LIBPNG)/libpng.a $(ZLIB)libzlib.a

# use libpng of operating system
lib_use_os_png:
	$(MAKE) -f make_libjpeg
	$(MAKE) objects CFLAGS+="-I$(LIBJPEG)"
	$(AR) $(AR_FLAGS) $(LIB_OUT_FILE) *.o $(LIBJPEG)/*.o

# use libpng and libjpeg of operating system
lib_use_os_png_jpeg:
	$(MAKE) objects
	$(AR) $(AR_FLAGS) $(LIB_OUT_FILE) *.o


# remove all created files
clean:
	$(RM) *.o $(EXE_OUT_FILE) $(LIB_OUT_FILE)
	$(MAKE) clean -f make_libjpeg
	$(MAKE) clean -f make_libpng
	$(MAKE) clean -f make_zlib

#-------------------------------------------------

objects: $(OBJECT_FILES)

# compile .c files to .o files
%.o: %.c *.h
	$(CC) $(CC_FLAGS) -c -o $@ $<

