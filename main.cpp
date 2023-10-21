#include <iostream>
#include <cstdlib>  // for the system function

int main() {
    int targetSize, vidLength, targetBitrate;
    std::string input;

    std::cout << "Enter target MiB size: ";
    std::cin >> targetSize;

    std::cout << "Enter vid length in seconds (add 2 seconds extra if you want): ";
    std::cin >> vidLength;

    std::cout << "Enter input filename: ";
    std::cin >> input;

    targetBitrate = (targetSize * 8388.608 / vidLength) - 128;

    std::cout << "target bit rate is " << targetBitrate << std::endl;

    std::string command1 = "ffmpeg -y -i \""+ input +"\" -c:v libx264 -b:v "+std::to_string(targetBitrate)+"k -pass 1 -an -f null NUL";
    std::string command2 = "ffmpeg -i \""+ input +"\" -c:v libx264 -b:v "+std::to_string(targetBitrate)+"k -pass 2 -c:a aac -b:a 128k \""+ input +"-compressed.mp4\"";

    // Convert to const char* when using in system()
    const char* cmd1 = command1.c_str();
    const char* cmd2 = command2.c_str();

    int result1 = system(cmd1);
    int result2 = system(cmd2);

    if (result1 == 0 && result2 == 0) {
        std::cout << "FFmpeg commands executed successfully." << std::endl;
    } else {
        std::cerr << "Error running FFmpeg commands." << std::endl;
    }

    return 0;
}
