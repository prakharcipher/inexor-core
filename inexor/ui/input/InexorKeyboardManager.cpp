﻿#include "inexor/ui/input/InexorKeyboardManager.hpp"
#include <stdint.h>                          // for uint32_t
#include <codecvt>                           // for codecvt_utf8_utf16
#include <locale>                            // for wstring_convert
#include <memory>                            // for __shared_ptr
#include <string>                            // for u16string, string

#include "SDL_keyboard.h"                    // for SDL_Keysym
#include "include/base/cef_logging.h"        // for COMPACT_GOOGLE_LOG_DCHECK
#include "include/base/cef_ref_counted.h"    // for scoped_refptr
#include "include/wrapper/cef_helpers.h"     // for CEF_REQUIRE_RENDERER_THREAD
#include "inexor/io/Logging.hpp"             // for Log, Logger, log_manager
#include "inexor/io/input/SDL2Keyboard.hpp"  // for convertSDLtoJSKeyCode

class CefBrowser;

namespace inexor {
namespace ui {
namespace input {

bool InexorKeyboardManager::Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
{
    CEF_REQUIRE_RENDERER_THREAD();
    return false;
}

bool InexorKeyboardManager::Get(const CefString& name, const CefRefPtr<CefV8Value> object, CefRefPtr<CefV8Value>& return_value, CefString& exception)
{
    CEF_REQUIRE_RENDERER_THREAD();
    return false;
}

bool InexorKeyboardManager::Set(const CefString& name, const CefRefPtr<CefV8Value> object, const CefRefPtr<CefV8Value> value, CefString& exception)
{
    CEF_REQUIRE_RENDERER_THREAD();
    return false;
}

#if _MSC_VER // visual studio 2015/17 bug see https://social.msdn.microsoft.com/Forums/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
std::u16string utf8_to_utf16(std::string utf8_string)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
    return reinterpret_cast<const char16_t *>(convert.from_bytes(utf8_string).data());
}
#else
std::u16string utf8_to_utf16(std::string utf8_string)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    return convert.from_bytes(utf8_string);
}

#endif

void InexorKeyboardManager::SendKeyEvent(SDL_Event &e)
{
    if(e.type == SDL_KEYDOWN)
    {
        CefKeyEvent cef_event;
        cef_event.modifiers = getKeyboardModifiers(e.key.keysym.mod);
        uint32_t javascript_keycode = convertSDLtoJSKeyCode(e.key.keysym.sym);
        cef_event.windows_key_code = javascript_keycode;
        cef_event.character = javascript_keycode;

        cef_event.type = KEYEVENT_RAWKEYDOWN; // or shall we send keydown?
        layer_manager->SendKeyEvent(cef_event);

        //cef_event.type = KEYEVENT_CHAR;
        //layer_manager->SendKeyEvent(cef_event);
        return;
    }
    if(e.type == SDL_KEYUP)
    {
        CefKeyEvent cef_event;
        cef_event.modifiers = getKeyboardModifiers(e.key.keysym.mod);
        cef_event.windows_key_code = convertSDLtoJSKeyCode(e.key.keysym.sym);

        cef_event.type = KEYEVENT_KEYUP;
        layer_manager->SendKeyEvent(cef_event);
        return;
    }
    if(e.type == SDL_TEXTINPUT)
    {
        CefKeyEvent cef_event;
//        cef_string_utf8_to_utf16(e.text.text, SDL_TEXTINPUTEVENT_TEXT_SIZE, )
        std::u16string decodedstr(utf8_to_utf16(e.text.text)); //SDL gives us utf8 and CEF has char16. we could also just assign it though (so the highest bits would be nulled).

        for(char16_t cha : decodedstr)
        {
            cef_event.character = cha;
            cef_event.windows_key_code = cha;
            cef_event.type = KEYEVENT_CHAR;
            layer_manager->SendKeyEvent(cef_event);
        }
    }
}

bool InexorKeyboardManager::OnPreKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& key_event, CefEventHandle os_event, bool* is_keyboard_shortcut) {
    CEF_REQUIRE_UI_THREAD();
    Log.ui->debug("InexorCefKeyboardManager::OnPreKeyEvent: key_event.type: {0} native_key_code: {1} windows_key_code: {2} is_system_key: {3}",
                                 key_event.type, key_event.native_key_code, key_event.windows_key_code, key_event.is_system_key);
    return false;
}

bool InexorKeyboardManager::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent& key_event, CefEventHandle os_event) {
    CEF_REQUIRE_UI_THREAD();
    Log.ui->debug("InexorCefKeyboardManager::OnKeyEvent: key_event.type: {0} native_key_code: {1} windows_key_code: {2} is_system_key: {3}",
                                 key_event.type, key_event.native_key_code, key_event.windows_key_code, key_event.is_system_key);
    return false;
}

}
}
}
