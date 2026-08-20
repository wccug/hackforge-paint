#include "win32_colorpicker.h"
#include <iostream>
// Author Mackenzie Stewart
// Color Picker for Windows and Linux
// ============================================================================
// WINDOWS PLATFORM NATIVE CALL
// ============================================================================
#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <commdlg.h>

#pragma comment(lib, "comdlg32.lib")

SDL_Color OpenNativeColorPicker(SDL_Window* window, SDL_Color currentColor) {
    CHOOSECOLOR cc;
    static COLORREF acrCustClr[16];

    HWND hwndOwner = (HWND)SDL_GetPointerProperty(
        SDL_GetWindowProperties(window), 
        SDL_PROP_WINDOW_WIN32_HWND_POINTER, 
        NULL
    );

    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwndOwner; 
    cc.lpCustColors = (LPDWORD)acrCustClr;
    cc.rgbResult = RGB(currentColor.r, currentColor.g, currentColor.b);
    cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ANYCOLOR;

    if (ChooseColor(&cc) == TRUE) {
        return SDL_Color{ GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult), 255 };
    }
    return currentColor;
}

// ============================================================================
// LINUX PLATFORM NATIVE CALL (XDG Desktop Portal over D-Bus)
// ============================================================================
#elif defined(__linux__)
#include <dbus/dbus.h>
#include <string>

SDL_Color OpenNativeColorPicker(SDL_Window* window, SDL_Color currentColor) {
    DBusError err;
    dbus_error_init(&err);

    DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (dbus_error_is_set(&err)) {
        std::cerr << "[Portal] D-Bus connection failed: " << err.message << std::endl;
        dbus_error_free(&err);
        return currentColor;
    }

    DBusMessage* msg = dbus_message_new_method_call(
        "org.freedesktop.portal.Desktop",      
        "/org/freedesktop/portal/desktop",     
        "org.freedesktop.portal.Screenshot",   
        "SelectColor"                          
    );

    if (!msg) return currentColor;

    DBusMessageIter iter;
    dbus_message_iter_init_append(msg, &iter);

    const char* parent_window = "";
    dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &parent_window);

    DBusMessageIter dict_iter;
    dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &dict_iter);
    dbus_message_iter_close_container(&iter, &dict_iter);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err)) {
        std::cerr << "[Portal] Interaction canceled or failed: " << err.message << std::endl;
        dbus_error_free(&err);
        return currentColor;
    }

    SDL_Color selectedColor = currentColor;
    DBusMessageIter reply_iter;
    if (dbus_message_iter_init(reply, &reply_iter)) {
        DBusMessageIter results_dict;
        dbus_message_iter_recurse(&reply_iter, &results_dict);

        while (dbus_message_iter_get_arg_type(&results_dict) == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter entry;
            dbus_message_iter_recurse(&results_dict, &entry);

            const char* key;
            dbus_message_iter_get_basic(&entry, &key);

            if (std::string(key) == "color") {
                dbus_message_iter_next(&entry);
                
                DBusMessageIter variant;
                dbus_message_iter_recurse(&entry, &variant);

                DBusMessageIter array_iter;
                dbus_message_iter_recurse(&variant, &array_iter);

                double r = 0.0, g = 0.0, b = 0.0;
                dbus_message_iter_get_basic(&array_iter, &r);
                dbus_message_iter_next(&array_iter);
                dbus_message_iter_get_basic(&array_iter, &g);
                dbus_message_iter_next(&array_iter);
                dbus_message_iter_get_basic(&array_iter, &b);

                selectedColor.r = static_cast<Uint8>(r * 255.0);
                selectedColor.g = static_cast<Uint8>(g * 255.0);
                selectedColor.b = static_cast<Uint8>(b * 255.0);
                selectedColor.a = 255;
                break;
            }
            dbus_message_iter_next(&results_dict);
        }
    }

    dbus_message_unref(reply);
    return selectedColor;
}

#else
SDL_Color OpenNativeColorPicker(SDL_Window* window, SDL_Color currentColor) {
    return currentColor;
}
#endif
