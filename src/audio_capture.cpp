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
        
        std::cout << "Using PulseAudio for audio capture" << std::endl;
        

        
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
    } else if (!wav_file_path.empty()) {
        capture_thread = std::thread(&AudioCapture::capture_wav_loop, this);
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

bool AudioCapture::read_wav_header(std::ifstream& file, int& file_sample_rate, int& file_channels, int& bits_per_sample, int& data_start) {
    char header[44];
    file.read(header, 44);
    
    if (file.gcount() < 44) {
        return false;
    }
    
    // Check RIFF header
    if (strncmp(header, "RIFF", 4) != 0 || strncmp(header + 8, "WAVE", 4) != 0) {
        return false;
    }
    
    // Extract format information
    file_channels = *reinterpret_cast<uint16_t*>(header + 22);
    file_sample_rate = *reinterpret_cast<uint32_t*>(header + 24);
    bits_per_sample = *reinterpret_cast<uint16_t*>(header + 34);
    
    // Find data chunk
    data_start = 12; // Start after RIFF header
    while (data_start < 100) { // Reasonable limit to search
        file.seekg(data_start);
        char chunk_id[5];
        file.read(chunk_id, 4);
        chunk_id[4] = '\0';
        
        if (strcmp(chunk_id, "data") == 0) {
            data_start += 8; // Skip chunk ID and size
            file.seekg(data_start);
            return true;
        }
        
        // Skip to next chunk
        uint32_t chunk_size;
        file.seekg(data_start + 4);
        file.read(reinterpret_cast<char*>(&chunk_size), 4);
        data_start += 8 + chunk_size;
    }
    
    return false;
}

void AudioCapture::capture_wav_loop() {
    std::ifstream wav_file(wav_file_path, std::ios::binary);
    if (!wav_file.is_open()) {
        std::cerr << "Failed to open WAV file: " << wav_file_path << std::endl;
        is_capturing = false;
        return;
    }
    
    int file_sample_rate, file_channels, bits_per_sample, data_start;
    if (!read_wav_header(wav_file, file_sample_rate, file_channels, bits_per_sample, data_start)) {
        std::cerr << "Failed to read WAV file header" << std::endl;
        is_capturing = false;
        return;
    }
    
    std::cout << "Playing WAV file: " << wav_file_path << std::endl;
    std::cout << "Format: " << file_sample_rate << "Hz, " << file_channels << " channels, " << bits_per_sample << " bits" << std::endl;
    
    wav_file.seekg(data_start);
    
    const int buffer_size = 1024;
    std::vector<float> float_buffer(buffer_size);
    
    if (bits_per_sample == 16) {
        std::vector<int16_t> int_buffer(buffer_size);
        
        while (is_capturing.load() && wav_file.read(reinterpret_cast<char*>(int_buffer.data()), buffer_size * sizeof(int16_t))) {
            int samples_read = wav_file.gcount() / sizeof(int16_t);
            
            // Convert to float and potentially resample if needed
            for (int i = 0; i < samples_read; ++i) {
                float_buffer[i] = static_cast<float>(int_buffer[i]) / 32768.0f;
                
                // Handle multi-channel by taking first channel
                if (file_channels > 1) {
                    i += (file_channels - 1);
                    samples_read = std::min(samples_read, buffer_size);
                }
            }
            
            if (audio_data_callback) {
                audio_data_callback(float_buffer);
            }
            
            // Simulate real-time playback with slightly faster processing for better responsiveness
            std::this_thread::sleep_for(std::chrono::milliseconds(buffer_size * 900 / sample_rate));
        }
    } else if (bits_per_sample == 32) {
        std::vector<float> file_buffer(buffer_size);
        
        while (is_capturing.load() && wav_file.read(reinterpret_cast<char*>(file_buffer.data()), buffer_size * sizeof(float))) {
            int samples_read = wav_file.gcount() / sizeof(float);
            
            // Handle multi-channel by taking first channel
            if (file_channels > 1) {
                int write_pos = 0;
                for (int i = 0; i < samples_read; i += file_channels) {
                    float_buffer[write_pos++] = file_buffer[i];
                    if (write_pos >= buffer_size) break;
                }
                samples_read = write_pos;
            } else {
                float_buffer.assign(file_buffer.begin(), file_buffer.begin() + samples_read);
            }
            
            if (audio_data_callback) {
                audio_data_callback(float_buffer);
            }
            
            // Simulate real-time playback with slightly faster processing for better responsiveness
            std::this_thread::sleep_for(std::chrono::milliseconds(buffer_size * 900 / sample_rate));
        }
    }
    
    std::cout << "Finished playing WAV file" << std::endl;
    is_capturing = false;
}
