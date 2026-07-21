#include <Kernel/VideoSource.h>
#include <gtest/gtest.h>

#include <limits>
#include <string>

TEST(VideoSourceTest, ReportsCameraNotFoundForInvalidDevice) {
    const std::string name = "unavailable-camera";

    try {
        VideoSource source(name, std::numeric_limits<int>::max());
        FAIL() << "Expected camera construction to fail";
    } catch (const std::string& exception) {
        EXPECT_EQ(exception, "Camera not found exception: " + name);
    } catch (...) {
        FAIL() << "Expected a std::string exception";
    }
}
