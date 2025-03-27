#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <json.hpp>  // Include the nlohmann json library

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <cstdlib>

using json = nlohmann::json;

// Function to get process ID from program name
pid_t get_pid_from_program_name(const std::string& program_name) {
    std::string command = "pgrep " + program_name;
    FILE* pipe = popen(command.c_str(), "r");

    if (!pipe) {
        std::cerr << "Error executing pgrep command." << std::endl;
        return -1;
    }

    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    
    fclose(pipe);

    // If we get the PID, convert it to integer
    if (!result.empty()) {
        return std::stoi(result);
    }

    return -1;  // No process found
}

int main() {
    // Read the JSON file containing process names
    std::ifstream input_file("processes.json");

    if (!input_file.is_open()) {
        std::cerr << "Error opening file processes.json" << std::endl;
        return 1;
    }

    // Parse the JSON file
    json j;
    input_file >> j;

    // Check if the JSON is an array
    if (!j.is_array()) {
        std::cerr << "Invalid JSON format. Expected an array of program names." << std::endl;
        return 1;
    }

    // Print the program names
    std::cout << "Program names from processes.json: " << std::endl;
    for (size_t i = 0; i < j.size(); ++i) {
        std::cout << i << ": " << j[i].get<std::string>() << std::endl;
    }

    // Get the index of the program to kill
    int index;
    std::cout << "Enter the index of the program to kill: ";
    std::cin >> index;

    // Check if the index is valid
    if (index < 0 || index >= j.size()) {
        std::cerr << "Invalid index." << std::endl;
        return 1;
    }

    // Get the program name from the JSON array
    std::string program_name = j[index].get<std::string>();

    // Get the PID of the program
    pid_t pid = get_pid_from_program_name(program_name);
    if (pid == -1) {
        std::cerr << "Could not find a running process for " << program_name << std::endl;
        return 1;
    }

    // Kill the process
    if (kill(pid, SIGKILL) == 0) {
        std::cout << "Successfully killed the process: " << program_name << " (PID: " << pid << ")" << std::endl;
    } else {
        std::cerr << "Failed to kill the process: " << program_name << std::endl;
    }

    return 0;
}

