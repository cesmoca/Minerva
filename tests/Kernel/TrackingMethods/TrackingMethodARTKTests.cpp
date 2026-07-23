#include <Kernel/TrackingMethods/TrackingMethodARTK.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

namespace {

class TemporaryPatternFixture {
public:
    TemporaryPatternFixture()
        : path_("tracking-method-artk-pattern-test.patt") {
        std::ofstream output(path_, std::ios::binary);
        for (int orientation = 0; orientation < 4; ++orientation) {
            for (int channel = 0; channel < 3; ++channel) {
                for (int pixel = 0; pixel < 16 * 16; ++pixel) {
                    output << ((orientation * 17 + channel * 31 + pixel) % 256)
                           << '\n';
                }
            }
        }
        output.put('\0');
    }

    ~TemporaryPatternFixture() { std::filesystem::remove(path_); }

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

TEST(TrackingMethodARTKTest, InitializesActiveAndSupportsActivationState) {
    TrackingMethodARTK method;

    EXPECT_TRUE(method.isActive());
    method.deactive();
    EXPECT_FALSE(method.isActive());
    method.active();
    EXPECT_TRUE(method.isActive());
}

TEST(TrackingMethodARTKTest, LoadsValidPatternFromResource) {
    TemporaryPatternFixture fixture;
    ASSERT_TRUE(std::filesystem::exists(fixture.path()));

    TrackingMethodARTK method;
    MAOMark mark("pattern", boost::filesystem::path(fixture.path().string()),
                 80.0F);

    method.addMAOMark(mark);

    EXPECT_GE(mark.getId(), 0);
}

}  // namespace
