#include "terminal_output.h"
#include <iostream>
#include <chrono>
#include <iomanip>

TerminalOutput::TerminalOutput() {
}

TerminalOutput::~TerminalOutput() {
    cleanup();
}

bool TerminalOutput::initialize() {
    // Create a temporary file for output
    output_filename = "/tmp/speakprompt_output.txt";
    output_file.open(output_filename, std::ios::app);
    
    if (!output_file.is_open()) {
        std::cerr << "Failed to open output file: " << output_filename << std::endl;
        return false;
    }
    
    // Clear the file initially
    output_file.close();
    output_file.open(output_filename, std::ios::trunc);
    
    
    
    return true;
}

void TerminalOutput::cleanup() {
    std::lock_guard<std::mutex> lock(output_mutex);
    
    if (output_file.is_open()) {
        output_file.close();
    }
    
    // Remove temporary file
    if (!output_filename.empty()) {
        std::remove(output_filename.c_str());
    }
}

void TerminalOutput::display_transcription(const std::string& text) {
    if (text.empty() || text == "." || text == "[BLANK_AUDIO]") {
        return;
    }
    
    std::lock_guard<std::mutex> lock(output_mutex);
    
    // Clean up the text - remove extra whitespace and make it continuous
    std::string clean_text = text;
    // Remove leading/trailing whitespace
    clean_text.erase(0, clean_text.find_first_not_of(" \t\n\r"));
    clean_text.erase(clean_text.find_last_not_of(" \t\n\r") + 1);
    
    // Skip if text is empty after cleaning
    if (clean_text.empty()) {
        return;
    }
    
    // Accumulate text with space for continuous paragraph
    if (!accumulated_text.empty() && accumulated_text.back() != ' ') {
        accumulated_text += " ";
    }
    accumulated_text += clean_text;
    
    // Output to console without timestamp, as continuous text
    std::cout << "\033[1m" << clean_text << " \033[0m" << std::flush;
    
    // Write to file as continuous text
    if (output_file.is_open()) {
        output_file << clean_text << " ";
        output_file.flush();
    }
    
    // Call external callback if set
    if (external_callback) {
        external_callback(clean_text);
    }
}

void TerminalOutput::show_status(const std::string& status) {
    std::lock_guard<std::mutex> lock(output_mutex);
    
    // Output status to console
    std::cout << "\033[1;34m[STATUS]\033[0m " << status << std::endl;
    std::cout.flush();
    
    // Write to file
    if (output_file.is_open()) {
        output_file << std::endl << "[STATUS] " << status << std::endl;
        output_file.flush();
    }
}

void TerminalOutput::clear_output() {
    std::lock_guard<std::mutex> lock(output_mutex);
    
    accumulated_text.clear();
    
    // Clear console output
    std::cout << "\033[2J\033[H" << std::endl;
    
    // Clear file
    if (output_file.is_open()) {
        output_file.close();
        output_file.open(output_filename, std::ios::trunc);
    }
    
    std::cout << "SpeakPrompt - Output cleared" << std::endl;
}

void TerminalOutput::set_external_callback(std::function<void(const std::string&)> callback) {
    external_callback = callback;
}

std::string TerminalOutput::get_accumulated_text() const {
    return accumulated_text;
}

void TerminalOutput::reset_accumulated_text() {
    std::lock_guard<std::mutex> lock(output_mutex);
    accumulated_text.clear();
}
