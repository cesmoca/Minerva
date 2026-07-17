#include <Kernel/TrackingMethods/TrackingMethod.h>
#include <gtest/gtest.h>

namespace {

class TestTrackingMethod : public TrackingMethod {
public:
    void pollMethod() override {}
};

}

TEST(TrackingMethodTest, ChangesActiveState) {
    TestTrackingMethod trackingMethod;
    EXPECT_TRUE(trackingMethod.isActive());

    trackingMethod.deactive();
    EXPECT_FALSE(trackingMethod.isActive());

    trackingMethod.active();
    EXPECT_TRUE(trackingMethod.isActive());
}
