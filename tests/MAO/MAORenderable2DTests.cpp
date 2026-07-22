#include <MAO/MAORenderable2D.h>
#include <MAO/MAORenderable2DImage.h>
#include <gtest/gtest.h>

#include <filesystem>

namespace {

class TestRenderable2D : public MAORenderable2D {
public:
    using MAORenderable2D::MAORenderable2D;

    void generateTexFromSDLSurface() override {}
};

class SDLGLContext {
public:
    SDLGLContext() {
        if (SDL_Init(SDL_INIT_VIDEO) == 0) {
            surface = SDL_SetVideoMode(16, 16, 32, SDL_OPENGL);
        }
    }

    ~SDLGLContext() { SDL_Quit(); }

    SDL_Surface* surface = nullptr;
};

TEST(MAORenderable2DTest, StoresAndUpdatesScreenProperties) {
    TestRenderable2D renderable("overlay", 10, 20, 30, 40);

    EXPECT_EQ(renderable.getType(), T_MAORENDERABLE2D);
    EXPECT_TRUE(renderable.isVisible());
    EXPECT_EQ(renderable.getX(), 10);
    EXPECT_EQ(renderable.getY(), 20);
    EXPECT_EQ(renderable.getWidth(), 30);
    EXPECT_EQ(renderable.getHeight(), 40);

    renderable.setVisible(false);
    renderable.setX(50);
    renderable.setY(60);
    renderable.setWidth(70);
    renderable.setHeight(80);

    EXPECT_FALSE(renderable.isVisible());
    EXPECT_EQ(renderable.getX(), 50);
    EXPECT_EQ(renderable.getY(), 60);
    EXPECT_EQ(renderable.getWidth(), 70);
    EXPECT_EQ(renderable.getHeight(), 80);
}

TEST(MAORenderable2DImageTest, LoadsBmpResourceIntoAnOpenGLTexture) {
    SDLGLContext context;
    ASSERT_NE(context.surface, nullptr) << SDL_GetError();

    const std::filesystem::path imagePath = "renderable-2d-image-test.bmp";
    SDL_Surface* bitmap = SDL_CreateRGBSurface(
        SDL_SWSURFACE, 2, 2, 24,
        0x000000ff, 0x0000ff00, 0x00ff0000, 0);
    ASSERT_NE(bitmap, nullptr) << SDL_GetError();
    ASSERT_EQ(SDL_SaveBMP(bitmap, imagePath.string().c_str()), 0)
        << SDL_GetError();
    SDL_FreeSurface(bitmap);

    {
        MAORenderable2DImage image(
            "image", boost::filesystem::path(imagePath.string()),
            1, 2, 3, 4);

        EXPECT_EQ(image.getType(), T_MAORENDERABLE2DIMAGE);
        EXPECT_EQ(image.getFilePath().generic_string(), imagePath.string());
    }

    std::filesystem::remove(imagePath);
}

}  // namespace
