#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdio.h>
#include <string>
#include <fstream>

#include "LTexture.h"
#include <queue>

// Main functions
bool init();
bool loadMedia();
void close();
void intializeMap();
void makeShapes();

// Utility functions

void loadBlocks();
void dropBlock();
void deleteRow();
void checkCollisions();
bool updateScore(int& score);

