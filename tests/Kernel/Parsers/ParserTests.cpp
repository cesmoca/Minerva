#include <Kernel/Parsers/ParserObj.h>
#include <Kernel/Parsers/ParserOrej.h>
#include <MAO/MAORenderable3DModel.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

namespace {

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

class TemporaryFile {
public:
    explicit TemporaryFile(std::filesystem::path path)
        : path_(std::move(path)) {}

    ~TemporaryFile() { std::filesystem::remove(path_); }

    void writeNullTerminatedText(const std::string& contents) const {
        std::ofstream output(path_, std::ios::binary);
        ASSERT_TRUE(output.is_open());
        output.write(contents.c_str(),
                     static_cast<std::streamsize>(contents.size() + 1));
        ASSERT_TRUE(output.good());
    }

    void writePpm() const {
        std::ofstream output(path_, std::ios::binary);
        ASSERT_TRUE(output.is_open());
        output << "P6\n1 1\n255\n";
        const char pixel[] = {static_cast<char>(0x20),
                              static_cast<char>(0x40),
                              static_cast<char>(0x80)};
        output.write(pixel, sizeof(pixel));
        ASSERT_TRUE(output.good());
    }

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

void expectBoxHalfExtents(MAORenderable3DModel& model,
                          float x, float y, float z) {
    model.generateBoxShape();
    auto* shape = static_cast<btBoxShape*>(model.getCollisionShape());
    ASSERT_NE(shape, nullptr);

    const btVector3 extents = shape->getHalfExtentsWithMargin();
    EXPECT_FLOAT_EQ(extents.x(), x);
    EXPECT_FLOAT_EQ(extents.y(), y);
    EXPECT_FLOAT_EQ(extents.z(), z);

    delete shape;
}

TEST(ParserObjTest, LoadsTriangularGeometryAndAppliesModelScale) {
    SDLGLContext context;
    ASSERT_NE(context.surface, nullptr) << SDL_GetError();

    TemporaryFile obj("parser-obj-test.obj");
    obj.writeNullTerminatedText(
        "# triangle\n"
        "v -1 -2 -3\n"
        "v 1 2 3\n"
        "v 0 1 0\n"
        "vt 0 0\n"
        "vt 1 0\n"
        "vt 0 1\n"
        "vn 0 0 1\n"
        "f 1/1/1 2/2/1 3/3/1\n");

    MAORenderable3DModel model(
        "obj-model", boost::filesystem::path(obj.path().string()), 2.0F);
    ParserObj::getInstance()->loadModel(
        boost::filesystem::path(obj.path().string()), model);

    expectBoxHalfExtents(model, 2.0F, 4.0F, 6.0F);
}

TEST(ParserOrejTest, LoadsTexturedGeometryAnimationAndAppliesModelScale) {
    SDLGLContext context;
    ASSERT_NE(context.surface, nullptr) << SDL_GetError();

    TemporaryFile orej("parser-orej-test.orej");
    TemporaryFile texture("parser-orej-test.ppm");
    texture.writePpm();
    orej.writeNullTerminatedText(
        "# triangle\n"
        "v -1 -2 -3\n"
        "v 1 2 3\n"
        "v 0 1 0\n"
        "f 1 2 3\n"
        "t 0 0 1 0 0 1\n"
        "m 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1\n");

    MAORenderable3DModel model(
        "orej-model", boost::filesystem::path(orej.path().string()), 3.0F);
    ParserOrej::getInstance()->loadModel(
        boost::filesystem::path(orej.path().string()), model);

    EXPECT_NO_THROW(model.playAnim());
    expectBoxHalfExtents(model, 3.0F, 6.0F, 9.0F);
}

}  // namespace
