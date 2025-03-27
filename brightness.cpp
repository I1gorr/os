#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <stdlib.h>

float getCpuUsage() {
    std::ifstream file("/proc/stat");
    std::string line;
    std::getline(file, line);
    
    long user, nice, system, idle;
    sscanf(line.c_str(), "cpu %ld %ld %ld %ld", &user, &nice, &system, &idle);
    
    long total = user + nice + system + idle;
    long usage = user + nice + system;
    
    return (float)usage / total * 100;
}

void setBrightness(float level) {
    std::string command = "brightnessctl set " + std::to_string((int)level) + "%";
    system(command.c_str());
}


int main() {
    while (true) {
        float cpuUsage = getCpuUsage();
        float brightness;

        if (cpuUsage < 20) {
            brightness = 10; // Low CPU, dim the screen
        } else if (cpuUsage >= 20 && cpuUsage <= 80) {
            brightness = 50; // Moderate CPU usage, keep brightness moderate
        } else {
            brightness = 100; // High CPU, full brightness
        }

        setBrightness(brightness);
        std::cout << "CPU Usage: " << cpuUsage << "%, Setting Brightness to: " << brightness << "%" << std::endl;

        sleep(60); // Sleep for 1 minute
    }

    return 0;
}

