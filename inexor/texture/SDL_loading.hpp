/// @file SDL_loading.hpp
/// Wrapper for the SDL API calls used to load textures.

#pragma once

#include "SDL_image.h"
#include "SDL_surface.h"  // for SDL_Surface

extern SDL_Surface *wrapsurface(void *data, int width, int height, int bpp);

extern bool canloadsurface(const char *name);
extern SDL_Surface *loadsurface(const char *name);


