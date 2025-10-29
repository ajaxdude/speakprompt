#ifndef TERMINAL_OUTPUT_H
#define TERMINAL_OUTPUT_H

#include <string>
#include <fstream>
#include <mutex>
#include <functional>

class TerminalOutput {
private:
    std::ofstream output_file;
    std::mutex output_mutex;
    std::string output_filename;
    std::string accumulated_text;
    
    std::function<void(const std::string&)> external_callback;
    
public:
    TerminalOutput();
    ~TerminalOutput();
    
    bool initialize();
    void cleanup();
    
    void display_transcription(const std::string& text);
    void show_status(const std::string& status);
    void clear_output();
    
    void set_external_callback(std::function<void(const std::string&)> callback);
    
    std::string get_accumulated_text() const;
    void reset_accumulated_text();
};

#endif // TERMINAL_OUTPUT_H
