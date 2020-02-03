# OBJ (.o) files will be compiled -c first 
# then the linker will be called -o to link th files

#OBJS specifies which files to compile as part of the project
OBJS = main.o LTexture.o

#CC specifies which compiler we're using
CC = g++

#CFLAGS additional compilation options e.g w - suppresses all warnings
CFLAGS = -w

#LINKER_FLAGS specifies the libraries we're linking against
LFLAGS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer

SOURCE = .

# Executable name
OBJ_NAME = run

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(OBJ_NAME) $(OBJS) $(LFLAGS)

main.o: main.cpp
	$(CC) -c $(SOURCE)/main.cpp

LTexture.o: LTexture.cpp
	$(CC) -c $(SOURCE)/LTexture.cpp

#This is the target that compiles our executable
clean:
	rm $(OBJ_NAME) $(OBJS)

