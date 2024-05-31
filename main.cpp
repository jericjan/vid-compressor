#include <iostream> // cin & cout
#include <cstdlib>  // system
#include <cmath>    // std::ceil

#include <cstdio>    // std::remove
#include <memory>    //std::unique_ptr
#include <stdexcept> //std::runtime_error
#include <string>    //std::string
#include <array>     //std::array

#include <algorithm> //std::remove()
#include <vector>    //std::vector

std::string input;
int targetBitrate;
bool includeAudio;

double exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
    return std::stod(result);
}

void deleteFiles(std::vector<const char *> filenames)
{
    int success = true;
    for (const char *filename : filenames)
    {
        if (std::remove(filename) == 0)
        {
            std::cout << filename << " deleted successfully." << std::endl;
        }
        else
        {
            std::cerr << "Error deleting the file." << std::endl;
            success = false;
        }
    }

    if (success == true)
    {
        std::cout << "All files deleted successfully.";
    }
    else
    {
        std::cout << "Not all files deleted...";
    }
}

void two_pass()
{
    std::string command1 = "ffmpeg -y -i \"" + input + "\" -c:v libx264 -b:v " + std::to_string(targetBitrate) + "k -pass 1 -an -f null NUL -loglevel error";

    std::string audioFlag = includeAudio ? "-b:a 128k" : "-an";
    std::string command2 = "ffmpeg -y -i \"" + input + "\" -c:v libx264 -b:v " + std::to_string(targetBitrate) + "k -pass 2 -c:a aac " + audioFlag + " \"" + input + "-compressed.mp4\" -loglevel error";

    // Convert to const char* when using in system()
    const char *cmd1 = command1.c_str();
    const char *cmd2 = command2.c_str();

    std::cout << "Running first pass...";
    int result1 = system(cmd1);
    std::cout << "Success!\nRunning second pass...";
    int result2 = system(cmd2);
    std::cout << "Success!\n";

    if (result1 == 0 && result2 == 0)
    {
        std::cout << "FFmpeg commands executed successfully." << std::endl;
    }
    else
    {
        std::cerr << "Error running FFmpeg commands." << std::endl;
    }

    deleteFiles({"ffmpeg2pass-0.log", "ffmpeg2pass-0.log.mbtree"});
}

void one_pass()
{
    std::string audioFlag = includeAudio ? "-b:a 128k" : "-an";
    std::string command1 = "ffmpeg -y -i \"" + input + "\" -c:v libx264 -b:v " + std::to_string(targetBitrate) + "k " + audioFlag + " \"" + input + "-compressed.mp4\" -loglevel error";
    const char *cmd1 = command1.c_str();
    int result1 = system(cmd1);

    if (result1 == 0)
    {
        std::cout << "FFmpeg commands executed successfully." << std::endl;
    }
    else
    {
        std::cerr << "Error running FFmpeg commands." << std::endl;
    }
}

void selectMode()
{

    int passMode;
    std::cout << "Select a mode:\n[1] One-pass\n[2] Two-pass\n";
    std::cin >> passMode;
    switch (passMode)
    {
    case 1:
        one_pass();
        break;
    case 2:
        two_pass();
        break;
    default:
        std::cout << "That's not in the options!";
        selectMode();
    }
}

void setAudio()
{
    char choice;
    std::cout << "Include audio? (y/n) ";
    std::cin >> choice;
    switch (choice)
    {
    case 'y':
        includeAudio = true;
        break;
    case 'n':
        includeAudio = false;
        break;
    default:
        std::cout << "That's not in the options!";
        setAudio();
    }
}

int main()
{
    int targetSize, vidLength;

    std::cout << "Enter target MiB size: ";
    std::cin >> targetSize;

    std::cout << "Enter input filename: ";
    std::cin.ignore();
    std::getline(std::cin, input);

    std::string lenCmd0 = ("ffprobe -v error -show_entries format=duration -of default=noprint_wrappers=1:nokey=1 \"" + input + "\"").c_str();
    const char *lenCmd = lenCmd0.c_str();

    double vidLenLong = exec(lenCmd);
    vidLength = std::ceil(vidLenLong);
    std::cout << "Vid length is " << std::to_string(vidLength) << " seconds\n";

    setAudio();

    targetBitrate = (targetSize * 8388.608 / vidLength);

    if (includeAudio == true)
    {
        targetBitrate -= 128;
    }

    std::cout << "target bit rate is " << targetBitrate << "kBit/s" << std::endl;
    std::cout << "This will be converted into x264 MP4\n";

    selectMode();

    return 0;
}
