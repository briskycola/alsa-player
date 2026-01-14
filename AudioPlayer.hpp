#include <string>

class AudioPlayer
{
    public:
        virtual ~AudioPlayer() = default;
        virtual bool play(const std::string &filename) = 0;
};
