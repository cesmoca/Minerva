#include <MLB/MLB.h>
#include <gtest/gtest.h>

TEST(MLBTest, PreservesNameAndParent) {
    MAO parent("parent");
    MLB brick("brick", parent);

    EXPECT_EQ(brick.getName(), "brick");
    EXPECT_EQ(&brick.getParent(), &parent);
}
