#include "ALSAPlayer.hpp"
#include <iostream>
#include <csignal>
#include <memory>

volatile sig_atomic_t isPlaying = true;

void handleSignal(int signal)
{
    isPlaying = false;
}

ALSAPlayer::ALSAPlayer() :
    audioDevice(nullptr),
    audioFile(nullptr)
{}

ALSAPlayer::~ALSAPlayer()
{
    if (audioDevice) snd_pcm_close(audioDevice);
    audioDevice = nullptr;

    if (audioFile) sf_close(audioFile);
    audioFile = nullptr;
}

bool ALSAPlayer::initAudioDevice()
{
    snd_pcm_hw_params_t *hardwareParameters;

    // Open the ALSA device and set to
    // system default.
    if (snd_pcm_open(&audioDevice, "plug:default", SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        std::cerr << "Could not open ALSA device\n";
        return false;
    }

    // Allocate memory for hardware parameters.
    if (snd_pcm_hw_params_malloc(&hardwareParameters) < 0)
    {
        std::cerr << "Could not allocate memory for hardware parameters\n";
        snd_pcm_close(audioDevice);
        audioDevice = nullptr;
        return false;
    }

    // Initialize hardware parameters to
    // newly allocated memory.
    if (snd_pcm_hw_params_any(audioDevice, hardwareParameters) < 0)
    {
        std::cerr << "Could not initialize hardware parameters\n";
        snd_pcm_hw_params_free(hardwareParameters);
        snd_pcm_close(audioDevice);
        hardwareParameters = nullptr;
        audioDevice = nullptr;
        return false;
    }

    // Set read/write permissions for ALSA.
    if (snd_pcm_hw_params_set_access(audioDevice, hardwareParameters, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
    {
        std::cerr << "Could not set read/write permissions for ALSA\n";
        snd_pcm_hw_params_free(hardwareParameters);
        snd_pcm_close(audioDevice);
        hardwareParameters = nullptr;
        audioDevice = nullptr;
        return false;
    }

    // Set audio format to 32-bit PCM
    // (will still play 16 and 24-bit PCM).
    if (snd_pcm_hw_params_set_format(audioDevice, hardwareParameters, SND_PCM_FORMAT_FLOAT_LE) < 0)
    {
        std::cerr << "Could not set audio format\n";
        snd_pcm_hw_params_free(hardwareParameters);
        snd_pcm_close(audioDevice);
        hardwareParameters = nullptr;
        audioDevice = nullptr;
        return false;
    }

    // Set audio channels to number of
    // channels on the audio file
    if (snd_pcm_hw_params_set_channels(audioDevice, hardwareParameters, fileInfo.channels) < 0)
    {
        std::cerr << "Could not set audio channels\n";
        snd_pcm_hw_params_free(hardwareParameters);
        snd_pcm_close(audioDevice);
        hardwareParameters = nullptr;
        audioDevice = nullptr;
        return false;
    }

    // Set audio sample rate to the
    // sample rate of the file.
    alsaSampleRate = fileInfo.samplerate;
    if (snd_pcm_hw_params_set_rate_near(audioDevice, hardwareParameters, &alsaSampleRate, 0) < 0)
    {
        std::cerr << "Could not set audio sample rate\n";
        snd_pcm_hw_params_free(hardwareParameters);
        snd_pcm_close(audioDevice);
        hardwareParameters = nullptr;
        audioDevice = nullptr;
        return false;
    }

    // Set period size depending on
    // the audio device.
    periodSize = 1024;
    if (snd_pcm_hw_params_set_period_size_near(audioDevice, hardwareParameters, &periodSize, nullptr) < 0)
    {
        std::cerr << "Could not set period size\n";
        snd_pcm_hw_params_free(hardwareParameters);
        snd_pcm_close(audioDevice);
        hardwareParameters = nullptr;
        audioDevice = nullptr;
        return false;
    }

    // Set buffer size depending on
    // the audio device.
    bufferSize = periodSize * 4; // 4 periods
    if (snd_pcm_hw_params_set_buffer_size_near(audioDevice, hardwareParameters, &bufferSize) < 0)
    {
        std::cerr << "Could not set buffer size\n";
        snd_pcm_hw_params_free(hardwareParameters);
        snd_pcm_close(audioDevice);
        hardwareParameters = nullptr;
        audioDevice = nullptr;
        return false;
    }

    // Set new hardware parameters to audio device.
    if (snd_pcm_hw_params(audioDevice, hardwareParameters) < 0)
    {
        std::cerr << "Could not set final hardware parameters\n";
        snd_pcm_hw_params_free(hardwareParameters);
        snd_pcm_close(audioDevice);
        hardwareParameters = nullptr;
        audioDevice = nullptr;
        return false;
    }

    // The hardware parameters have been set in the ALSA device.
    // We no longer need the memory hardwareParameters is pointing
    // to anymore, so we free it.
    snd_pcm_hw_params_free(hardwareParameters);
    hardwareParameters = nullptr;

    // Prepare the audio device for playback
    if (snd_pcm_prepare(audioDevice) < 0)
    {
        std::cerr << "Could not prepare audio device for playback\n";
        snd_pcm_close(audioDevice);
        audioDevice = nullptr;
        return false;
    }
    return true;
}

bool ALSAPlayer::play(const std::string &filename)
{
    sf_count_t framesRead;
    snd_pcm_sframes_t writtenFrames;
    SF_FORMAT_INFO formatInfo;

    // Open the audio file.
    audioFile = sf_open(filename.c_str(), SFM_READ, &fileInfo);
    if (!audioFile)
    {
        std::cerr << "Could not open audio file\n";
        return false;
    }

    // Get metadata
    const char *title = sf_get_string(audioFile, 0x01);
    const char *trackno = sf_get_string(audioFile, 0x09);
    const char *artist = sf_get_string(audioFile, 0x04);
    const char *album = sf_get_string(audioFile, 0x07);
    formatInfo.format = fileInfo.format;
    sf_command(nullptr, SFC_GET_FORMAT_INFO, &formatInfo, sizeof(formatInfo));

    // Print info about audio file.
    std::cout << "Playing: " << filename << "\n";
    if (title != nullptr) std::cout << "Title: " << title << "\n";
    if (trackno != nullptr) std::cout << "Track: " << trackno << "\n";
    if (artist != nullptr) std::cout << "Artist: " << artist << "\n";
    if (album != nullptr) std::cout << "Album: " << album << "\n";
    std::cout << "Channels: " << fileInfo.channels << "\n";
    std::cout << "Format: " << formatInfo.name << "\n";
    std::cout << "Sample Rate: " << fileInfo.samplerate << " hz\n";

    // Initialize audio device.
    if (!initAudioDevice())
    {
        std::cerr << "Could not initialize audio device\n";
        sf_close(audioFile);
        audioFile = nullptr;
        return false;
    }

    // Initialize buffer to store audio data.
    buffer.resize(bufferSize * fileInfo.channels);

    // Continue playing until there are no
    // more frames to read.
    while (isPlaying)
    {
        framesRead = sf_readf_float(audioFile, buffer.data(), bufferSize);

        // No more frames to read.
        if (framesRead <= 0) break;

        // Write frames to the audio device.
        writtenFrames = snd_pcm_writei(audioDevice, buffer.data(), framesRead);

        // Check for errors (XRUN, etc).
        if (writtenFrames < 0)
        {
            if (snd_pcm_recover(audioDevice, writtenFrames, 1) < 0) break;
            continue;
        }

        if (!isPlaying) break;
    }

    if (!isPlaying)
    {
        std::cerr << "Signal handled\n";
        snd_pcm_drop(audioDevice);
    }

    else
    {
        snd_pcm_drain(audioDevice);
    }

    return true;
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

    const std::string filename = argv[1];

    std::unique_ptr<AudioPlayer> player = std::make_unique<ALSAPlayer>();
    player->play(filename);

    return 0;
}
