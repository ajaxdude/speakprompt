#include <iostream>
#include <memory>
#include <signal.h>
#include <string>
#ifndef NO_GUI
#include <gtk/gtk.h>
#include "gui.h"
#endif
#include "audio_capture.h"
#include "transcription_engine.h"
#include "hotkey_manager.h"
#include "terminal_output.h"

class SpeakPromptApp {
private:
#ifndef NO_GUI
    std::unique_ptr<GUI> gui;
#endif
    std::unique_ptr<AudioCapture> audio_capture;
    std::unique_ptr<TranscriptionEngine> transcription_engine;
    std::unique_ptr<HotkeyManager> hotkey_manager;
    std::unique_ptr<TerminalOutput> terminal_output;
    bool is_recording = false;
    bool use_gui = true;

public:
    SpeakPromptApp() {
#ifdef NO_GUI
        use_gui = false;
#endif
        
        // Initialize components
        audio_capture = std::make_unique<AudioCapture>();
        transcription_engine = std::make_unique<TranscriptionEngine>();
        hotkey_manager = std::make_unique<HotkeyManager>();
        terminal_output = std::make_unique<TerminalOutput>();

        if (use_gui) {
#ifndef NO_GUI
            gui = std::make_unique<GUI>();
            
            // Set up GUI callbacks
            gui->set_hotkey_changed_callback([this](const std::string& hotkey) {
                hotkey_manager->set_hotkey(hotkey);
            });

            gui->set_toggle_recording_callback([this]() {
                toggle_recording();
            });
#endif
        }

        // Set up hotkey and transcription callbacks
        hotkey_manager->set_hotkey_pressed_callback([this]() {
            toggle_recording();
        });

        transcription_engine->set_transcription_callback([this](const std::string& text) {
            terminal_output->display_transcription(text);
        });
    }

    void run() {
        if (!audio_capture->initialize()) {
            std::cerr << "Failed to initialize audio capture" << std::endl;
            return;
        }

        if (!transcription_engine->initialize()) {
            std::cerr << "Failed to initialize transcription engine" << std::endl;
            return;
        }

        if (!hotkey_manager->initialize()) {
            std::cerr << "Failed to initialize hotkey manager" << std::endl;
            return;
        }

        if (!terminal_output->initialize()) {
            std::cerr << "Failed to initialize terminal output" << std::endl;
            return;
        }

        if (use_gui) {
#ifndef NO_GUI
            gui->show();
            gtk_main();
#endif
        } else {
            run_console_mode();
        }
    }

private:
    void run_console_mode() {
        std::cout << "SpeakPrompt - Console Mode" << std::endl;
        std::cout << "Press Enter to start/stop transcription (Ctrl+C to quit)" << std::endl;
        std::cout << "Hotkey: " << hotkey_manager->get_hotkey() << std::endl;
        std::cout << std::endl;

        // Set up signal handler for Ctrl+C
        signal(SIGINT, [](int) {
            std::cout << "\nExiting..." << std::endl;
            exit(0);
        });

        std::string input;
        while (std::getline(std::cin, input)) {
            toggle_recording();
        }
    }

    void toggle_recording() {
        if (is_recording) {
            stop_recording();
        } else {
            start_recording();
        }
    }

    void start_recording() {
        std::cout << "\nStarting transcription..." << std::endl;
        
        if (audio_capture->start_capture()) {
            transcription_engine->start_transcription();
            is_recording = true;
            
#ifndef NO_GUI
            if (use_gui) {
                gui->set_recording_state(true);
            }
#endif
            
            terminal_output->show_status("Recording started - Speak now");
        } else {
            std::cerr << "Failed to start audio capture" << std::endl;
        }
    }

    void stop_recording() {
        std::cout << "\nStopping transcription..." << std::endl;
        
        audio_capture->stop_capture();
        transcription_engine->stop_transcription();
        is_recording = false;
        
#ifndef NO_GUI
        if (use_gui) {
            gui->set_recording_state(false);
        }
#endif
        
        terminal_output->show_status("Recording stopped");
    }
};

int main(int argc, char* argv[]) {
#ifndef NO_GUI
    gtk_init(&argc, &argv);
#endif

    try {
        SpeakPromptApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
