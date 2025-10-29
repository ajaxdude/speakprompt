#include "transcription_engine.h"
#include "whisper.h"
#include <iostream>
#include <fstream>
#include <algorithm>

TranscriptionEngine::TranscriptionEngine() : ctx(nullptr) {
}

TranscriptionEngine::~TranscriptionEngine() {
    cleanup();
}

bool TranscriptionEngine::initialize() {
    // Try to find the model file in several locations, prioritizing faster models
    std::vector<std::string> model_paths = {
        "./models/ggml-large-v3-turbo.bin",
        "../models/ggml-large-v3-turbo.bin",
        "/usr/share/speakprompt/models/ggml-large-v3-turbo.bin",
        "./models/ggml-base.en.bin",
        "../models/ggml-base.en.bin",
        "/usr/share/speakprompt/models/ggml-base.en.bin",
        "ggml-base.en.bin"
    };
    
    std::string model_path;
    for (const auto& path : model_paths) {
        std::ifstream file(path);
        if (file.good()) {
            model_path = path;
            break;
        }
    }
    
    if (model_path.empty()) {
        std::cerr << "Could not find whisper model file. Please download ggml-large-v3-turbo.bin or ggml-base.en.bin" << std::endl;
        return false;
    }
    
    ctx = whisper_init_from_file(model_path.c_str());
    if (!ctx) {
        std::cerr << "Failed to initialize whisper context" << std::endl;
        return false;
    }
    
    // Extract just the model filename from the path
    std::string model_name = model_path.substr(model_path.find_last_of("/\\") + 1);
    // Remove .bin extension and add brackets
    model_name = model_name.substr(0, model_name.find_last_of('.'));
    std::cout << "Loaded [" << model_name << "] for real-time transcription" << std::endl;
    return true;
}

bool TranscriptionEngine::start_transcription() {
    if (is_transcribing.load()) {
        return true; // Already transcribing
    }
    
    if (!ctx) {
        std::cerr << "TranscriptionEngine not initialized" << std::endl;
        return false;
    }
    
    audio_buffer.clear();
    is_transcribing = true;
    transcription_thread = std::thread(&TranscriptionEngine::transcription_loop, this);
    
    return true;
}

void TranscriptionEngine::stop_transcription() {
    is_transcribing = false;
    queue_cv.notify_all();
    
    if (transcription_thread.joinable()) {
        transcription_thread.join();
    }
    
    // Process any remaining audio
    if (!audio_buffer.empty()) {
        std::string final_text = transcribe_audio(audio_buffer);
        if (!final_text.empty() && transcription_callback) {
            transcription_callback(final_text);
        }
        audio_buffer.clear();
    }
}

void TranscriptionEngine::cleanup() {
    stop_transcription();
    
    if (ctx) {
        whisper_free(ctx);
        ctx = nullptr;
    }
}

void TranscriptionEngine::add_audio_data(const std::vector<float>& audio) {
    if (!is_transcribing.load()) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        audio_queue.push(audio);
    }
    queue_cv.notify_one();
}

void TranscriptionEngine::set_transcription_callback(std::function<void(const std::string&)> callback) {
    transcription_callback = callback;
}

void TranscriptionEngine::transcription_loop() {
    while (is_transcribing.load()) {
        std::unique_lock<std::mutex> lock(queue_mutex);
        
        // Wait for short time or new audio data (real-time processing)
        queue_cv.wait_for(lock, std::chrono::milliseconds(100), [this] { 
            return !audio_queue.empty() || !is_transcribing.load(); 
        });
        
        if (!is_transcribing.load()) {
            break;
        }
        
        // Collect available audio data
        while (!audio_queue.empty()) {
            auto audio_chunk = audio_queue.front();
            audio_queue.pop();
            audio_buffer.insert(audio_buffer.end(), audio_chunk.begin(), audio_chunk.end());
        }
        lock.unlock();
        
        // Process in chunks if we have enough data for real-time streaming
        while (audio_buffer.size() >= chunk_samples) {
            std::vector<float> chunk(audio_buffer.begin(), audio_buffer.begin() + chunk_samples);
            process_audio_chunk(chunk);
            
            // Remove processed chunk, but keep overlap for continuity
            audio_buffer.erase(audio_buffer.begin(), audio_buffer.begin() + (chunk_samples - overlap_samples));
        }
    }
}

void TranscriptionEngine::process_audio_chunk(const std::vector<float>& audio) {
    std::string text = transcribe_audio(audio);
    if (!text.empty() && transcription_callback) {
        transcription_callback(text);
    }
}

std::string TranscriptionEngine::transcribe_audio(const std::vector<float>& audio) {
    if (!ctx || audio.empty()) {
        return "";
    }
    
    // Set up whisper parameters for real-time streaming
    whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
    params.print_realtime = false;
    params.print_progress = false;
    params.print_timestamps = false;
    params.print_special = false;
    params.translate = false;
    params.language = "en";
    params.n_threads = 8;  // Use more threads for faster processing
    params.offset_ms = 0;
    params.duration_ms = (audio.size() * 1000) / sample_rate;
    
    // Real-time optimizations
    params.max_tokens = 32;  // Limit output tokens for faster processing
    params.audio_ctx = 0;    // Use full context for better accuracy
    
    // Run inference
    int result = whisper_full(ctx, params, audio.data(), audio.size());
    if (result != 0) {
        std::cerr << "Failed to process audio" << std::endl;
        return "";
    }
    
    // Extract text
    std::string text;
    int n_segments = whisper_full_n_segments(ctx);
    for (int i = 0; i < n_segments; ++i) {
        const char* segment_text = whisper_full_get_segment_text(ctx, i);
        if (segment_text) {
            text += segment_text;
        }
    }
    
    // Clean up whitespace
    text.erase(0, text.find_first_not_of(" \t\n\r"));
    text.erase(text.find_last_not_of(" \t\n\r") + 1);
    
    return text;
}
