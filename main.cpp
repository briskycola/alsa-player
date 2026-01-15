#if defined(__linux__)
    #include "ALSAPlayer.hpp"
#elif defined(__APPLE__)
    #include "CoreAudioPlayer.hpp"
#else
    #error "Unsupported Platform"
#endif

#include <iostream>
#include <csignal>
#include <memory>

volatile sig_atomic_t isPlaying = true;

void handleSignal(int signal)
{
    isPlaying = false;
}

int main(int argc, char **argv)
{
    // Check for signals from the OS.
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);

    // Check if the user entered the
    // audio file as a command-line argument.
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <audio-file>\n";
        return 1;
    }

    std::unique_ptr<AudioPlayer> player;

    const std::string filename = argv[1];

#if defined(__linux__)
    player = std::make_unique<ALSAPlayer>();
#elif defined(__APPLE__)
    player = std::make_unique<CoreAudioPlayer>();
#endif

    player->play(filename);

    return 0;
}
