#include <iostream>
#include <memory>
#include <signal.h>
#include <thread>
#include <chrono>
#include "audio_capture.h"
#include "transcription_engine.h"
#include "terminal_output.h"

class SimpleSpeakPrompt {
private:
    std::unique_ptr<AudioCapture> audio_capture;
    std::unique_ptr<TranscriptionEngine> transcription_engine;
    std::unique_ptr<TerminalOutput> terminal_output;
    bool is_recording = false;

public:
    SimpleSpeakPrompt() {
        // Initialize components
        audio_capture = std::make_unique<AudioCapture>();
        transcription_engine = std::make_unique<TranscriptionEngine>();
        terminal_output = std::make_unique<TerminalOutput>();

        // Set up transcription callback
        transcription_engine->set_transcription_callback([this](const std::string& text) {
            terminal_output->display_transcription(text);
        });
    }

    bool initialize() {
        if (!audio_capture->initialize()) {
            std::cerr << "Failed to initialize audio capture" << std::endl;
            return false;
        }

        if (!transcription_engine->initialize()) {
            std::cerr << "Failed to initialize transcription engine" << std::endl;
            return false;
        }

        if (!terminal_output->initialize()) {
            std::cerr << "Failed to initialize terminal output" << std::endl;
            return false;
        }

        return true;
    }

    void run() {
        std::cout << "\n=== SpeakPrompt - Simple Speech-to-Text ===" << std::endl;
        std::cout << "Press Enter to start/stop transcription" << std::endl;
        std::cout << "Press Ctrl+C to quit" << std::endl;
        std::cout << "========================================\n" << std::endl;

        // Set up signal handler for Ctrl+C
        signal(SIGINT, [](int) {
            std::cout << "\n\nExiting SpeakPrompt..." << std::endl;
            exit(0);
        });

        std::string input;
        while (std::getline(std::cin, input)) {
            toggle_recording();
        }
    }

private:
    void toggle_recording() {
        if (is_recording) {
            stop_recording();
        } else {
            start_recording();
        }
    }

    void start_recording() {
        std::cout << "\n🎙️  Starting transcription... (Speak now)" << std::endl;
        
        if (audio_capture->start_capture()) {
            transcription_engine->start_transcription();
            is_recording = true;
            terminal_output->show_status("Recording started");
        } else {
            std::cerr << "Failed to start audio capture" << std::endl;
        }
    }

    void stop_recording() {
        std::cout << "\n⏹️  Stopping transcription..." << std::endl;
        
        audio_capture->stop_capture();
        transcription_engine->stop_transcription();
        is_recording = false;
        terminal_output->show_status("Recording stopped");
        
        std::cout << "Press Enter to start again, Ctrl+C to quit" << std::endl;
    }
};

int main() {
    try {
        SimpleSpeakPrompt app;
        
        if (!app.initialize()) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }
        
        app.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Application error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
