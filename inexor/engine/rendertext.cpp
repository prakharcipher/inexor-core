#include <stdio.h>                            // for sscanf
#include <string.h>                           // for memcpy, memset, strlen
#include <algorithm>                          // for max

#include "SDL_opengl.h"                       // for glBindTexture, glBlendFunc
#include "inexor/engine/engine.hpp"           // for xtraverts
#include "inexor/engine/glemu.hpp"            // for attribf, attrib, end
#include "inexor/engine/rendertext.hpp"
#include "inexor/io/legacy/stream.hpp"        // for makerelpath
#include "inexor/shared/command.hpp"          // for COMMAND, getcurexecdir
#include "inexor/shared/cube_formatting.hpp"  // for defvformatstring
#include "inexor/shared/cube_hash.hpp"        // for hashnameset, hashbase
#include "inexor/shared/cube_loops.hpp"       // for i, loopv, loopi
#include "inexor/shared/cube_types.hpp"       // for uchar
#include "inexor/shared/geom.hpp"             // for bvec, matrix4x3, vec2
#include "inexor/shared/iengine.hpp"          // for fatal
#include "inexor/shared/tools.hpp"            // for max
#include "inexor/texture/texture.hpp"         // for textureload, Texture
#include "inexor/util/legacy_time.hpp"        // for totalmillis


static hashnameset<font> fonts;
static font *fontdef = nullptr;
static int fontdeftex = 0;

font *curfont = nullptr;
int curfonttex = 0;

void newfont(char *name, char *tex, int *defaultw, int *defaulth)
{
    font *f = &fonts[name];
    if(!f->name) f->name = newstring(name);
    f->texs.shrink(0);
    f->texs.add(textureload(tex, 0, true, false));
	if(f->texs.last() == notexture) 
	{ //try in same folder as the config file
		f->texs.last() = textureload(makerelpath(getcurexecdir(), tex));
	}
    f->chars.shrink(0);
    f->charoffset = '!';
    f->defaultw = *defaultw;
    f->defaulth = *defaulth;
    f->scale = f->defaulth;

    fontdef = f;
    fontdeftex = 0;
}

void fontoffset(char *c)
{
    if(!fontdef) return;
    
    fontdef->charoffset = c[0];
}

void fontscale(int *scale)
{
    if(!fontdef) return;

    fontdef->scale = *scale > 0 ? *scale : fontdef->defaulth; 
}

void fonttex(char *s)
{
    if(!fontdef) return;

    Texture *t = textureload(s, 0, true, false);
	if(t == notexture) 
	{ //try in same folder as the config file
		t = textureload(makerelpath(getcurexecdir(), s));
	}

    loopv(fontdef->texs) if(fontdef->texs[i] == t) { fontdeftex = i; return; }
    fontdeftex = fontdef->texs.length();
    fontdef->texs.add(t);
}

void fontchar(int *x, int *y, int *w, int *h, int *offsetx, int *offsety, int *advance)
{
    if(!fontdef) return;

    font::charinfo &c = fontdef->chars.add();
    c.x = *x;
    c.y = *y;
    c.w = *w ? *w : fontdef->defaultw;
    c.h = *h ? *h : fontdef->defaulth;
    c.offsetx = *offsetx;
    c.offsety = *offsety;
    c.advance = *advance ? *advance : c.offsetx + c.w;
    c.tex = fontdeftex;
}

void fontskip(int *n)
{
    if(!fontdef) return;
    loopi(max(*n, 1))
    {
        font::charinfo &c = fontdef->chars.add();
        c.x = c.y = c.w = c.h = c.offsetx = c.offsety = c.advance = c.tex = 0;
    }
}

COMMANDN(font, newfont, "ssii");
COMMAND(fontoffset, "s");
COMMAND(fontscale, "i");
COMMAND(fonttex, "s");
COMMAND(fontchar, "iiiiiii");
COMMAND(fontskip, "i");

void fontalias(const char *dst, const char *src)
{
    font *s = fonts.access(src);
    if(!s) return;
    font *d = &fonts[dst];
    if(!d->name) d->name = newstring(dst);
    d->texs = s->texs;
    d->chars = s->chars;
    d->charoffset = s->charoffset;
    d->defaultw = s->defaultw;
    d->defaulth = s->defaulth;
    d->scale = s->scale;

    fontdef = d;
    fontdeftex = d->texs.length()-1;
}

COMMAND(fontalias, "ss");

