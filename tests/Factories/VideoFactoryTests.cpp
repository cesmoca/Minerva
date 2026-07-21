#include <Factories/VideoFactory.h>
#include <gtest/gtest.h>

#include <string>

TEST(VideoFactoryTest, ReportsMissingMainCamera) {
    VideoFactory factory;

    EXPECT_THROW(factory.getMainCamera(), const char*);
}

TEST(VideoFactoryTest, ReportsMissingNamedCamera) {
    VideoFactory factory;

    try {
        factory.getCamera("missing-camera");
        FAIL() << "Expected camera lookup to fail";
    } catch (const std::string& exception) {
        EXPECT_EQ(exception, "No camera found with name missing-camera");
    } catch (...) {
        FAIL() << "Expected a std::string exception";
    }
}
