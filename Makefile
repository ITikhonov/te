CFALGS=-g -Wall
LDLIBS=-lSDL -lSDL_image -lpng

all: te

te: te.o pngwrite/IMG_savepng.o

