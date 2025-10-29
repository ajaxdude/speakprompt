#include "llm_processor.h"
#include "llama.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>

LLMProcessor::LLMProcessor() : model(nullptr), ctx(nullptr), is_initialized(false), is_processing(false) {
}

LLMProcessor::~LLMProcessor() {
    cleanup();
}

bool LLMProcessor::initialize(const std::string& model_file_path) {
    model_path = model_file_path;
    
    // Check if model file exists
    std::ifstream file(model_path);
    if (!file.good()) {
        std::cerr << "LLM model file not found: " << model_path << std::endl;
        return false;
    }
    file.close();
    
    // Initialize llama.cpp backend parameters
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = -1; // Use all available GPU layers
    model_params.use_mmap = true;
    model_params.use_mlock = false;
    
    // Suppress all llama.cpp output during model loading
    std::cout.setstate(std::ios::failbit);
    std::cerr.setstate(std::ios::failbit);
    
    // Save original stdout/stderr and redirect to /dev/null
    FILE *stdout_orig = stdout, *stderr_orig = stderr;
    FILE *devnull = fopen("/dev/null", "w");
    stdout = devnull;
    stderr = devnull;
    
    model = llama_model_load_from_file(model_path.c_str(), model_params);
    
    // Restore original stdout/stderr and streams
    stdout = stdout_orig;
    stderr = stderr_orig;
    fclose(devnull);
    std::cout.clear(std::ios::failbit);
    std::cerr.clear(std::ios::failbit);
    
    if (!model) {
        std::cerr << "Failed to load LLM model: " << model_path << std::endl;
        return false;
    }
    
    // Initialize context parameters
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 4096; // Context size
    ctx_params.n_batch = 512;
    ctx_params.n_threads = 8;
    ctx_params.n_threads_batch = 8;
    
    // Suppress context initialization output
    std::cout.setstate(std::ios::failbit);
    std::cerr.setstate(std::ios::failbit);
    
    FILE *stdout_orig2 = stdout, *stderr_orig2 = stderr;
    FILE *devnull2 = fopen("/dev/null", "w");
    stdout = devnull2;
    stderr = devnull2;
    
    ctx = llama_init_from_model(model, ctx_params);
    
    stdout = stdout_orig2;
    stderr = stderr_orig2;
    fclose(devnull2);
    std::cout.clear(std::ios::failbit);
    std::cerr.clear(std::ios::failbit);
    if (!ctx) {
        std::cerr << "Failed to create LLM context" << std::endl;
        llama_model_free(model);
        model = nullptr;
        return false;
    }
    
    is_initialized = true;
    
    // Extract model name from the loaded model
    char desc_buf[512];
    int desc_len = llama_model_desc(model, desc_buf, sizeof(desc_buf));
    if (desc_len > 0) {
        model_name = std::string(desc_buf);
        // Extract just the first part (e.g., "magistral-small-2509" from full description)
        size_t space_pos = model_name.find(' ');
        if (space_pos != std::string::npos) {
            model_name = model_name.substr(0, space_pos);
        }
        // Clean up and format nicely
        std::replace(model_name.begin(), model_name.end(), '-', ' ');
        // Capitalize first letter of each word
        bool capitalize = true;
        for (char& c : model_name) {
            if (capitalize && c >= 'a' && c <= 'z') {
                c = c - ('a' - 'A');
                capitalize = false;
            } else if (c == ' ') {
                capitalize = true;
            }
        }
    } else {
        // Fallback: extract from filename
        model_name = model_path.substr(model_path.find_last_of("/\\") + 1);
        size_t dot_pos = model_name.find_last_of('.');
        if (dot_pos != std::string::npos) {
            model_name = model_name.substr(0, dot_pos);
        }
        std::replace(model_name.begin(), model_name.end(), '-', ' ');
    }
    
    std::cout << "LLM processor initialized with model: " << model_name << std::endl;
    return true;
}

void LLMProcessor::cleanup() {
    cancel_processing();
    
    if (ctx) {
        llama_free(ctx);
        ctx = nullptr;
    }
    
    if (model) {
        llama_model_free(model);
        model = nullptr;
    }
    
    is_initialized = false;
}

void LLMProcessor::process_text_async(const std::string& raw_text, 
                                    std::function<void(const std::string&)> callback) {
    if (!is_initialized) {
        std::cerr << "LLM processor not initialized" << std::endl;
        if (callback) {
            callback("");
        }
        return;
    }
    
    if (is_processing.load()) {
        std::cerr << "LLM processor is busy, cannot process new text" << std::endl;
        if (callback) {
            callback("");
        }
        return;
    }
    
    completion_callback = callback;
    is_processing = true;
    
    // Start processing in a separate thread
    processing_thread = std::thread(&LLMProcessor::processing_worker, this, raw_text);
    processing_thread.detach();
}

