#include <MAO/MAOMark.h>
#include <gtest/gtest.h>

namespace {

void expectIdentity(const cv::Mat& matrix) {
    ASSERT_EQ(matrix.rows, 4);
    ASSERT_EQ(matrix.cols, 4);
    ASSERT_EQ(matrix.type(), CV_32F);

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            EXPECT_FLOAT_EQ(
                matrix.at<float>(row, column),
                row == column ? 1.0F : 0.0F);
        }
    }
}

TEST(MAOMarkTest, PreservesConfigurationAndInitialMatrices) {
    const boost::filesystem::path path("markers/hiro.patt");
    MAOMark mark("hiro", path, 80.0F);

    EXPECT_EQ(mark.getName(), "hiro");
    EXPECT_EQ(mark.getType(), T_MAOMARK);
    EXPECT_EQ(mark.getPath(), path);
    EXPECT_FLOAT_EQ(mark.getSize(), 80.0F);
    ASSERT_NE(mark.getCenter(), nullptr);
    EXPECT_DOUBLE_EQ(mark.getCenter()[0], 0.0);
    EXPECT_DOUBLE_EQ(mark.getCenter()[1], 0.0);
    expectIdentity(mark.getPosMatrix());
    expectIdentity(mark.getOffsetMatrix());
}

TEST(MAOMarkTest, UpdatesIdentifierAndOffsetMatrix) {
    MAOMark mark("hiro", boost::filesystem::path("hiro.patt"), 80.0F);
    mark.setId(17);
    EXPECT_EQ(mark.getId(), 17);

    cv::Mat offset = cv::Mat::eye(4, 4, CV_32F);
    offset.at<float>(0, 3) = 2.0F;
    mark.setOffsetMatrix(offset);
    EXPECT_FLOAT_EQ(mark.getOffsetMatrix().at<float>(0, 3), 2.0F);

    mark.setOffsetMatrix(static_cast<const cv::Mat*>(nullptr));
    EXPECT_FLOAT_EQ(mark.getOffsetMatrix().at<float>(0, 3), 2.0F);
}

TEST(MAOMarkTest, ComposesTrackingMatrixWithOffset) {
    MAOMark mark("hiro", boost::filesystem::path("hiro.patt"), 80.0F);
    const double offset[16] = {
        1.0, 0.0, 0.0, 2.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };
    mark.setOffsetMatrix(offset);

    cv::Mat tracking = cv::Mat::eye(4, 4, CV_32F);
    tracking.at<float>(1, 3) = 3.0F;
    mark.setTrackingMatrix(tracking);

    EXPECT_FLOAT_EQ(mark.getPosMatrix().at<float>(0, 3), 2.0F);
    EXPECT_FLOAT_EQ(mark.getPosMatrix().at<float>(1, 3), 3.0F);
}

TEST(MAOMarkTest, RejectsInvalidOffsetMatrixDimensions) {
    MAOMark mark("hiro", boost::filesystem::path("hiro.patt"), 80.0F);
    const cv::Mat invalid = cv::Mat::eye(3, 3, CV_32F);

    EXPECT_THROW(mark.setOffsetMatrix(invalid), const char*);
}

}  // namespace
