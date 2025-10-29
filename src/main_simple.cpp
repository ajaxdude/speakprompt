#include <iostream>
#include <memory>
#include <signal.h>
#include <thread>
#include <chrono>
#include <atomic>
#include "audio_capture.h"
#include "transcription_engine.h"
#include "terminal_output.h"
#include "llm_processor.h"

class SimpleSpeakPrompt {
private:
    std::unique_ptr<AudioCapture> audio_capture;
    std::unique_ptr<TranscriptionEngine> transcription_engine;
    std::unique_ptr<TerminalOutput> terminal_output;
    std::unique_ptr<LLMProcessor> llm_processor;
    bool is_recording = false;

public:
    SimpleSpeakPrompt() {
        // Initialize components
        audio_capture = std::make_unique<AudioCapture>();
        transcription_engine = std::make_unique<TranscriptionEngine>();
        terminal_output = std::make_unique<TerminalOutput>();
        llm_processor = std::make_unique<LLMProcessor>();

        // Set up transcription callback
        transcription_engine->set_transcription_callback([this](const std::string& text) {
            terminal_output->display_transcription(text);
        });
        
        // Set up audio data callback
        audio_capture->set_audio_data_callback([this](const std::vector<float>& audio) {
            transcription_engine->add_audio_data(audio);
        });
        
        // Use the provided WAV file for testing
        audio_capture->set_wav_file_path("/home/papa/ai/stacks/whisper.cpp/samples/jfk.wav");
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

        // Initialize LLM processor - try multiple model paths
        std::vector<std::string> llm_model_paths = {
            "./models/llm/Magistral-Small-2509-Q4_K_M.gguf",
            "../models/llm/Magistral-Small-2509-Q4_K_M.gguf",
            "/home/papa/ai/projects/speakprompt/models/llm/Magistral-Small-2509-Q4_K_M.gguf"
        };
        
        bool llm_initialized = false;
        for (const auto& path : llm_model_paths) {
            if (llm_processor->initialize(path)) {
                llm_initialized = true;
                break;
            }
        }
        
        if (!llm_initialized) {
            std::cout << "Warning: LLM processor not initialized. Text cleanup will be skipped." << std::endl;
            std::cout << "Download Magistral-Small-2509-Q4_K_M.gguf to enable AI text cleanup." << std::endl;
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
            terminal_output->show_status("ON AIR");
        } else {
            std::cerr << "Failed to start audio capture" << std::endl;
        }
    }

    void stop_recording() {
        audio_capture->stop_capture();
        transcription_engine->stop_transcription();
        is_recording = false;
        
        // Add a small delay to ensure final transcription is processed
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        terminal_output->show_status("OFF AIR");
        
        // Get the accumulated transcribed text
        std::string raw_text = terminal_output->get_accumulated_text();
        
        if (!raw_text.empty() && llm_processor && !llm_processor->is_busy()) {
            std::cout << "\n🤖 Processing text with AI..." << std::endl;
            
            // Process text asynchronously with LLM
            llm_processor->process_text_async(raw_text, [this](const std::string& cleaned_text) {
                if (!cleaned_text.empty()) {
                    // Clear the accumulated text and show the cleaned version
                    terminal_output->reset_accumulated_text();
                    terminal_output->display_transcription("✨ Cleaned text:");
                    terminal_output->display_transcription(cleaned_text);
                }
                std::cout << "\n⏹️  Transcription stopped." << std::endl;
                std::cout << "Press Enter to start again, Ctrl+C to quit" << std::endl;
            });
        } else {
            std::cout << "\n⏹️  Transcription stopped." << std::endl;
            std::cout << "Press Enter to start again, Ctrl+C to quit" << std::endl;
        }
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
