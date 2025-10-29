#include "hotkey_manager.h"
#include <iostream>
#include <sstream>
#include <algorithm>

HotkeyManager::HotkeyManager() : display(nullptr) {
}

HotkeyManager::~HotkeyManager() {
    cleanup();
}

bool HotkeyManager::initialize() {
    display = XOpenDisplay(nullptr);
    if (!display) {
        std::cout << "Warning: Cannot open X11 display, hotkey functionality disabled" << std::endl;
        std::cout << "Running in console mode - use Enter to toggle recording" << std::endl;
        return true; // Continue without hotkey support
    }
    
    root_window = DefaultRootWindow(display);
    
    // Parse default hotkey
    if (!parse_hotkey(current_hotkey)) {
        std::cerr << "Failed to parse default hotkey" << std::endl;
        return false;
    }
    
    return true;
}

bool HotkeyManager::start_listening() {
    if (is_listening.load()) {
        return true; // Already listening
    }
    
    if (!display) {
        // No X11 display available, just return true (console mode)
        return true;
    }
    
    if (!grab_hotkey()) {
        std::cerr << "Failed to grab hotkey" << std::endl;
        return false;
    }
    
    is_listening = true;
    hotkey_thread = std::thread(&HotkeyManager::hotkey_loop, this);
    
    return true;
}

void HotkeyManager::stop_listening() {
    is_listening = false;
    
    if (hotkey_thread.joinable()) {
        hotkey_thread.join();
    }
    
    ungrab_hotkey();
}

void HotkeyManager::cleanup() {
    stop_listening();
    
    if (display) {
        XCloseDisplay(display);
        display = nullptr;
    }
}

bool HotkeyManager::set_hotkey(const std::string& hotkey) {
    bool was_active = is_listening.load();
    
    // Stop listening temporarily
    if (was_active) {
        stop_listening();
    }
    
    // Release old hotkey
    ungrab_hotkey();
    
    // Parse and grab new hotkey
    if (parse_hotkey(hotkey)) {
        if (grab_hotkey()) {
            current_hotkey = hotkey;
            
            // Restart listening if it was active
            if (was_active) {
                start_listening();
            }
            
            return true;
        }
    }
    
    // If we failed, try to restore the previous hotkey
    if (was_active) {
        parse_hotkey(current_hotkey);
        grab_hotkey();
        start_listening();
    }
    
    return false;
}

void HotkeyManager::set_hotkey_pressed_callback(std::function<void()> callback) {
    hotkey_pressed_callback = callback;
}

void HotkeyManager::hotkey_loop() {
    XEvent event;
    
    while (is_listening.load()) {
        if (XPending(display)) {
            XNextEvent(display, &event);
            
            if (event.type == KeyPress) {
                XKeyEvent key_event = event.xkey;
                
                // Check if this matches our hotkey
                if (key_event.keycode == keycode) {
                    unsigned int state = key_event.state & (ShiftMask | ControlMask | Mod1Mask | Mod4Mask);
                    if (state == modifiers) {
                        if (hotkey_pressed_callback) {
                            hotkey_pressed_callback();
                        }
                    }
                }
            }
        } else {
            // Sleep briefly to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

bool HotkeyManager::parse_hotkey(const std::string& hotkey) {
    std::string lower_hotkey = hotkey;
    std::transform(lower_hotkey.begin(), lower_hotkey.end(), lower_hotkey.begin(), ::tolower);
    
    modifiers = 0;
    
    // Parse modifiers
    if (lower_hotkey.find("ctrl+") != std::string::npos) {
        modifiers |= ControlMask;
    }
    if (lower_hotkey.find("shift+") != std::string::npos) {
        modifiers |= ShiftMask;
    }
    if (lower_hotkey.find("alt+") != std::string::npos) {
        modifiers |= Mod1Mask;
    }
    if (lower_hotkey.find("super+") != std::string::npos || lower_hotkey.find("win+") != std::string::npos) {
        modifiers |= Mod4Mask;
    }
    
    // Extract key part
    size_t plus_pos = lower_hotkey.find_last_of('+');
    std::string key_part = (plus_pos != std::string::npos) ? 
                           lower_hotkey.substr(plus_pos + 1) : lower_hotkey;
    
    // Convert key name to keysym
    KeySym keysym = NoSymbol;
    if (key_part == "space") {
        keysym = XK_space;
    } else if (key_part == "return" || key_part == "enter") {
        keysym = XK_Return;
    } else if (key_part == "escape" || key_part == "esc") {
        keysym = XK_Escape;
    } else if (key_part == "tab") {
        keysym = XK_Tab;
    } else if (key_part == "backspace") {
        keysym = XK_BackSpace;
    } else if (key_part == "delete") {
        keysym = XK_Delete;
    } else if (key_part == "home") {
        keysym = XK_Home;
    } else if (key_part == "end") {
        keysym = XK_End;
    } else if (key_part == "pageup") {
        keysym = XK_Page_Up;
    } else if (key_part == "pagedown") {
        keysym = XK_Page_Down;
    } else if (key_part == "up") {
        keysym = XK_Up;
    } else if (key_part == "down") {
        keysym = XK_Down;
    } else if (key_part == "left") {
        keysym = XK_Left;
    } else if (key_part == "right") {
        keysym = XK_Right;
    } else if (key_part == "f1") keysym = XK_F1;
    else if (key_part == "f2") keysym = XK_F2;
    else if (key_part == "f3") keysym = XK_F3;
    else if (key_part == "f4") keysym = XK_F4;
    else if (key_part == "f5") keysym = XK_F5;
    else if (key_part == "f6") keysym = XK_F6;
    else if (key_part == "f7") keysym = XK_F7;
    else if (key_part == "f8") keysym = XK_F8;
    else if (key_part == "f9") keysym = XK_F9;
    else if (key_part == "f10") keysym = XK_F10;
    else if (key_part == "f11") keysym = XK_F11;
    else if (key_part == "f12") keysym = XK_F12;
    else if (key_part.length() == 1) {
        // Single character key
        keysym = XStringToKeysym(key_part.c_str());
    }
    
    if (keysym == NoSymbol) {
        std::cerr << "Unknown key: " << key_part << std::endl;
        return false;
    }
    
    keycode = XKeysymToKeycode(display, keysym);
    if (keycode == 0) {
        std::cerr << "No keycode for keysym: " << key_part << std::endl;
        return false;
    }
    
    return true;
}

bool HotkeyManager::grab_hotkey() {
    if (!display || keycode == 0) {
        return false;
    }
    
    // Grab the hotkey
    int result = XGrabKey(display, keycode, modifiers, root_window, 
                         True, GrabModeAsync, GrabModeAsync);
    
    if (result != GrabSuccess) {
        std::cerr << "Failed to grab hotkey" << std::endl;
        return false;
    }
    
    return true;
}

void HotkeyManager::ungrab_hotkey() {
    if (display && keycode != 0) {
        XUngrabKey(display, keycode, modifiers, root_window);
        XFlush(display);
    }
}
