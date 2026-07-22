#include <MAO/MAORenderable2DText.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <vector>

namespace {

class SDLTTFGLContext {
public:
    SDLTTFGLContext() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            return;
        }

        surface = SDL_SetVideoMode(16, 16, 32, SDL_OPENGL);
        if (surface != nullptr) {
            ttfInitialized = TTF_Init() == 0;
        }
    }

    ~SDLTTFGLContext() {
        if (ttfInitialized) {
            TTF_Quit();
        }
        SDL_Quit();
    }

    SDL_Surface* surface = nullptr;
    bool ttfInitialized = false;
};

std::filesystem::path findTestFont() {
    const std::vector<std::filesystem::path> candidates = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/Caladea-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf",
    };

    for (const auto& candidate : candidates) {
        if (std::filesystem::is_regular_file(candidate)) {
            return candidate;
        }
    }

    return {};
}

TEST(MAORenderable2DTextTest, LoadsFontAndRegeneratesTextTexture) {
    SDLTTFGLContext context;
    ASSERT_NE(context.surface, nullptr) << SDL_GetError();
    ASSERT_TRUE(context.ttfInitialized) << TTF_GetError();

    const std::filesystem::path fontPath = findTestFont();
    ASSERT_FALSE(fontPath.empty())
        << "No supported system TrueType test font was found";

    MAORenderable2DText text(
        "caption", boost::filesystem::path(fontPath.string()), 16,
        "Hello", 10, 20, TTF_STYLE_NORMAL);

    EXPECT_EQ(text.getType(), T_MAORENDERABLE2DTEXT);
    EXPECT_EQ(text.getText(), "Hello");
    EXPECT_EQ(text.getPtSize(), 16);
    EXPECT_EQ(text.getStyle(), TTF_STYLE_NORMAL);
    EXPECT_EQ(text.getX(), 10);
    EXPECT_EQ(text.getY(), 20);
    EXPECT_GT(text.getWidth(), 0);
    EXPECT_GT(text.getHeight(), 0);

    const int initialWidth = text.getWidth();
    text.setText("Hello, Minerva");

    EXPECT_EQ(text.getText(), "Hello, Minerva");
    EXPECT_GT(text.getWidth(), initialWidth);
    EXPECT_GT(text.getHeight(), 0);
}

TEST(MAORenderable2DTextTest, StoresStyleSizeAndColorProperties) {
    SDLTTFGLContext context;
    ASSERT_NE(context.surface, nullptr) << SDL_GetError();
    ASSERT_TRUE(context.ttfInitialized) << TTF_GetError();

    const std::filesystem::path fontPath = findTestFont();
    ASSERT_FALSE(fontPath.empty())
        << "No supported system TrueType test font was found";

    MAORenderable2DText text(
        "caption", boost::filesystem::path(fontPath.string()), 12,
        "Properties", 0, 0, TTF_STYLE_NORMAL);

    EXPECT_EQ(text.getR(), 250);
    EXPECT_EQ(text.getG(), 50);
    EXPECT_EQ(text.getB(), 50);

    text.setStyle(TTF_STYLE_BOLD);
    text.setPtSize(24);
    // Preserve the legacy public order: red, blue, green.
    text.setColor(10, 20, 30);

    EXPECT_EQ(text.getStyle(), TTF_STYLE_BOLD);
    EXPECT_EQ(text.getPtSize(), 24);
    EXPECT_EQ(text.getR(), 10);
    EXPECT_EQ(text.getG(), 30);
    EXPECT_EQ(text.getB(), 20);
}

}  // namespace
