#include <Kernel/Parsers/Parser3ds.h>
#include <MAO/MAORenderable3DModel.h>
#include <gtest/gtest.h>

#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>

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

class Lib3dsFileHandle {
public:
    Lib3dsFileHandle() : file(lib3ds_file_new()) {}
    ~Lib3dsFileHandle() { lib3ds_file_free(file); }

    Lib3dsFile* file;
};

class Temporary3dsFixture {
public:
    explicit Temporary3dsFixture(const std::string& stem)
        : modelPath_(stem + ".3ds"), texturePath_(stem + ".ppm") {}

    bool create(bool textured) {
        Lib3dsFileHandle fixture;
        if (fixture.file == nullptr) {
            return false;
        }

        fixture.file->frames = 1;
        fixture.file->segment_from = 0;
        fixture.file->segment_to = 0;

        Lib3dsMesh* mesh = lib3ds_mesh_new("triangle");
        if (mesh == nullptr) {
            return false;
        }
        if (!lib3ds_mesh_new_point_list(mesh, 3) ||
            !lib3ds_mesh_new_texel_list(mesh, 3) ||
            !lib3ds_mesh_new_face_list(mesh, 1)) {
            lib3ds_mesh_free(mesh);
            return false;
        }

        setPoint(mesh->pointL[0], -1.0F, -2.0F, -3.0F);
        setPoint(mesh->pointL[1], 1.0F, 2.0F, 3.0F);
        setPoint(mesh->pointL[2], 0.0F, 1.0F, 0.0F);

        mesh->texelL[0][0] = 0.0F;
        mesh->texelL[0][1] = 0.0F;
        mesh->texelL[1][0] = 1.0F;
        mesh->texelL[1][1] = 0.0F;
        mesh->texelL[2][0] = 0.0F;
        mesh->texelL[2][1] = 1.0F;

        mesh->faceL[0].points[0] = 0;
        mesh->faceL[0].points[1] = 1;
        mesh->faceL[0].points[2] = 2;

        if (textured) {
            if (!writePpm()) {
                lib3ds_mesh_free(mesh);
                return false;
            }
            Lib3dsMaterial* material = lib3ds_material_new();
            if (material == nullptr) {
                lib3ds_mesh_free(mesh);
                return false;
            }
            std::strcpy(material->name, "fixture-material");
            std::strcpy(material->texture1_map.name,
                        texturePath_.filename().string().c_str());
            std::strcpy(mesh->faceL[0].material, material->name);
            lib3ds_file_insert_material(fixture.file, material);
        }

        lib3ds_file_insert_mesh(fixture.file, mesh);
        return lib3ds_file_save(
                   fixture.file, modelPath_.string().c_str()) ==
               LIB3DS_TRUE;
    }

    ~Temporary3dsFixture() {
        std::filesystem::remove(modelPath_);
        std::filesystem::remove(texturePath_);
    }

    const std::filesystem::path& modelPath() const { return modelPath_; }

private:
    static void setPoint(Lib3dsPoint& point, float x, float y, float z) {
        point.pos[0] = x;
        point.pos[1] = y;
        point.pos[2] = z;
    }

    bool writePpm() const {
        std::ofstream output(texturePath_, std::ios::binary);
        if (!output.is_open()) {
            return false;
        }
        output << "P6\n1 1\n255\n";
        const char pixel[] = {static_cast<char>(0x20),
                              static_cast<char>(0x40),
                              static_cast<char>(0x80)};
        output.write(pixel, sizeof(pixel));
        return output.good();
    }

    std::filesystem::path modelPath_;
    std::filesystem::path texturePath_;
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

void loadFixtureAndExpectGeometry(const std::string& stem, bool textured) {
    SDLGLContext context;
    ASSERT_NE(context.surface, nullptr) << SDL_GetError();

    Temporary3dsFixture fixture(stem);
    ASSERT_TRUE(fixture.create(textured));
    const boost::filesystem::path modelPath(fixture.modelPath().string());
    MAORenderable3DModel model("3ds-model", modelPath, 2.0F);

    Parser3ds::getInstance()->loadModel(modelPath, model);

    EXPECT_NO_THROW(model.playAnim());
    expectBoxHalfExtents(model, 2.0F, 4.0F, 6.0F);
}

TEST(Parser3dsTest, LoadsGeneratedGeometryAndAppliesModelScale) {
    loadFixtureAndExpectGeometry("parser-3ds-geometry-test", false);
}

TEST(Parser3dsTest, LoadsGeneratedMaterialAndTextureResource) {
    loadFixtureAndExpectGeometry("parser-3ds-texture-test", true);
}

}  // namespace
