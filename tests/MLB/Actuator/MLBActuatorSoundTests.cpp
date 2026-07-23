#include <MLB/Actuator/MLBActuatorSound.h>
#include <SDL.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

class TemporaryWaveFile {
public:
    explicit TemporaryWaveFile(const std::filesystem::path& path)
        : path_(path) {
        constexpr std::uint32_t sampleRate = 22050;
        constexpr std::uint16_t channels = 1;
        constexpr std::uint16_t bitsPerSample = 16;
        constexpr std::uint32_t sampleCount = sampleRate;
        constexpr std::uint32_t dataSize =
            sampleCount * channels * (bitsPerSample / 8);

        std::vector<unsigned char> bytes;
        bytes.reserve(44 + dataSize);
        appendText(bytes, "RIFF");
        appendU32(bytes, 36 + dataSize);
        appendText(bytes, "WAVE");
        appendText(bytes, "fmt ");
        appendU32(bytes, 16);
        appendU16(bytes, 1);
        appendU16(bytes, channels);
        appendU32(bytes, sampleRate);
        appendU32(bytes, sampleRate * channels * (bitsPerSample / 8));
        appendU16(bytes, channels * (bitsPerSample / 8));
        appendU16(bytes, bitsPerSample);
        appendText(bytes, "data");
        appendU32(bytes, dataSize);
        bytes.resize(44 + dataSize, 0);

        std::ofstream output(path_, std::ios::binary);
        output.write(
            reinterpret_cast<const char*>(bytes.data()),
            static_cast<std::streamsize>(bytes.size()));
    }

    ~TemporaryWaveFile() { std::filesystem::remove(path_); }

    const std::filesystem::path& path() const { return path_; }

private:
    static void appendText(
        std::vector<unsigned char>& bytes, const char* text) {
        for (int i = 0; i < 4; ++i) {
            bytes.push_back(static_cast<unsigned char>(text[i]));
        }
    }

    static void appendU16(
        std::vector<unsigned char>& bytes, std::uint16_t value) {
        bytes.push_back(static_cast<unsigned char>(value & 0xff));
        bytes.push_back(static_cast<unsigned char>((value >> 8) & 0xff));
    }

    static void appendU32(
        std::vector<unsigned char>& bytes, std::uint32_t value) {
        for (int shift = 0; shift < 32; shift += 8) {
            bytes.push_back(
                static_cast<unsigned char>((value >> shift) & 0xff));
        }
    }

    std::filesystem::path path_;
};

TEST(MLBActuatorSoundTest, LoadsAndPlaysResourceBackedWave) {
    char audioDriver[] = "SDL_AUDIODRIVER=dummy";
    ASSERT_EQ(SDL_putenv(audioDriver), 0);
    ASSERT_EQ(SDL_Init(SDL_INIT_AUDIO), 0) << SDL_GetError();
    ASSERT_EQ(
        Mix_OpenAudio(22050, AUDIO_S16SYS, 1, 1024),
        0) << Mix_GetError();

    const TemporaryWaveFile wave("actuator-sound-test.wav");
    MAO parent("parent");
    {
        MLBActuatorSound actuator(
            "sound", parent, wave.path().generic_string());

        EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORSOUND);
        EXPECT_NO_THROW(actuator.actuate());
        EXPECT_GE(Mix_Playing(-1), 1);
        Mix_HaltChannel(-1);
    }

    Mix_CloseAudio();
    SDL_Quit();
}

}  // namespace
