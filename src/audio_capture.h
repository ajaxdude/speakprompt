#ifndef AUDIO_CAPTURE_H
#define AUDIO_CAPTURE_H

#ifdef HAVE_PULSE
#include <pulse/simple.h>
#include <pulse/error.h>
#endif
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <fstream>

class AudioCapture {
private:
#ifdef HAVE_PULSE
    pa_simple* pa_handle = nullptr;
#endif
    std::thread capture_thread;
    std::atomic<bool> is_capturing{false};
    std::atomic<bool> use_pulse{false};
    
    // Audio parameters
    const int sample_rate = 16000;
    const int channels = 1;
#ifdef HAVE_PULSE
    const pa_sample_format_t sample_format = PA_SAMPLE_S16LE;
#endif
    
    std::function<void(const std::vector<float>&)> audio_data_callback;
    
    void capture_loop();
    void capture_pulse_loop();
    void capture_file_loop();

public:
    AudioCapture();
    ~AudioCapture();
    
    bool initialize();
    bool start_capture();
    void stop_capture();
    void cleanup();
    
    void set_audio_data_callback(std::function<void(const std::vector<float>&)> callback);
    
    bool is_active() const { return is_capturing.load(); }
    
    // Audio configuration
    int get_sample_rate() const { return sample_rate; }
    int get_channels() const { return channels; }
};

#endif // AUDIO_CAPTURE_H
