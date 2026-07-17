#include <Kernel/Resources/ResourceFile.h>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

class ResourceFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        path = std::filesystem::current_path() / "resource-file-test.bin";
        std::filesystem::remove(path);
    }

    void TearDown() override {
        std::filesystem::remove(path);
    }

    std::filesystem::path path;
};

TEST_F(ResourceFileTest, LoadsBinaryFileContents) {
    const std::string contents("abc\0xyz", 7);
    {
        std::ofstream output(path, std::ios::binary);
        ASSERT_TRUE(output.is_open());
        output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    }

    ResourceFile resource(boost::filesystem::path(path.string()));

    ASSERT_TRUE(resource.isOpened());
    EXPECT_EQ(resource.getSize(), contents.size());
    EXPECT_EQ(std::string(resource.getData(), resource.getSize()), contents);

    char buffer[7] = {};
    EXPECT_EQ(resource.read(buffer, 1, sizeof(buffer)), sizeof(buffer));
    EXPECT_EQ(std::string(buffer, sizeof(buffer)), contents);
    EXPECT_TRUE(resource.isEOF());
    resource.free();
}

TEST_F(ResourceFileTest, RemainsClosedWhenFileDoesNotExist) {
    ResourceFile resource(boost::filesystem::path(path.string()));

    EXPECT_FALSE(resource.isOpened());
    EXPECT_EQ(resource.getData(), nullptr);
}

}  // namespace
