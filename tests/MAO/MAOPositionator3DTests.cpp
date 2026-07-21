#include <MAO/MAOPositionator3D.h>
#include <gtest/gtest.h>

namespace {

class TestPositionator3D : public MAOPositionator3D {
public:
    using MAOPositionator3D::MAOPositionator3D;

    cv::Mat& getPosMatrix() override {
        return _posMatrix;
    }

    void setMatrix(cv::Mat& matrix) {
        setPosMatrix(matrix);
    }

    void setMatrix(const double* matrix) {
        setPosMatrix(matrix);
    }
};

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

TEST(MAOPositionator3DTest, StartsUnpositionedWithIdentityMatrix) {
    TestPositionator3D object("positionator");

    EXPECT_EQ(object.getName(), "positionator");
    EXPECT_EQ(object.getType(), T_MAOPOSITIONATOR3D);
    EXPECT_FALSE(object.isPositioned());
    expectIdentity(object.getPosMatrix());
}

TEST(MAOPositionator3DTest, ChangesPositionedState) {
    TestPositionator3D object("positionator");

    object.setPositioned(true);
    EXPECT_TRUE(object.isPositioned());

    object.setPositioned(false);
    EXPECT_FALSE(object.isPositioned());
}

TEST(MAOPositionator3DTest, CopiesFloatAndDoubleMatrices) {
    TestPositionator3D object("positionator");
    cv::Mat floatMatrix = cv::Mat::eye(4, 4, CV_32F);
    floatMatrix.at<float>(3, 0) = 2.5F;
    object.setMatrix(floatMatrix);
    EXPECT_FLOAT_EQ(object.getPosMatrix().at<float>(3, 0), 2.5F);

    const double doubleMatrix[16] = {
        1.0, 0.0, 0.0, 4.0,
        0.0, 1.0, 0.0, 5.0,
        0.0, 0.0, 1.0, 6.0,
        0.0, 0.0, 0.0, 1.0,
    };
    object.setMatrix(doubleMatrix);

    EXPECT_FLOAT_EQ(object.getPosMatrix().at<float>(0, 3), 4.0F);
    EXPECT_FLOAT_EQ(object.getPosMatrix().at<float>(1, 3), 5.0F);
    EXPECT_FLOAT_EQ(object.getPosMatrix().at<float>(2, 3), 6.0F);
}

TEST(MAOPositionator3DTest, RejectsNonFourByFourMatrix) {
    TestPositionator3D object("positionator");
    cv::Mat invalid = cv::Mat::eye(3, 3, CV_32F);

    EXPECT_THROW(object.setMatrix(invalid), const char*);
}

}  // namespace
