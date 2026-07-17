#include <Kernel/Resources/ResourcesManager.h>

#include <gtest/gtest.h>

#include <zip.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

class ResourcesManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        resourcePath = std::filesystem::current_path() / "manager-resource.bin";
        dataPath = std::filesystem::current_path() / ResourcesManager::dataFileName;
        std::filesystem::remove(resourcePath);
        std::filesystem::remove(dataPath);
    }

    void TearDown() override {
        std::filesystem::remove(resourcePath);
        std::filesystem::remove(dataPath);
    }

    static std::string ReadEntry(zip_t* archive, const std::string& name) {
        zip_stat_t details;
        zip_stat_init(&details);
        EXPECT_EQ(zip_stat(archive, name.c_str(), 0, &details), 0);
        zip_file_t* file = zip_fopen(archive, name.c_str(), 0);
        EXPECT_NE(file, nullptr);
        if (file == nullptr) {
            return {};
        }

        std::vector<char> buffer(static_cast<std::size_t>(details.size));
        EXPECT_EQ(zip_fread(file, buffer.data(), details.size), static_cast<zip_int64_t>(details.size));
        EXPECT_EQ(zip_fclose(file), 0);
        return std::string(buffer.begin(), buffer.end());
    }

    std::filesystem::path resourcePath;
    std::filesystem::path dataPath;
};

TEST_F(ResourcesManagerTest, DefinesOriginalPackageNames) {
    EXPECT_EQ(ResourcesManager::dataFileName, "data.dat");
    EXPECT_EQ(ResourcesManager::sourceCodeFile, "code.mrv");
}

TEST_F(ResourcesManagerTest, FallsBackToAFileWhenPackageIsAbsent) {
    const std::string contents = "standalone resource";
    {
        std::ofstream output(resourcePath, std::ios::binary);
        ASSERT_TRUE(output.is_open());
        output << contents;
    }

    ResourcesManager manager;
    Resource& resource = manager.getResource(resourcePath.filename().string());

    ASSERT_TRUE(resource.isOpened());
    EXPECT_EQ(std::string(resource.getData(), resource.getSize()), contents);
    resource.free();
    delete &resource;
}

TEST_F(ResourcesManagerTest, PacksResourcesAndProcessedSourceCode) {
    const std::string contents("resource\0bytes", 14);
    {
        std::ofstream output(resourcePath, std::ios::binary);
        ASSERT_TRUE(output.is_open());
        output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    }
    std::stringstream source;
    source << "world { }";

    ResourcesManager manager;
    manager.addResource(resourcePath.filename().string());
    manager.pack(source);

    int error = 0;
    zip_t* archive = zip_open(dataPath.string().c_str(), ZIP_RDONLY, &error);
    ASSERT_NE(archive, nullptr);
    EXPECT_EQ(ReadEntry(archive, resourcePath.filename().string()), contents);
    EXPECT_EQ(ReadEntry(archive, ResourcesManager::sourceCodeFile), source.str());
    EXPECT_EQ(zip_close(archive), 0);
}

}  // namespace
