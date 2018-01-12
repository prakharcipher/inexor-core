/// @file SDL_loading.cpp
/// Wrapper for the SDL API calls used to load textures.

#include <algorithm>                     // for min

#include "SDL_blendmode.h"               // for ::SDL_BLENDMODE_NONE
#include "SDL_endian.h"                  // for SDL_BIG_ENDIAN, SDL_BYTEORDER
#include "SDL_image.h"                   // for IMG_Load
#include "SDL_pixels.h"                  // for SDL_PixelFormat, SDL_Color
#include "inexor/io/legacy/stream.hpp"   // for findfile, openfile, stream
#include "inexor/shared/cube_loops.hpp"  // for i, loopi
#include "inexor/shared/cube_types.hpp"  // for uint
#include "inexor/shared/tools.hpp"       // for min
#include "inexor/texture/SDL_loading.hpp"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define RGBAMASKS 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#define RGBMASKS  0xff0000, 0x00ff00, 0x0000ff, 0
#else
#define RGBAMASKS 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#define RGBMASKS  0x0000ff, 0x00ff00, 0xff0000, 0
#endif

SDL_Surface *wrapsurface(void *data, int width, int height, int bpp)
{
    switch(bpp)
    {
    case 3: return SDL_CreateRGBSurfaceFrom(data, width, height, 8 * bpp, bpp*width, RGBMASKS);
    case 4: return SDL_CreateRGBSurfaceFrom(data, width, height, 8 * bpp, bpp*width, RGBAMASKS);
    }
    return nullptr;
}

SDL_Surface *creatergbsurface(SDL_Surface *os)
{
    SDL_Surface *ns = SDL_CreateRGBSurface(SDL_SWSURFACE, os->w, os->h, 24, RGBMASKS);
    if(ns) SDL_BlitSurface(os, nullptr, ns, nullptr);
    SDL_FreeSurface(os);
    return ns;
}

SDL_Surface *creatergbasurface(SDL_Surface *os)
{
    SDL_Surface *ns = SDL_CreateRGBSurface(SDL_SWSURFACE, os->w, os->h, 32, RGBAMASKS);
    if(ns)
    {
        SDL_SetSurfaceBlendMode(os, SDL_BLENDMODE_NONE);
        SDL_BlitSurface(os, nullptr, ns, nullptr);
    }
    SDL_FreeSurface(os);
    return ns;
}

bool checkgrayscale(SDL_Surface *s)
{
    // gray scale images have 256 levels, no colorkey, and the palette is a ramp
    if(s->format->palette)
    {
        if(s->format->palette->ncolors != 256 || SDL_GetColorKey(s, nullptr) >= 0) return false;
        const SDL_Color *colors = s->format->palette->colors;
        loopi(256) if(colors[i].r != i || colors[i].g != i || colors[i].b != i) return false;
    }
    return true;
}

SDL_Surface *fixsurfaceformat(SDL_Surface *s)
{
    if(!s) return nullptr;
    if(!s->pixels || min(s->w, s->h) <= 0 || s->format->BytesPerPixel <= 0)
    {
        SDL_FreeSurface(s);
        return nullptr;
    }
    static const uint rgbmasks[] = { RGBMASKS }, rgbamasks[] = { RGBAMASKS };
    switch(s->format->BytesPerPixel)
    {
    case 1:
        if(!checkgrayscale(s)) return SDL_GetColorKey(s, nullptr) >= 0 ? creatergbasurface(s) : creatergbsurface(s);
        break;
    case 3:
        if(s->format->Rmask != rgbmasks[0] || s->format->Gmask != rgbmasks[1] || s->format->Bmask != rgbmasks[2])
            return creatergbsurface(s);
        break;
    case 4:
        if(s->format->Rmask != rgbamasks[0] || s->format->Gmask != rgbamasks[1] || s->format->Bmask != rgbamasks[2] || s->format->Amask != rgbamasks[3])
            return s->format->Amask ? creatergbasurface(s) : creatergbsurface(s);
        break;
    }
    return s;
}

bool canloadsurface(const char *name)
{
    stream *f = openfile(name, "rb");
    if(!f) return false;
    delete f;
    return true;
}

SDL_Surface *loadsurface(const char *name)
{
    SDL_Surface *s = IMG_Load(findfile(name, "rb"));
    return fixsurfaceformat(s);
}
