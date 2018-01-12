#pragma once

#include <list>                                         // for list, list<>:...
#include <string>                                       // for string

#include "include/cef_app.h"
#include "include/cef_base.h"                           // for CefRefPtr
#include "include/cef_browser.h"                        // for CefBrowserHost
#include "include/cef_v8.h"                             // for CefV8Value (p...
#include "inexor/ui/context/InexorContextProvider.hpp"  // for InexorContext...
#include "inexor/ui/layer/InexorLayer.hpp"              // for InexorLayer
#include "inexor/ui/layer/InexorLayerProvider.hpp"

namespace inexor {
namespace ui {
namespace layer {

class InexorLayerProvider;

class InexorLayerManager : public inexor::ui::context::InexorContextProvider
{

    public:
        InexorLayerManager(int width, int height);

        // Rendering
        // void Render();
        // void RenderLayer(std::string name);
        void SetScreenSize(int width, int height);
        int GetScreenWidth() { return width; };
        int GetScreenHeight() { return height; };

        // Layers
        void InitializeLayers();
        void InitializeLayer(CefRefPtr<InexorLayerProvider> layer_provider);
        void DestroyLayers();
        void AddLayerProvider(CefRefPtr<InexorLayerProvider> layer_provider);

        CefRefPtr<InexorLayer> CreateLayer(std::string name, std::string url);
        CefRefPtr<InexorLayer> CreateLayer(std::string name, int x, int y, int width, int height, std::string url);
        CefRefPtr<InexorLayer> GetLayer(std::string name);
        std::list<std::string> GetLayers();
        std::list<CefRefPtr<InexorLayer> > GetLayerList() { return layers; };
        bool LayerExists(std::string name);
        void ShowLayer(std::string name);
        void HideLayer(std::string name);
        void BringToFront(std::string name);
        void SendToBack(std::string name);
        void BringForward(std::string name);
        void SendBackward(std::string name);

        // Input events
        void SendKeyEvent(CefKeyEvent event);
        void SendMouseClickEvent(const CefMouseEvent& event, CefBrowserHost::MouseButtonType type, bool mouseUp, int clickCount);
        void SendMouseMoveEvent(const CefMouseEvent& event, bool mouseLeave);
        void SendMouseWheelEvent(const CefMouseEvent& event, int deltaX, int deltaY);

        // InexorContextProvider
        void InitializeContext() override;
        bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override;
        bool Get(const CefString& name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& retval, CefString& exception) override;
        bool Set(const CefString& name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString& exception) override;
        std::string GetContextName() override { return "layer"; };

    private:
        std::list<CefRefPtr<InexorLayer> > layers;
        std::list<CefRefPtr<InexorLayerProvider> > layer_providers;

        int width;
        int height;

        std::list<CefRefPtr<InexorLayer> >::iterator GetIterator(std::string name);
        void _CreateLayer(std::string name, std::string url);

        // Include the default reference counting implementation.
        IMPLEMENT_REFCOUNTING(InexorLayerManager);

};

}
}
}