bool setfont(const char *name)
{
    font *f = fonts.access(name);
    if(!f) return false;
    curfont = f;
    return true;
}

static vector<font *> fontstack;

void pushfont()
{
    fontstack.add(curfont);
}

bool popfont()
{
    if(fontstack.empty()) return false;
    curfont = fontstack.pop();
    return true;
}

void gettextres(int &w, int &h)
{
    if(w < MINRESW || h < MINRESH)
    {
        if(MINRESW > w*MINRESH/h)
        {
            h = h*MINRESW/w;
            w = MINRESW;
        }
        else
        {
            w = w*MINRESH/h;
            h = MINRESH;
        }
    }
}

float text_widthf(const char *str) 
{
    float width, height;
    text_boundsf(str, width, height);
    return width;
}

#define FONTTAB (4*FONTW)
#define TEXTTAB(x) ((int((x)/FONTTAB)+1.0f)*FONTTAB)

void tabify(const char *str, int *numtabs)
{
    int tw = max(*numtabs, 0)*FONTTAB-1, tabs = 0;
    for(float w = text_widthf(str); w <= tw; w = TEXTTAB(w)) ++tabs;
    int len = strlen(str);
    char *tstr = newstring(len + tabs);
    memcpy(tstr, str, len);
    memset(&tstr[len], '\t', tabs);
    tstr[len+tabs] = '\0';
    stringret(tstr);
}

COMMAND(tabify, "si");
    
void draw_textf(const char *fstr, int left, int top, ...)
{
    defvformatstring(str, top, fstr);
    draw_text(str, left, top);
}

const matrix4x3 *textmatrix = nullptr;

static float draw_char(Texture *&tex, int c, float x, float y, float scale)
{
    font::charinfo &info = curfont->chars[c-curfont->charoffset];
    if(tex != curfont->texs[info.tex])
    {
        xtraverts += gle::end();
        tex = curfont->texs[info.tex];
        glBindTexture(GL_TEXTURE_2D, tex->id);
    }

    float x1 = x + scale*info.offsetx,
          y1 = y + scale*info.offsety,
          x2 = x + scale*(info.offsetx + info.w),
          y2 = y + scale*(info.offsety + info.h),
          tx1 = info.x / float(tex->xs),
          ty1 = info.y / float(tex->ys),
          tx2 = (info.x + info.w) / float(tex->xs),
          ty2 = (info.y + info.h) / float(tex->ys);

    if(textmatrix)
    {
        gle::attrib(textmatrix->transform(vec2(x1, y1))); gle::attribf(tx1, ty1);
        gle::attrib(textmatrix->transform(vec2(x2, y1))); gle::attribf(tx2, ty1);
        gle::attrib(textmatrix->transform(vec2(x2, y2))); gle::attribf(tx2, ty2);
        gle::attrib(textmatrix->transform(vec2(x1, y2))); gle::attribf(tx1, ty2);
    }
    else
    {
        gle::attribf(x1, y1); gle::attribf(tx1, ty1);
        gle::attribf(x2, y1); gle::attribf(tx2, ty1);
        gle::attribf(x2, y2); gle::attribf(tx2, ty2);
        gle::attribf(x1, y2); gle::attribf(tx1, ty2);
    }

    return scale*info.advance;
}

static void text_color(const char *curstring, int a, int &skipamount)
{
    bvec color;
    // [38;2;64;255;128m
    if(curstring && sscanf(curstring, "[38;2;%hhu;%hhu;%hhum", &color.r, &color.g, &color.b) == 3)
    {
        for(int i = 12; i < 18; i++)
            if(curstring[i] == 'm')
            {
                skipamount = i;
                break;
            }
        xtraverts += gle::end();
        gle::color(color, a);
    }
}

#define TEXTSKELETON \
    float y = 0, x = 0, scale = curfont->scale/float(curfont->defaulth);\
    int i;\
    for(i = 0; str[i]; i++)\
    {\
        TEXTINDEX(i)\
        int c = uchar(str[i]);\
        if(c=='\t')      { x = TEXTTAB(x); TEXTWHITE(i) }\
        else if(c==' ')  { x += scale*curfont->defaultw; TEXTWHITE(i) }\
        else if(c=='\n') { TEXTLINE(i) x = 0; y += FONTH; }\
        else if(c=='\x1b') { if(str[i+1]) { i++; TEXTCOLOR(i) }}\
        else if(curfont->chars.inrange(c-curfont->charoffset))\
        {\
            float cw = scale*curfont->chars[c-curfont->charoffset].advance;\
            if(cw <= 0) continue;\
            if(maxwidth != -1)\
            {\
                int j = i;\
                float w = cw;\
                for(; str[i+1]; i++)\
                {\
                    int c = uchar(str[i+1]);\
                    if(c=='\f') { if(str[i+2]) i++; continue; }\
                    if(i-j > 16) break;\
                    if(!curfont->chars.inrange(c-curfont->charoffset)) break;\
                    float cw = scale*curfont->chars[c-curfont->charoffset].advance;\
                    if(cw <= 0 || w + cw > maxwidth) break;\
                    w += cw;\
                }\
                if(x + w > maxwidth && j!=0) { TEXTLINE(j-1) x = 0; y += FONTH; }\
                TEXTWORD\
            }\
            else\
            { TEXTCHAR(i) }\
        }\
    }

