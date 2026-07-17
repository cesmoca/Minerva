#include <Kernel/Resources/ResourceZip.h>

#include <gtest/gtest.h>

#include <zip.h>

#include <filesystem>
#include <string>

namespace {

class ResourceZipTest : public ::testing::Test {
protected:
    void SetUp() override {
        archivePath = std::filesystem::current_path() / "resource-zip-test.zip";
        std::filesystem::remove(archivePath);
    }

    void TearDown() override {
        std::filesystem::remove(archivePath);
    }

    void CreateArchive(const std::string& entryName, const std::string& contents) {
        int error = 0;
        zip_t* archive = zip_open(archivePath.string().c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
        ASSERT_NE(archive, nullptr);
        zip_source_t* source = zip_source_buffer(archive, contents.data(), contents.size(), 0);
        ASSERT_NE(source, nullptr);
        ASSERT_GE(zip_file_add(archive, entryName.c_str(), source, ZIP_FL_OVERWRITE), 0);
        ASSERT_EQ(zip_close(archive), 0);
    }

    std::filesystem::path archivePath;
};

TEST_F(ResourceZipTest, LoadsAnExistingArchiveEntry) {
    const std::string contents("zip\0data", 8);
    CreateArchive("assets/item.bin", contents);

    int error = 0;
    zip_t* archive = zip_open(archivePath.string().c_str(), ZIP_RDONLY, &error);
    ASSERT_NE(archive, nullptr);

    ResourceZip resource(archive, boost::filesystem::path("assets/item.bin"));

    ASSERT_TRUE(resource.isOpened());
    EXPECT_EQ(resource.getSize(), contents.size());
    EXPECT_EQ(std::string(resource.getData(), resource.getSize()), contents);
    resource.free();
    EXPECT_EQ(zip_close(archive), 0);
}

TEST_F(ResourceZipTest, RemainsClosedWhenEntryDoesNotExist) {
    CreateArchive("present.txt", "present");

    int error = 0;
    zip_t* archive = zip_open(archivePath.string().c_str(), ZIP_RDONLY, &error);
    ASSERT_NE(archive, nullptr);

    ResourceZip resource(archive, boost::filesystem::path("missing.txt"));

    EXPECT_FALSE(resource.isOpened());
    EXPECT_EQ(resource.getData(), nullptr);
    EXPECT_EQ(zip_close(archive), 0);
}

}  // namespace
