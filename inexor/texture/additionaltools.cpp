/// @file additionaltools.cpp
/// additional texture generation tools provided by the engine.

#include <SDL_opengl.h>                       // for GL_COMPRESSED_RGB_S3TC_...
#include <string.h>                           // for memcpy, memset, strlen
#include <algorithm>                          // for max
#include <memory>                             // for __shared_ptr

#include "SDL_opengl.h"                       // for glGetTexLevelParameteriv
#include "inexor/engine/engine.hpp"           // for hasS3TC
#include "inexor/engine/glexts.hpp"           // for glGetCompressedTexImage_
#include "inexor/io/Logging.hpp"              // for Log, Logger, log_manager
#include "inexor/io/legacy/stream.hpp"        // for stream, openfile, path
#include "inexor/network/SharedVar.hpp"       // for SharedVar
#include "inexor/shared/cube_endian.hpp"      // for lilswap
#include "inexor/shared/cube_formatting.hpp"  // for concatstring, defformat...
#include "inexor/shared/cube_tools.hpp"       // for copystring
#include "inexor/shared/cube_types.hpp"       // for uchar, uint, string
#include "inexor/shared/geom.hpp"             // for bvec, vec
#include "inexor/shared/tools.hpp"            // for max
#include "inexor/texture/compressedtex.hpp"   // for DDSURFACEDESC2, DDSURFA...
#include "inexor/texture/image.hpp"           // for ImageData
#include "inexor/texture/macros.hpp"          // for dst, src, read2writetex
#include "inexor/texture/savetexture.hpp"     // for guessimageformat, savei...
//#include "inexor/texture/additionaltools.hpp"
#include "inexor/texture/texsettings.hpp"     // for usetexcompress
#include "inexor/texture/texture.hpp"         // for loadimage, gettexture

void flipnormalmapy(char *destfile, char *normalfile) // jpg/png /tga-> tga
{
    ImageData ns;
    if(!loadimage(normalfile, ns)) return;
    ImageData d(ns.w, ns.h, 3);
    readwritetex(d, ns,
                 dst[0] = src[0];
    dst[1] = 255 - src[1];
    dst[2] = src[2];
    );
    saveimage(destfile, guessimageformat(destfile, IMG_TGA), d);
}

void mergenormalmaps(char *heightfile, char *normalfile) // jpg/png/tga + tga -> tga
{
    ImageData hs, ns;
    if(!loadimage(heightfile, hs) || !loadimage(normalfile, ns) || hs.w != ns.w || hs.h != ns.h) return;
    ImageData d(ns.w, ns.h, 3);
    read2writetex(d, hs, srch, ns, srcn,
                  *(bvec *)dst = bvec(((bvec *)srcn)->tonormal().mul(2).add(((bvec *)srch)->tonormal()).normalize());
    );
    saveimage(normalfile, guessimageformat(normalfile, IMG_TGA), d);
}

void gendds(char *infile, char *outfile)
{
    if(!hasS3TC || usetexcompress <= 1) { Log.std->error("OpenGL driver does not support S3TC texture compression"); return; }

    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

    defformatstring(cfile, "<compress>%s", infile);
    extern void reloadtex(char *name);
    Texture *t = gettexture(cfile);
    if(t) reloadtex(cfile);
    t = textureload(cfile);
    if(t == notexture) { Log.std->error("failed loading {}", infile); return; }

    glBindTexture(GL_TEXTURE_2D, t->id);
    GLint compressed = 0, format = 0, width = 0, height = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

    if(!compressed) { Log.std->error("failed compressing {}", infile); return; }
    int fourcc = 0;
    switch(format)
    {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: fourcc = FOURCC_DXT1; Log.std->info("compressed as DXT1"); break;
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: fourcc = FOURCC_DXT1; Log.std->info("compressed as DXT1a"); break;
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: fourcc = FOURCC_DXT3; Log.std->info("compressed as DXT3"); break;
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: fourcc = FOURCC_DXT5; Log.std->info("compressed as DXT5"); break;
    default:
        Log.std->error("failed compressing {0}: unknown format: {1:#x}", infile, format); break;
        return;
    }

    if(!outfile[0])
    {
        static string buf;
        copystring(buf, infile);
        int len = strlen(buf);
        if(len > 4 && buf[len - 4] == '.') memcpy(&buf[len - 4], ".dds", 4);
        else concatstring(buf, ".dds");
        outfile = buf;
    }

    stream *f = openfile(path(outfile, true), "wb");
    if(!f) { Log.std->error("failed writing to {}", outfile); return; }

    int csize = 0;
    for(int lw = width, lh = height, level = 0;;)
    {
        GLint size = 0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, level++, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &size);
        csize += size;
        if(max(lw, lh) <= 1) break;
        if(lw > 1) lw /= 2;
        if(lh > 1) lh /= 2;
    }

    DDSURFACEDESC2 d;
    memset(&d, 0, sizeof(d));
    d.dwSize = sizeof(DDSURFACEDESC2);
    d.dwWidth = width;
    d.dwHeight = height;
    d.dwLinearSize = csize;
    d.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT | DDSD_LINEARSIZE | DDSD_MIPMAPCOUNT;
    d.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
    d.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    d.ddpfPixelFormat.dwFlags = DDPF_FOURCC | (format != GL_COMPRESSED_RGB_S3TC_DXT1_EXT ? DDPF_ALPHAPIXELS : 0);
    d.ddpfPixelFormat.dwFourCC = fourcc;

    uchar *data = new uchar[csize], *dst = data;
    for(int lw = width, lh = height;;)
    {
        GLint size;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, d.dwMipMapCount, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &size);
        glGetCompressedTexImage_(GL_TEXTURE_2D, d.dwMipMapCount++, dst);
        dst += size;
        if(max(lw, lh) <= 1) break;
        if(lw > 1) lw /= 2;
        if(lh > 1) lh /= 2;
    }

    lilswap((uint *)&d, sizeof(d) / sizeof(uint));

    f->write("DDS ", 4);
    f->write(&d, sizeof(d));
    f->write(data, csize);
    delete f;

    delete[] data;

    Log.std->info("wrote DDS file {}", outfile);

    setuptexcompress();
}