//all the chars are guaranteed to be either drawable or color commands
#define TEXTWORDSKELETON \
                for(; j <= i; j++)\
                {\
                    TEXTINDEX(j)\
                    int c = uchar(str[j]);\
                    if(c=='\x1b') { if(str[j+1]) { j++; TEXTCOLOR(j) }}\
                    else { float cw = scale*curfont->chars[c-curfont->charoffset].advance; TEXTCHAR(j) }\
                }

#define TEXTEND(cursor) if(cursor >= i) { do { TEXTINDEX(cursor); } while(0); }

int text_visible(const char *str, float hitx, float hity, int maxwidth)
{
    #define TEXTINDEX(idx)
    #define TEXTWHITE(idx) if(y+FONTH > hity && x >= hitx) return idx;
    #define TEXTLINE(idx) if(y+FONTH > hity) return idx;
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += cw; TEXTWHITE(idx)
    #define TEXTWORD TEXTWORDSKELETON
    TEXTSKELETON
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
    return i;
}

//inverse of text_visible
void text_posf(const char *str, int cursor, float &cx, float &cy, int maxwidth) 
{
    #define TEXTINDEX(idx) if(idx == cursor) { cx = x; cy = y; break; }
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx)
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += cw;
    #define TEXTWORD TEXTWORDSKELETON if(i >= cursor) break;
    cx = cy = 0;
    TEXTSKELETON
    TEXTEND(cursor)
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
}

void text_boundsf(const char *str, float &width, float &height, int maxwidth)
{
    #define TEXTINDEX(idx)
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx) if(x > width) width = x;
    #define TEXTCOLOR(idx)
    #define TEXTCHAR(idx) x += cw;
    #define TEXTWORD x += w;
    width = 0;
    TEXTSKELETON
    height = y + FONTH;
    TEXTLINE(_)
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
}

void draw_text(const char *str, int left, int top, int r, int g, int b, int a, int cursor, int maxwidth) 
{
    #define TEXTINDEX(idx) if(idx == cursor) { cx = x; cy = y; }
    #define TEXTWHITE(idx)
    #define TEXTLINE(idx) 
    #define TEXTCOLOR(idx) if(usecolor) { int nextpos = 0; text_color(&str[idx], a, nextpos); idx += nextpos;}// colorstack, sizeof(colorstack), colorpos, color, a);
    #define TEXTCHAR(idx) draw_char(tex, c, left+x, top+y, scale); x += cw;
    #define TEXTWORD TEXTWORDSKELETON

    bvec color(r, g, b);
    int colorpos = 0;
    float cx = -FONTW, cy = 0;
    bool usecolor = true;
    if(a < 0) { usecolor = false; a = -a; }
    Texture *tex = curfont->texs[0];
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    gle::color(color, a);
    gle::defvertex(textmatrix ? 3 : 2);
    gle::deftexcoord0();
    gle::begin(GL_QUADS);
    TEXTSKELETON
    TEXTEND(cursor)
    xtraverts += gle::end();
    if(cursor >= 0 && (totalmillis/250)&1)
    {
        gle::color(color, a);
        if(maxwidth != -1 && cx >= maxwidth) { cx = 0; cy += FONTH; }
        draw_char(tex, '_', left+cx, top+cy, scale);
        xtraverts += gle::end();
    }
    #undef TEXTINDEX
    #undef TEXTWHITE
    #undef TEXTLINE
    #undef TEXTCOLOR
    #undef TEXTCHAR
    #undef TEXTWORD
}

void reloadfonts()
{
    enumerate(fonts, font, f,
        loopv(f.texs) if(!reloadtexture(*f.texs[i])) fatal("failed to reload font texture");
    );
}

