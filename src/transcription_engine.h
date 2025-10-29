#ifndef TRANSCRIPTION_ENGINE_H
#define TRANSCRIPTION_ENGINE_H

#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>

// Forward declaration for Whisper context
struct whisper_context;

class TranscriptionEngine {
private:
    whisper_context* ctx = nullptr;
    std::thread transcription_thread;
    std::atomic<bool> is_transcribing{false};
    
    // Audio buffer management
    std::queue<std::vector<float>> audio_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    
    // Configuration
    const int sample_rate = 16000;
    const int chunk_samples = 30 * sample_rate; // 30 second chunks
    std::vector<float> audio_buffer;
    
    std::function<void(const std::string&)> transcription_callback;
    
    void transcription_loop();
    void process_audio_chunk(const std::vector<float>& audio);
    std::string transcribe_audio(const std::vector<float>& audio);
    
public:
    TranscriptionEngine();
    ~TranscriptionEngine();
    
    bool initialize();
    bool start_transcription();
    void stop_transcription();
    void cleanup();
    
    void add_audio_data(const std::vector<float>& audio);
    void set_transcription_callback(std::function<void(const std::string&)> callback);
    
    bool is_active() const { return is_transcribing.load(); }
};

#endif // TRANSCRIPTION_ENGINE_H
