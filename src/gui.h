#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>
#include <string>
#include <functional>

class GUI {
private:
    GtkWidget* window;
    GtkWidget* main_box;
    GtkWidget* status_label;
    GtkWidget* hotkey_entry;
    GtkWidget* toggle_button;
    GtkWidget* info_label;
    
    std::function<void(const std::string&)> hotkey_changed_callback;
    std::function<void()> toggle_recording_callback;

public:
    GUI();
    ~GUI();
    
    void show();
    void set_recording_state(bool is_recording);
    void set_hotkey(const std::string& hotkey);
    
    void set_hotkey_changed_callback(std::function<void(const std::string&)> callback);
    void set_toggle_recording_callback(std::function<void()> callback);

private:
    static void on_toggle_button_clicked(GtkWidget* widget, gpointer data);
    static void on_hotkey_changed(GtkWidget* widget, gpointer data);
    static gboolean on_window_close(GtkWidget* widget, GdkEvent* event, gpointer data);
    
    void setup_ui();
    void update_status_display();
};

#endif // GUI_H
