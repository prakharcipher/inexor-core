#pragma once

#include "inexor/enumerations/editor_states.hpp"
#include "inexor/shared/geom.hpp"

struct Texture;
class VSlot;

/// 
struct g3d_gui
{
    virtual ~g3d_gui() {}

    virtual void start(int starttime, float basescale, int *tab = NULL, bool allowinput = true) = 0;
    virtual void end() = 0;

    virtual int text(const char *text, int color, const char *icon = NULL) = 0;
    int textf(const char *fmt, int color, const char *icon = NULL, ...) PRINTFARGS(2, 5)
    {
        defvformatstring(str, icon, fmt);
        return text(str, color, icon);
    }
    virtual int button(const char *text, int color, const char *icon = NULL) = 0;
    int buttonf(const char *fmt, int color, const char *icon = NULL, ...) PRINTFARGS(2, 5)
    {
        defvformatstring(str, icon, fmt);
        return button(str, color, icon);
    }
    virtual int title(const char *text, int color, const char *icon = NULL) = 0;
    int titlef(const char *fmt, int color, const char *icon = NULL, ...) PRINTFARGS(2, 5)
    {
        defvformatstring(str, icon, fmt);
        return title(str, color, icon);
    }
    virtual void background(int color, int parentw = 0, int parenth = 0) = 0;

    virtual void pushlist() {}
    virtual void poplist() {}

    virtual bool allowautotab(bool on) = 0;
    virtual bool shouldtab() { return false; }
    virtual void tab(const char *name = NULL, int color = 0) = 0;
    virtual int image(Texture *t, float scale, bool overlaid = false) = 0;
    virtual int texture(VSlot &vslot, float scale, bool overlaid = true) = 0;
    virtual int playerpreview(int model, int team, int weap, float scale, bool overlaid = false) { return 0; }
    virtual int modelpreview(const char *name, int anim, float scale, bool overlaid = false) { return 0; }
    virtual int prefabpreview(const char *prefab, const vec &color, float scale, bool overlaid = false) { return 0; }
    virtual void slider(int &val, int vmin, int vmax, int color, const char *label = NULL) = 0;
    virtual void separator() = 0;
    virtual void progress(float percent) = 0;
    virtual void strut(float size) = 0;
    virtual void space(float size) = 0;
    virtual void spring(int weight = 1) = 0;
    virtual void column(int col) = 0;
    virtual char *keyfield(const char *name, int color, int length, int height = 0, const char *initval = NULL, int initmode = EDITORFOCUSED) = 0;
    virtual char *field(const char *name, int color, int length, int height = 0, const char *initval = NULL, int initmode = EDITORFOCUSED) = 0;
    virtual void textbox(const char *text, int width, int height, int color = 0xFFFFFF) = 0;
    virtual bool mergehits(bool on) = 0;
};
