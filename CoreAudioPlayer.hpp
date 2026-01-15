#include "AudioPlayer.hpp"
#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sndfile.h>
#include <string>

class CoreAudioPlayer : public AudioPlayer
{
    private:
        AudioQueueRef queue;
        AudioQueueBufferRef buffers[4];
        SNDFILE *audioFile;
        SF_INFO fileInfo;
        AudioStreamBasicDescription asbd;
        UInt32 framesPerBuffer;
        UInt32 bytesPerBuffer;
        CFRunLoopRef runLoop;

        bool initAudioDevice();
        void stopAudioDevice();
        static bool checkOSStatus(OSStatus status, const char *message);
        static void audioQueueCallback(void *context, AudioQueueRef queue, AudioQueueBufferRef buffer);
        static void queueRunningCallback(void *context, AudioQueueRef queue, AudioQueuePropertyID propertyID);
        void fillBuffer(AudioQueueBufferRef buffer);
    public:
        CoreAudioPlayer();
        ~CoreAudioPlayer() override;
        bool play(const std::string &filename) override;
};
