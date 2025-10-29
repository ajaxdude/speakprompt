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
    
    std::cout << "SpeakPrompt - Terminal Output Active" << std::endl;
    std::cout << "Transcription will be written to: " << output_filename << std::endl;
    std::cout << "Use 'tail -f " << output_filename << "' to follow along" << std::endl;
    std::cout << std::endl;
    
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
    if (text.empty()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(output_mutex);
    
    // Accumulate text
    accumulated_text += text;
    
    // Output to console with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::cout << "\033[1;32m[" << std::put_time(std::localtime(&time_t), "%H:%M:%S");
    std::cout << "." << std::setfill('0') << std::setw(3) << ms.count() << "]\033[0m ";
    std::cout << "\033[1m" << text << "\033[0m" << std::endl;
    std::cout.flush();
    
    // Write to file
    if (output_file.is_open()) {
        output_file << text << " ";
        output_file.flush();
    }
    
    // Call external callback if set
    if (external_callback) {
        external_callback(text);
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
