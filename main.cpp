#include <iostream>
#include <cstdlib>  // for the system function
#include <cmath>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include <algorithm>
#include <vector>

double exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return std::stod(result);
}

void deleteFiles(std::vector<const char*> filenames){
    int success = true;
    for (const char* filename: filenames) {
        if (std::remove(filename) == 0) {
            std::cout << filename << " deleted successfully." << std::endl;
        } else {
            std::cerr << "Error deleting the file." << std::endl;
            success = false;
        }
    }

    if (success = true) {
        std::cout << "All files deleted successfully.";
    } else {
        std::cout << "Not all files deleted...";
    }

}

int main() {
    int targetSize, vidLength, targetBitrate;
    std::string input;

    std::cout << "Enter target MiB size: ";
    std::cin >> targetSize;

    std::cout << "Enter input filename: ";
    std::cin >> input;

    std::string lenCmd0 = ("ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \""+input+"\"").c_str();
    const char* lenCmd = lenCmd0.c_str();

    double vidLenLong = exec(lenCmd);
    vidLength = std::ceil(vidLenLong);
    std::cout << "Vid length is " << std::to_string(vidLength) << " seconds\n";

    targetBitrate = (targetSize * 8388.608 / vidLength) - 128;

    std::cout << "target bit rate is " << targetBitrate << "kBit/s" << std::endl;

    std::string command1 = "ffmpeg -y -i \""+ input +"\" -c:v libx264 -b:v "+std::to_string(targetBitrate)+"k -pass 1 -an -f null NUL -loglevel error";
    std::string command2 = "ffmpeg -y -i \""+ input +"\" -c:v libx264 -b:v "+std::to_string(targetBitrate)+"k -pass 2 -c:a aac -b:a 128k \""+ input +"-compressed.mp4\" -loglevel error";

    // Convert to const char* when using in system()
    const char* cmd1 = command1.c_str();
    const char* cmd2 = command2.c_str();

    std::cout << "Running first pass...";
    int result1 = system(cmd1);
    std::cout << "Success!\nRunning second pass...";
    int result2 = system(cmd2);
    std::cout << "Success!\n";

    if (result1 == 0 && result2 == 0) {
        std::cout << "FFmpeg commands executed successfully." << std::endl;
    } else {
        std::cerr << "Error running FFmpeg commands." << std::endl;
    }
    
    deleteFiles({"ffmpeg2pass-0.log", "ffmpeg2pass-0.log.mbtree"});

    return 0;
}
