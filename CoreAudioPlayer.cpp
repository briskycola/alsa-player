#include "CoreAudioPlayer.hpp"
#include <iostream>
#include <csignal>

extern volatile sig_atomic_t isPlaying;

CoreAudioPlayer::CoreAudioPlayer() :
    queue(nullptr),
    audioFile(nullptr),
    framesPerBuffer(1024),
    bytesPerBuffer(0),
    runLoop(nullptr)
{
    buffers[0] = buffers[1] = buffers[2] = buffers[3] = nullptr;
}

CoreAudioPlayer::~CoreAudioPlayer()
{
    stopAudioDevice();

    if (runLoop)
    {
        CFRelease(runLoop);
    }
    runLoop = nullptr;

    if (audioFile)
    {
        sf_close(audioFile);
    }
    audioFile = nullptr;
}

bool CoreAudioPlayer::checkOSStatus(OSStatus status, const char *message)
{
    if (status == noErr) return true;
    else std::cerr << "OSStatus: " << status << "\n";
    return false;
}

bool CoreAudioPlayer::initAudioDevice()
{
    // Set fields for float playback.
    asbd.mSampleRate = static_cast<Float64>(fileInfo.samplerate);
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    asbd.mChannelsPerFrame = static_cast<UInt32>(fileInfo.channels);
    asbd.mFramesPerPacket = 1;
    asbd.mBitsPerChannel = 32;
    asbd.mBytesPerFrame = asbd.mChannelsPerFrame * sizeof(float);
    asbd.mBytesPerPacket = asbd.mBytesPerFrame * asbd.mFramesPerPacket;

    bytesPerBuffer = framesPerBuffer * asbd.mBytesPerFrame;

    OSStatus status = AudioQueueNewOutput(&asbd, CoreAudioPlayer::audioQueueCallback, this, nullptr, nullptr, 0, &queue);
    if (!checkOSStatus(status, "AudioQueueNewOutput failed")) return false;

    status = AudioQueueAddPropertyListener(queue, kAudioQueueProperty_IsRunning, CoreAudioPlayer::queueRunningCallback, this);
    if (!checkOSStatus(status, "AudioQueueAddPropertyListener failed")) return false;

    // Allocate buffers
    for (int i = 0; i < 4; i++)
    {
        status = AudioQueueAllocateBuffer(queue, bytesPerBuffer, &buffers[i]);
        if (!checkOSStatus(status, "AudioQueueAllocateBuffer failed")) return false;

        fillBuffer(buffers[i]);

        status = AudioQueueEnqueueBuffer(queue, buffers[i], 0, nullptr);
        if (!checkOSStatus(status, "AudioQueueEnqueueBuffer failed")) return false;
    }

    status = AudioQueueStart(queue, nullptr);
    if (!checkOSStatus(status, "AudioQueueStart failed")) return false;
    return true;
}

void CoreAudioPlayer::stopAudioDevice()
{
    if (!queue) return;

    // If signal happened, stop immediately.
    AudioQueueStop(queue, true);

    AudioQueueDispose(queue, true);
    queue = nullptr;

    buffers[0] = buffers[1] = buffers[2] = buffers[3] = nullptr;
}

void CoreAudioPlayer::fillBuffer(AudioQueueBufferRef buffer)
{
    if (!audioFile || !buffer)
    {
        buffer->mAudioDataByteSize = 0;
        return;
    }

    float *audioData = static_cast<float*>(buffer->mAudioData);
    sf_count_t framesToRead = static_cast<sf_count_t>(framesPerBuffer);

    // Read frames.
    sf_count_t framesRead = sf_readf_float(audioFile, audioData, framesToRead);

    if (framesRead <= 0)
    {
        buffer->mAudioDataByteSize = 0;
        return;
    }

    buffer->mAudioDataByteSize = static_cast<UInt32>(framesRead * fileInfo.channels * sizeof(float));
}

void CoreAudioPlayer::audioQueueCallback(void *context, AudioQueueRef queue, AudioQueueBufferRef buffer)
{
    CoreAudioPlayer *coreAudioPlayer = static_cast<CoreAudioPlayer*>(context);
    if (!coreAudioPlayer) return;

    // If there's a signal, stop immediately.
    if (!isPlaying)
    {
        buffer->mAudioDataByteSize = 0;
        AudioQueueStop(coreAudioPlayer->queue, true);
        return;
    }

    coreAudioPlayer->fillBuffer(buffer);

    // If at EOF, stop gracefully.
    if (buffer->mAudioDataByteSize == 0)
    {
        AudioQueueStop(coreAudioPlayer->queue, false);
        return;
    }

    AudioQueueEnqueueBuffer(coreAudioPlayer->queue, buffer, 0, nullptr);
}

void CoreAudioPlayer::queueRunningCallback(void *context, AudioQueueRef queue, AudioQueuePropertyID propertyID)
{
    CoreAudioPlayer *coreAudioPlayer = static_cast<CoreAudioPlayer*>(context);
    if (!coreAudioPlayer) return;

    UInt32 isRunning = 0;
    UInt32 size = sizeof(isRunning);

    AudioQueueGetProperty(queue, kAudioQueueProperty_IsRunning, &isRunning, &size);
    if (!isRunning && coreAudioPlayer->runLoop) CFRunLoopStop(coreAudioPlayer->runLoop);
}

bool CoreAudioPlayer::play(const std::string &filename)
{
    // Open the audio file.
    audioFile = sf_open(filename.c_str(), SFM_READ, &fileInfo);
    if (!audioFile)
    {
        std::cerr << "Could not open audio file\n";
        return false;
    }

    runLoop = CFRunLoopGetCurrent();
    CFRetain(runLoop);

    // Get metadata.
    const char* title  = sf_get_string(audioFile, SF_STR_TITLE);
    const char* track  = sf_get_string(audioFile, SF_STR_TRACKNUMBER);
    const char* artist = sf_get_string(audioFile, SF_STR_ARTIST);
    const char* album  = sf_get_string(audioFile, SF_STR_ALBUM);

    SF_FORMAT_INFO formatInfo;
    formatInfo.format = fileInfo.format;
    sf_command(nullptr, SFC_GET_FORMAT_INFO, &formatInfo, sizeof(formatInfo));

    std::cout << "Playing: " << filename << "\n";
    if (title)  std::cout << "Title: " << title << "\n";
    if (track)  std::cout << "Track: " << track << "\n";
    if (artist) std::cout << "Artist: " << artist << "\n";
    if (album)  std::cout << "Album: " << album << "\n";
    std::cout << "Channels: " << fileInfo.channels << "\n";
    std::cout << "Format: " << formatInfo.name << "\n";
    std::cout << "Sample Rate: " << fileInfo.samplerate << " hz\n";

    // Initialize CoreAudio device
    if (!initAudioDevice())
    {
        std::cerr << "Could not initialize CoreAudio device\n";
        sf_close(audioFile);
        audioFile = nullptr;
        return false;
    }

    // Block here until AudioQueue stops
    CFRunLoopRun();

    if (!isPlaying)
    {
        std::cerr << "Signal handled\n";
    }
    return true;
}
