#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

// Function to read CPU usage from /proc/stat
float get_cpu_usage() {
    std::ifstream stat_file("/proc/stat");
    std::string line;
    std::getline(stat_file, line);
    std::istringstream ss(line);
    
    std::string cpu;
    long user, nice, system, idle, iowait, irq, softirq, steal;

    ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    // Calculate total CPU time
    long total_time = user + nice + system + idle + iowait + irq + softirq + steal;
    long idle_time = idle + iowait;

    static long prev_total_time = 0;
    static long prev_idle_time = 0;

    // Calculate the difference in total time and idle time
    long total_diff = total_time - prev_total_time;
    long idle_diff = idle_time - prev_idle_time;

    // Update the previous values for next calculation
    prev_total_time = total_time;
    prev_idle_time = idle_time;

    // Calculate CPU usage as a percentage
    return (1.0 - (float)idle_diff / total_diff) * 100.0;
}

// Function to set CPU governor (single core or all cores)
void set_governor(const std::string& governor, bool all_cores = true) {
    std::string governor_path = "/sys/devices/system/cpu/cpu";
    std::string governor_file = "/cpufreq/scaling_governor";

    // Set the governor for each CPU core
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (all_cores) {
        for (int i = 0; i < num_cores; ++i) {
            std::ofstream governor_file_stream(governor_path + std::to_string(i) + governor_file);
            if (governor_file_stream.is_open()) {
                governor_file_stream << governor;
                governor_file_stream.close();
            } else {
                std::cerr << "Error: Unable to set governor for cpu" << i << std::endl;
            }
        }
    } else {
        // Set only the first core
        std::ofstream governor_file_stream(governor_path + "0" + governor_file);
        if (governor_file_stream.is_open()) {
            governor_file_stream << governor;
            governor_file_stream.close();
        } else {
            std::cerr << "Error: Unable to set governor for cpu0" << std::endl;
        }
    }
}

// Function to set CPU frequency
void set_frequency(int freq_khz) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    for (int i = 0; i < num_cores; ++i) {
        std::ofstream freq_file_stream("/sys/devices/system/cpu/cpu" + std::to_string(i) + "/cpufreq/scaling_setspeed");
        if (freq_file_stream.is_open()) {
            freq_file_stream << freq_khz;
            freq_file_stream.close();
        } else {
            std::cerr << "Error: Unable to set frequency for cpu" << i << std::endl;
        }
    }
}

int main() {
    while (true) {
        // Get the CPU usage
        float cpu_usage = get_cpu_usage();
        std::cout << "Current CPU Usage: " << cpu_usage << "%" << std::endl;

        if (cpu_usage > 50) {
            std::cout << "High CPU usage detected. Setting all cores to performance mode." << std::endl;
            set_governor("performance", true);  // All cores to performance
        }
        else if (cpu_usage >= 30 && cpu_usage <= 50) {
            std::cout << "Moderate CPU usage detected. Boosting one core." << std::endl;
            set_governor("performance", false);  // Only boost first core to performance
        }
        else if (cpu_usage >= 20 && cpu_usage < 30) {
            std::cout << "Low CPU usage detected. Setting to balanced mode." << std::endl;
            set_governor("ondemand", true);  // Balanced mode (ondemand) for all cores
        }
        else {
            std::cout << "Very low CPU usage detected. Enabling battery saver mode." << std::endl;
            set_governor("powersave", true);  // Battery saver mode (powersave)
        }

        // Sleep for 30 seconds before checking again
        sleep(30);
    }

    return 0;
}

