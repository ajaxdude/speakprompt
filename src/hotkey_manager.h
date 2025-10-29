#ifndef HOTKEY_MANAGER_H
#define HOTKEY_MANAGER_H

#include <string>
#include <functional>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <thread>
#include <atomic>

class HotkeyManager {
private:
    Display* display = nullptr;
    Window root_window;
    std::thread hotkey_thread;
    std::atomic<bool> is_listening{false};
    
    // Hotkey configuration
    KeyCode keycode = 0;
    unsigned int modifiers = 0;
    std::string current_hotkey = "Ctrl+Shift+Space";
    
    std::function<void()> hotkey_pressed_callback;
    
    void hotkey_loop();
    bool parse_hotkey(const std::string& hotkey);
    bool grab_hotkey();
    void ungrab_hotkey();
    
public:
    HotkeyManager();
    ~HotkeyManager();
    
    bool initialize();
    bool start_listening();
    void stop_listening();
    void cleanup();
    
    bool set_hotkey(const std::string& hotkey);
    std::string get_hotkey() const { return current_hotkey; }
    
    void set_hotkey_pressed_callback(std::function<void()> callback);
    
    bool is_active() const { return is_listening.load(); }
};

#endif // HOTKEY_MANAGER_H
