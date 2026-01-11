#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <vector>
#include <string>

class alsaPlayer
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
        alsaPlayer();
        ~alsaPlayer();
        bool play(const std::string &filename);
};
