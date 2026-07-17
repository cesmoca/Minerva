#include <Kernel/PathPoint.h>
#include <gtest/gtest.h>

TEST(PathPointTest, PreservesConstructorValues) {
    PathPoint point(-1.5F, 2.25F, 3.75F, 0.5F, 10, 20, 30, true);
    EXPECT_FLOAT_EQ(point.getX(), -1.5F);
    EXPECT_FLOAT_EQ(point.getY(), 2.25F);
    EXPECT_FLOAT_EQ(point.getZ(), 3.75F);
    EXPECT_FLOAT_EQ(point.getSize(), 0.5F);
    EXPECT_EQ(point.getR(), 10);
    EXPECT_EQ(point.getG(), 20);
    EXPECT_EQ(point.getB(), 30);
    EXPECT_TRUE(point.getVisible());

    PathPoint hidden(0.0F, 0.0F, 0.0F, 0.0F, 0, 0, 0, false);
    EXPECT_FALSE(hidden.getVisible());
}
