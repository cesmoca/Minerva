#include <Factories/TrackingMethodFactory.h>
#include <gtest/gtest.h>

TEST(TrackingMethodFactoryTest, ReturnsOwnedARTKMethodAndSkipsInactivePoll) {
    TrackingMethodFactory factory;
    TrackingMethodARTK& method = factory.getTrackingMethodARTK();

    EXPECT_EQ(&method, &factory.getTrackingMethodARTK());

    method.deactive();
    EXPECT_NO_THROW(factory.pollMethods());
}
