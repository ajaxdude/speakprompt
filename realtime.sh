#include "whisper.h"
#include <vector>
#include <string>
#include <iostream>

// Function to load audio and split into chunks
std::vector<std::vector<float>> split_audio(const std::vector<float>& pcm, int chunk_samples, int overlap_samples = 0) {
    std::vector<std::vector<float>> chunks;
    int step = chunk_samples - overlap_samples;

    for (size_t start = 0; start < pcm.size(); start += step) {
        size_t end = std::min(start + chunk_samples, pcm.size());
        chunks.emplace_back(pcm.begin() + start, pcm.begin() + end);
    }

    return chunks;
}

int main() {
    // Load model
    struct whisper_context* ctx = whisper_init_from_file("models/ggml-base.en.bin");

    // Load audio (assume it's a 16-bit mono 16kHz PCM WAV)
    std::vector<float> pcmf32;
    whisper_load_wav_file_f32("input.wav", pcmf32);  // Provided by whisper.cpp

    // Split into 30-second chunks (assuming 16kHz sample rate)
    int chunk_samples = 30 * 16000;
    int overlap_samples = 2 * 16000; // optional 2-second overlap
    auto chunks = split_audio(pcmf32, chunk_samples, overlap_samples);

    // Loop over chunks and transcribe
    std::string full_transcript;
    for (size_t i = 0; i < chunks.size(); ++i) {
        whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
        whisper_full(ctx, params, chunks[i].data(), chunks[i].size());

        // Get text result
        int n_segments = whisper_full_n_segments(ctx);
        for (int j = 0; j < n_segments; ++j) {
            full_transcript += whisper_full_get_segment_text(ctx, j);
        }

        std::cout << "[Chunk " << i + 1 << "/" << chunks.size() << "] Done\n";
    }

    std::cout << "\nFull Transcript:\n" << full_transcript << "\n";

    whisper_free(ctx);
    return 0;
}
