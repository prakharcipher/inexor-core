#pragma once

#include <iostream>
#include <string>                                       // for string
#include <utility>                                      // for move

#include "SDL_events.h"                                 // for SDL_Event
#include "SDL_opengl.h"                                 // for GL_ONE
#include "include/cef_app.h"
#include "include/cef_base.h"                           // for CefRefPtr
#include "include/cef_v8.h"                             // for CefV8Value (p...
#include "inexor/engine/engine.hpp"
#include "inexor/ui/context/InexorContextProvider.hpp"  // for InexorContext...
#include "inexor/ui/layer/InexorLayerManager.hpp"       // for InexorLayerMa...

#ifndef GL_ONE
#define GL_ONE 0x1
#endif

extern bool settexture(const char *name, int clamp);

namespace inexor {
namespace ui {
namespace input {

class InexorMouseManager : public inexor::ui::context::InexorContextProvider
{

    public:
        InexorMouseManager(CefRefPtr<inexor::ui::layer::InexorLayerManager> layer_manager, int screen_width, int screen_height)
            : layer_manager(std::move(layer_manager)),
              screen_width(screen_width),
              screen_height(screen_height),
              x(screen_width / 2),
              y(screen_height / 2),
              scale_x(0.015f),
              scale_y(0.025f),
              visible(true),
              texture("interface/cursor/default.png") {};


        bool IsVisible() { return visible; };
        void Hide() { visible = false; };
        void Show() { visible = true; };
        void SetScreenSize(int screen_width, int screen_height) { this->screen_width = screen_width; this->screen_height = screen_height; };
        int GetAbsoluteX() { return x; };
        int GetAbsoluteY() { return y; };
        int GetScreenWidth() { return screen_width; };
        int GetScreenHeight() { return screen_height; };
        float GetScaledX() { return (float) x / (float) screen_width; };
        float GetScaledY() { return (float) y / (float) screen_height; };
        int GetWidth() { return screen_width * scale_x; };
        int GetHeight() { return screen_height * scale_y; };
        std::string GetTexture() { return texture; };
        void SetTexture(std::string texture) { this->texture = texture; };

        // SDL Events
        void SendMouseMoveEvent(SDL_Event &e);
        void SendMouseClickEvent(SDL_Event &e);
        void SendMouseWheelEvent(SDL_Event &e);

        // InexorContextProvider
        void InitializeContext() override;
        bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override;
        bool Get(const CefString& name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval, CefString& exception) override;
        bool Set(const CefString& name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString& exception) override;
        std::string GetContextName() override { return "mouse"; };

	private:
        // Layer Manager
        CefRefPtr<inexor::ui::layer::InexorLayerManager> layer_manager;

        // The width and height of the screen
        int screen_width;
        int screen_height;

        // The position of the mouse on the screen
        int x;
        int y;

        // The width/height of the mouse in percent
        // scale_x = 0.03 => mouse width 3% of the screen width
        float scale_x;
        float scale_y;

        // If true, the mouse is visible
        bool visible;

        // The texture of the mouse
        std::string texture;

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(InexorMouseManager);

};

}
}
}

