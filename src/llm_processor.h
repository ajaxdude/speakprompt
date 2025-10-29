#ifndef LLM_PROCESSOR_H
#define LLM_PROCESSOR_H

#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>

// Forward declaration for llama.cpp types
struct llama_model;
struct llama_context;

class LLMProcessor {
private:
    llama_model* model;
    llama_context* ctx;
    std::string model_path;
    bool is_initialized;
    std::atomic<bool> is_processing;
    
    // Thread for async processing
    std::thread processing_thread;
    
    // Callback for when processing is complete
    std::function<void(const std::string&)> completion_callback;
    
public:
    LLMProcessor();
    ~LLMProcessor();
    
    bool initialize(const std::string& model_file_path);
    void cleanup();
    
    // Process text to clean it up (async)
    void process_text_async(const std::string& raw_text, 
                           std::function<void(const std::string&)> callback);
    
    // Synchronous version (blocking)
    std::string process_text(const std::string& raw_text);
    
    // Check if processing is currently happening
    bool is_busy() const;
    
    // Cancel current processing
    void cancel_processing();
    
private:
    // Internal processing function
    std::string clean_up_text(const std::string& raw_text);
    
    // Thread function for async processing
    void processing_worker(const std::string& text);
    
    // Create prompt for text cleanup
    std::string create_cleanup_prompt(const std::string& raw_text);
    
    // Generate response from LLM
    std::string generate_response(const std::string& prompt);
};

#endif // LLM_PROCESSOR_H