std::string LLMProcessor::process_text(const std::string& raw_text) {
    if (!is_initialized) {
        std::cerr << "LLM processor not initialized" << std::endl;
        return "";
    }
    
    if (is_processing.load()) {
        std::cerr << "LLM processor is busy" << std::endl;
        return "";
    }
    
    is_processing = true;
    std::string result = clean_up_text(raw_text);
    is_processing = false;
    
    return result;
}

bool LLMProcessor::is_busy() const {
    return is_processing.load();
}

void LLMProcessor::cancel_processing() {
    is_processing = false;
    if (processing_thread.joinable()) {
        processing_thread.join();
    }
}

void LLMProcessor::processing_worker(const std::string& text) {
    std::string result = clean_up_text(text);
    is_processing = false;
    
    if (completion_callback) {
        completion_callback(result);
    }
}

std::string LLMProcessor::clean_up_text(const std::string& raw_text) {
    if (!is_initialized || raw_text.empty()) {
        return raw_text;
    }
    
    std::string prompt = create_cleanup_prompt(raw_text);
    return generate_response(prompt);
}

std::string LLMProcessor::create_cleanup_prompt(const std::string& raw_text) {
    std::stringstream prompt;
    prompt << "You are a text cleaning assistant. Your task is to improve spoken transcriptions by:\n";
    prompt << "1. Removing repetitions and filler words (um, uh, like, you know, etc.)\n";
    prompt << "2. Fixing grammar and sentence structure\n";
    prompt << "3. Making the text more concise and coherent\n";
    prompt << "4. Preserving the original meaning and key points\n";
    prompt << "5. Organizing rambling thoughts into clear, structured sentences\n\n";
    prompt << "Please clean up the following transcribed text:\n\n";
    prompt << raw_text << "\n\n";
    prompt << "Provide only the cleaned-up text without any explanations or commentary.";
    
    return prompt.str();
}

std::string LLMProcessor::generate_response(const std::string& prompt) {
    if (!ctx || !model) {
        return "";
    }
    
    // Get vocab from model
    const llama_vocab * vocab = llama_model_get_vocab(model);
    
    // Tokenize the prompt
    std::vector<llama_token> tokens;
    tokens.resize(prompt.length() * 2);  // Estimate
    
    int n_tokens = llama_tokenize(
        vocab,
        prompt.c_str(),
        prompt.length(),
        tokens.data(),
        tokens.size(),
        true,  // add_bos
        false  // special
    );
    
    if (n_tokens < 0) {
        std::cerr << "Failed to tokenize prompt" << std::endl;
        return "";
    }
    
    tokens.resize(n_tokens);
    
    // Initialize sampler with temperature and other parameters
    auto sparams = llama_sampler_chain_default_params();
    sparams.no_perf = false;
    llama_sampler * smpl = llama_sampler_chain_init(sparams);
    
    // Add sampling strategies
    llama_sampler_chain_add(smpl, llama_sampler_init_top_k(40));
    llama_sampler_chain_add(smpl, llama_sampler_init_top_p(0.8f, 1));
    llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.3f));
    llama_sampler_chain_add(smpl, llama_sampler_init_dist(1234));  // seed
    
    // Clear the KV cache (not needed with new API - fresh context)
    // llama_kv_cache_clear(ctx);
    
    // Process the prompt
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());
    
    // Decode prompt
    if (llama_decode(ctx, batch) != 0) {
        std::cerr << "Failed to decode prompt" << std::endl;
        llama_sampler_free(smpl);
        return "";
    }
    
    // Generate response
    std::string response;
    const int max_tokens = 1024;  // Maximum tokens to generate
    int n_decode = 0;
    
    while (n_decode < max_tokens) {
        // Sample next token
        llama_token new_token = llama_sampler_sample(smpl, ctx, -1);
        
        // Check for end of sequence
        if (new_token == llama_vocab_eos(vocab)) {
            break;
        }
        
        // Convert token to string
        char buf[256];
        int n = llama_token_to_piece(vocab, new_token, buf, sizeof(buf), 0, true);
        if (n > 0) {
            response.append(buf, n);
        }
        
        // Prepare next batch
        batch = llama_batch_get_one(&new_token, 1);
        
        // Decode
        if (llama_decode(ctx, batch) != 0) {
            std::cerr << "Failed to decode during generation" << std::endl;
            break;
        }
        
        n_decode++;
    }
    
    // Cleanup
    llama_sampler_free(smpl);
    
    // Clean up the response
    // Remove any leading/trailing whitespace
    response.erase(0, response.find_first_not_of(" \t\n\r"));
    response.erase(response.find_last_not_of(" \t\n\r") + 1);
    
    return response;
}

std::string LLMProcessor::get_model_name() const {
    return model_name;
}
