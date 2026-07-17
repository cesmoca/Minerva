#include <Kernel/Resources/Resource.h>

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <string>

namespace {

class MemoryResource : public Resource {
public:
    explicit MemoryResource(const std::string& contents)
        : Resource(boost::filesystem::path("memory")) {
        _size = contents.size();
        _data = new char[_size];
        std::copy(contents.begin(), contents.end(), _data);
        _opened = true;
    }

    ~MemoryResource() override {
        if (isOpened()) {
            free();
        }
    }

private:
    void load() override {
    }
};

TEST(ResourceTest, ExposesLoadedDataAndState) {
    MemoryResource resource("Minerva");

    EXPECT_TRUE(resource.isOpened());
    EXPECT_FALSE(resource.isEOF());
    EXPECT_EQ(resource.getSize(), 7U);
    EXPECT_EQ(std::string(resource.getData(), resource.getSize()), "Minerva");
    EXPECT_EQ(resource.tell(), 0);
}

TEST(ResourceTest, ReadsSequentiallyAndStopsAtEnd) {
    MemoryResource resource("abcdef");
    char buffer[8] = {};

    EXPECT_EQ(resource.read(buffer, 1, 2), 2);
    EXPECT_EQ(std::string(buffer, 2), "ab");
    EXPECT_EQ(resource.tell(), 2);
    EXPECT_FALSE(resource.isEOF());

    EXPECT_EQ(resource.read(buffer, 2, 3), 4);
    EXPECT_EQ(std::string(buffer, 4), "cdef");
    EXPECT_EQ(resource.tell(), 6);
    EXPECT_TRUE(resource.isEOF());
    EXPECT_EQ(resource.read(buffer, 1, 1), 0);
}

TEST(ResourceTest, SeeksAndRewindsWithinBounds) {
    MemoryResource resource("abcdef");
    char value = 0;

    EXPECT_TRUE(resource.seek(3));
    EXPECT_EQ(resource.tell(), 3);
    EXPECT_EQ(resource.read(&value, 1, 1), 1);
    EXPECT_EQ(value, 'd');

    EXPECT_TRUE(resource.seek(static_cast<unsigned long>(resource.getSize())));
    EXPECT_TRUE(resource.isEOF());
    EXPECT_FALSE(resource.seek(static_cast<unsigned long>(resource.getSize() + 1)));
    EXPECT_EQ(resource.tell(), static_cast<long>(resource.getSize()));

    resource.rewind();
    EXPECT_EQ(resource.tell(), 0);
    EXPECT_FALSE(resource.isEOF());
}

TEST(ResourceTest, FreeMarksTheResourceClosed) {
    MemoryResource resource("data");

    resource.free();

    EXPECT_FALSE(resource.isOpened());
}

}  // namespace
