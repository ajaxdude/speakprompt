#include "gui.h"

GUI::GUI() : window(nullptr) {
    setup_ui();
}

GUI::~GUI() {
    if (window) {
        gtk_widget_destroy(window);
    }
}

void GUI::setup_ui() {
    // Create main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "SpeakPrompt");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    
    // Center the window
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    
    // Create main container
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(main_box), 20);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    
    // Title label
    GtkWidget* title_label = gtk_label_new("SpeakPrompt");
    PangoFontDescription* font_desc = pango_font_description_from_string("Sans Bold 16");
    gtk_widget_modify_font(title_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_box_pack_start(GTK_BOX(main_box), title_label, FALSE, FALSE, 0);
    
    // Status frame
    GtkWidget* status_frame = gtk_frame_new("Status");
    gtk_box_pack_start(GTK_BOX(main_box), status_frame, FALSE, FALSE, 5);
    
    GtkWidget* status_alignment = gtk_alignment_new(0.5, 0.5, 0, 0);
    gtk_container_add(GTK_CONTAINER(status_frame), status_alignment);
    
    status_label = gtk_label_new("Ready");
    gtk_container_add(GTK_CONTAINER(status_alignment), status_label);
    
    // Hotkey frame
    GtkWidget* hotkey_frame = gtk_frame_new("Global Hotkey");
    gtk_box_pack_start(GTK_BOX(main_box), hotkey_frame, FALSE, FALSE, 5);
    
    GtkWidget* hotkey_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(hotkey_box), 5);
    gtk_container_add(GTK_CONTAINER(hotkey_frame), hotkey_box);
    
    hotkey_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(hotkey_entry), "Ctrl+Shift+Space");
    gtk_entry_set_max_length(GTK_ENTRY(hotkey_entry), 20);
    gtk_box_pack_start(GTK_BOX(hotkey_box), hotkey_entry, TRUE, TRUE, 0);
    
    GtkWidget* hotkey_label = gtk_label_new("(e.g., Ctrl+Shift+Space)");
    gtk_widget_modify_font(hotkey_label, pango_font_description_from_string("Sans 8"));
    gtk_box_pack_start(GTK_BOX(hotkey_box), hotkey_label, FALSE, FALSE, 0);
    
    // Toggle button
    toggle_button = gtk_button_new_with_label("Start Recording");
    gtk_widget_set_size_request(toggle_button, 200, 40);
    gtk_box_pack_start(GTK_BOX(main_box), toggle_button, FALSE, FALSE, 10);
    
    // Info label
    info_label = gtk_label_new("Press the hotkey or button to start/pause transcription");
    gtk_label_set_justify(GTK_LABEL(info_label), GTK_JUSTIFY_CENTER);
    gtk_widget_modify_font(info_label, pango_font_description_from_string("Sans 9"));
    gtk_box_pack_start(GTK_BOX(main_box), info_label, FALSE, FALSE, 5);
    
    // Connect signals
    g_signal_connect(window, "delete-event", G_CALLBACK(on_window_close), this);
    g_signal_connect(toggle_button, "clicked", G_CALLBACK(on_toggle_button_clicked), this);
    g_signal_connect(hotkey_entry, "changed", G_CALLBACK(on_hotkey_changed), this);
    
    // Show all widgets
    gtk_widget_show_all(window);
}

void GUI::show() {
    if (window) {
        gtk_widget_show(window);
    }
}

void GUI::set_recording_state(bool is_recording) {
    if (toggle_button) {
        gtk_button_set_label(GTK_BUTTON(toggle_button), 
                           is_recording ? "Stop Recording" : "Start Recording");
        
        // Change button color based on state
        GdkColor color;
        if (is_recording) {
            color.red = 0xffff;
            color.green = 0x6666;
            color.blue = 0x6666;
        } else {
            color.red = 0x6666;
            color.green = 0xffff;
            color.blue = 0x6666;
        }
        gtk_widget_modify_bg(toggle_button, GTK_STATE_NORMAL, &color);
    }
    
    if (status_label) {
        gtk_label_set_text(GTK_LABEL(status_label), 
                          is_recording ? "Recording..." : "Ready");
    }
    
    update_status_display();
}

void GUI::set_hotkey(const std::string& hotkey) {
    if (hotkey_entry) {
        gtk_entry_set_text(GTK_ENTRY(hotkey_entry), hotkey.c_str());
    }
}

void GUI::set_hotkey_changed_callback(std::function<void(const std::string&)> callback) {
    hotkey_changed_callback = callback;
}

void GUI::set_toggle_recording_callback(std::function<void()> callback) {
    toggle_recording_callback = callback;
}

void GUI::update_status_display() {
    // Force GTK to update the display
    if (main_box) {
        gtk_widget_queue_draw(main_box);
    }
}

void GUI::on_toggle_button_clicked(GtkWidget* widget, gpointer data) {
    GUI* gui = static_cast<GUI*>(data);
    if (gui->toggle_recording_callback) {
        gui->toggle_recording_callback();
    }
}

void GUI::on_hotkey_changed(GtkWidget* widget, gpointer data) {
    GUI* gui = static_cast<GUI*>(data);
    if (gui->hotkey_changed_callback && gui->hotkey_entry) {
        const char* hotkey = gtk_entry_get_text(GTK_ENTRY(gui->hotkey_entry));
        gui->hotkey_changed_callback(std::string(hotkey));
    }
}

gboolean GUI::on_window_close(GtkWidget* widget, GdkEvent* event, gpointer data) {
    gtk_main_quit();
    return FALSE;
}
