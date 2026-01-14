#include "AudioPlayer.hpp"
#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <vector>
#include <string>

class ALSAPlayer : public AudioPlayer
{
    private:
        snd_pcm_t *audioDevice;
        SNDFILE *audioFile;
        SF_INFO fileInfo;
        std::vector<float> buffer;
        snd_pcm_uframes_t bufferSize, periodSize;
        unsigned int alsaSampleRate;

        bool initAudioDevice();
    public:
        ALSAPlayer();
        ~ALSAPlayer() override;
        bool play(const std::string &filename) override;
};
