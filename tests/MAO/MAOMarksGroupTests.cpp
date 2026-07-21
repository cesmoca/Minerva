#include <MAO/MAOMarksGroup.h>
#include <gtest/gtest.h>

namespace {

TEST(MAOMarksGroupTest, StartsUnpositionedWithIdentityMatrix) {
    MAOMarksGroup group("group");

    EXPECT_EQ(group.getName(), "group");
    EXPECT_EQ(group.getType(), T_MAOMARKSGROUP);
    EXPECT_FALSE(group.isPositioned());
    EXPECT_FLOAT_EQ(group.getPosMatrix().at<float>(0, 0), 1.0F);
    EXPECT_FLOAT_EQ(group.getPosMatrix().at<float>(3, 3), 1.0F);
}

TEST(MAOMarksGroupTest, UsesFirstPositionedMarkAndUpdatesState) {
    MAOMark first("first", boost::filesystem::path("first.patt"), 80.0F);
    MAOMark second("second", boost::filesystem::path("second.patt"), 80.0F);
    MAOMarksGroup group("group");
    group.addMarktoGroup(first);
    group.addMarktoGroup(second);

    first.getPosMatrix().at<float>(0, 3) = 3.0F;
    first.setPositioned(true);
    second.getPosMatrix().at<float>(0, 3) = 7.0F;
    second.setPositioned(true);

    group.checkIfPositioned();
    EXPECT_TRUE(group.isPositioned());
    EXPECT_FLOAT_EQ(group.getPosMatrix().at<float>(0, 3), 3.0F);

    first.setPositioned(false);
    second.setPositioned(false);
    group.checkIfPositioned();
    EXPECT_FALSE(group.isPositioned());
}

TEST(MAOMarksGroupTest, RejectsDuplicateMarkName) {
    MAOMark first("mark", boost::filesystem::path("first.patt"), 80.0F);
    MAOMark duplicate("mark", boost::filesystem::path("second.patt"), 80.0F);
    MAOMarksGroup group("group");
    group.addMarktoGroup(first);

    EXPECT_THROW(group.addMarktoGroup(duplicate), std::string);
}

}  // namespace
