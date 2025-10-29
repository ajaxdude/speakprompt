#include "audio_capture.h"
#include <iostream>
#include <cstring>
#include <chrono>
#include <random>
#include <thread>
#include <cmath>

AudioCapture::AudioCapture() {
}

AudioCapture::~AudioCapture() {
    cleanup();
}

bool AudioCapture::initialize() {
#ifdef HAVE_PULSE
    // First try to detect the audio server
    std::cout << "Detecting audio system..." << std::endl;
    
    // Check if we're running on PipeWire with PulseAudio compatibility
    pa_sample_spec ss;
    ss.format = sample_format;
    ss.rate = sample_rate;
    ss.channels = channels;

    int error;
    pa_handle = pa_simple_new(nullptr, "SpeakPrompt", PA_STREAM_RECORD, nullptr,
                             "voice capture", &ss, nullptr, nullptr, &error);
    
    if (pa_handle) {
        use_pulse = true;
        
        // Try to detect if we're using PipeWire
        const char* server_env = getenv("PULSE_SERVER");
        if (server_env && strstr(server_env, "pipewire")) {
            std::cout << "Using PipeWire (via PulseAudio compatibility) for audio capture" << std::endl;
        } else {
            // Check by trying to get server info
            pa_mainloop* mainloop = pa_mainloop_new();
            if (mainloop) {
                pa_context* context = pa_context_new(pa_mainloop_get_api(mainloop), "SpeakPrompt-detect");
                if (context) {
                    pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr);
                    // Give it a moment to connect
                    pa_mainloop_iterate(mainloop, 100, nullptr);
                    if (pa_context_get_state(context) == PA_CONTEXT_READY) {
                        pa_operation* op = pa_context_get_server_info(context, 
                            [](pa_context*, const pa_server_info* info, void* userdata) {
                                if (info && info->server_name && strstr(info->server_name, "PipeWire")) {
                                    std::cout << "Detected PipeWire audio server" << std::endl;
                                } else if (info && info->server_name) {
                                    std::cout << "Detected PulseAudio server: " << info->server_name << std::endl;
                                }
                            }, nullptr);
                        if (op) {
                            pa_operation_unref(op);
                        }
                        pa_mainloop_iterate(mainloop, 100, nullptr);
                    }
                    pa_context_disconnect(context);
                    pa_context_unref(context);
                }
                pa_mainloop_free(mainloop);
            }
            std::cout << "Using PulseAudio for audio capture" << std::endl;
        }
        
        // Test if we can actually read from the audio device
        std::vector<int16_t> test_buffer(1024);
        int result = pa_simple_read(pa_handle, test_buffer.data(), 
                                  test_buffer.size() * sizeof(int16_t), &error);
        if (result == 0) {
            std::cout << "Audio capture test successful" << std::endl;
        } else {
            std::cout << "Warning: Audio capture test failed: " << pa_strerror(error) << std::endl;
            std::cout << "This might indicate a permissions or device access issue" << std::endl;
        }
        
        return true;
    } else {
        std::cerr << "PulseAudio initialization failed: " << pa_strerror(error) << std::endl;
        std::cerr << "Error details: ";
        switch (error) {
            case PA_ERR_CONNECTIONREFUSED:
                std::cerr << "Connection refused - audio server may not be running" << std::endl;
                break;
            case PA_ERR_ACCESS:
                std::cerr << "Access denied - check permissions" << std::endl;
                break;
            case PA_ERR_NOTSUPPORTED:
                std::cerr << "Operation not supported" << std::endl;
                break;
            default:
                std::cerr << "Unknown error (code: " << error << ")" << std::endl;
                break;
        }
    }
#endif
    
    // Fallback to demo mode
    use_pulse = false;
    std::cout << "Warning: No audio capture available, running in demo mode" << std::endl;
    std::cout << "Audio will be simulated for testing purposes" << std::endl;
    std::cout << "Possible solutions:" << std::endl;
    std::cout << "1. Install PipeWire or PulseAudio development packages" << std::endl;
    std::cout << "2. Check if audio server is running: pactl info" << std::endl;
    std::cout << "3. Check microphone permissions" << std::endl;
    return true;
}

bool AudioCapture::start_capture() {
    if (is_capturing.load()) {
        return true; // Already capturing
    }
    
#ifdef HAVE_PULSE
    if (!pa_handle && use_pulse) {
        std::cerr << "AudioCapture not initialized" << std::endl;
        return false;
    }
#endif
    
    is_capturing = true;
    
    if (use_pulse) {
        capture_thread = std::thread(&AudioCapture::capture_pulse_loop, this);
    } else {
        capture_thread = std::thread(&AudioCapture::capture_file_loop, this);
    }
    
    return true;
}

void AudioCapture::stop_capture() {
    is_capturing = false;
    
    if (capture_thread.joinable()) {
        capture_thread.join();
    }
}

void AudioCapture::cleanup() {
    stop_capture();
    
#ifdef HAVE_PULSE
    if (pa_handle) {
        pa_simple_free(pa_handle);
        pa_handle = nullptr;
    }
#endif
}

void AudioCapture::set_audio_data_callback(std::function<void(const std::vector<float>&)> callback) {
    audio_data_callback = callback;
}

void AudioCapture::capture_pulse_loop() {
#ifdef HAVE_PULSE
    const int buffer_size = 1024; // samples per buffer
    std::vector<int16_t> buffer(buffer_size);
    std::vector<float> float_buffer(buffer_size);
    
    while (is_capturing.load()) {
        int error;
        int result = pa_simple_read(pa_handle, buffer.data(), 
                                  buffer_size * sizeof(int16_t), &error);
        
        if (result < 0) {
            std::cerr << "pa_simple_read() failed: " << pa_strerror(error) << std::endl;
            break;
        }
        
        // Convert int16_t to float
        for (int i = 0; i < buffer_size; ++i) {
            float_buffer[i] = static_cast<float>(buffer[i]) / 32768.0f;
        }
        
        // Send audio data to callback
        if (audio_data_callback) {
            audio_data_callback(float_buffer);
        }
    }
#endif
}

void AudioCapture::capture_file_loop() {
    // Demo mode - generate simulated audio data
    const int buffer_size = 1024;
    std::vector<float> buffer(buffer_size);
    
    // Simple noise generator for demo
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> noise(0.0f, 0.1f);
    
    static float phase = 0.0f;
    
    while (is_capturing.load()) {
        // Generate simulated audio (sine wave + noise)
        for (int i = 0; i < buffer_size; ++i) {
            float sine = 0.3f * std::sin(2.0f * M_PI * 440.0f * phase / sample_rate); // 440Hz tone
            buffer[i] = sine + noise(gen);
            phase += 1.0f;
        }
        
        // Send audio data to callback
        if (audio_data_callback) {
            audio_data_callback(buffer);
        }
        
        // Simulate real-time audio rate
        std::this_thread::sleep_for(std::chrono::milliseconds(buffer_size * 1000 / sample_rate));
    }
}
