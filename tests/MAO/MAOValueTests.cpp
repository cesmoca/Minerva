#include <MAO/MAOValue.h>
#include <gtest/gtest.h>

#include <string>

TEST(MAOValueTest, DefaultsToEmpty) {
    MAOValue value;
    EXPECT_EQ(value.getType(), MAOPROPERTY_EMPTY);
    EXPECT_EQ(value.toString(), "");
}

TEST(MAOValueTest, StoresUpdatesAndFormatsInteger) {
    MAOValue value(MAOPROPERTY_INT, -42);
    EXPECT_EQ(value.getType(), MAOPROPERTY_INT);
    EXPECT_EQ(value.getValue<int>(), -42);
    EXPECT_EQ(value.toString(), "-42");

    value.setValue<int>(17);
    EXPECT_EQ(value.getValue<int>(), 17);
    EXPECT_EQ(value.toString(), "17");
}

TEST(MAOValueTest, StoresUpdatesAndFormatsFloat) {
    MAOValue value(MAOPROPERTY_FLOAT, 1.25F);
    EXPECT_EQ(value.getType(), MAOPROPERTY_FLOAT);
    EXPECT_FLOAT_EQ(value.getValue<float>(), 1.25F);
    EXPECT_EQ(value.toString(), "1.25");

    value.setValue<float>(-2.5F);
    EXPECT_FLOAT_EQ(value.getValue<float>(), -2.5F);
    EXPECT_EQ(value.toString(), "-2.5");
}

TEST(MAOValueTest, StoresUpdatesAndFormatsBoolean) {
    MAOValue value(MAOPROPERTY_BOOLEAN, false);
    EXPECT_EQ(value.getType(), MAOPROPERTY_BOOLEAN);
    EXPECT_FALSE(value.getValue<bool>());
    EXPECT_EQ(value.toString(), "false");

    value.setValue<bool>(true);
    EXPECT_TRUE(value.getValue<bool>());
    EXPECT_EQ(value.toString(), "true");
}

TEST(MAOValueTest, StoresUpdatesAndFormatsString) {
    MAOValue value(MAOPROPERTY_STRING, std::string("Minerva value"));
    EXPECT_EQ(value.getType(), MAOPROPERTY_STRING);
    EXPECT_EQ(value.getValue<std::string>(), "Minerva value");
    EXPECT_EQ(value.toString(), "Minerva value");

    value.setValue<std::string>("updated value");
    EXPECT_EQ(value.getValue<std::string>(), "updated value");
    EXPECT_EQ(value.toString(), "updated value");
}

TEST(MAOValueTest, CopyConstructsIndependentScalarAndStringValues) {
    MAOValue integer(MAOPROPERTY_INT, 11);
    MAOValue integerCopy(integer);
    integerCopy.setValue<int>(22);
    EXPECT_EQ(integer.getValue<int>(), 11);
    EXPECT_EQ(integerCopy.getValue<int>(), 22);

    MAOValue floating(MAOPROPERTY_FLOAT, 3.5F);
    MAOValue floatingCopy(floating);
    floatingCopy.setValue<float>(7.0F);
    EXPECT_FLOAT_EQ(floating.getValue<float>(), 3.5F);
    EXPECT_FLOAT_EQ(floatingCopy.getValue<float>(), 7.0F);

    MAOValue boolean(MAOPROPERTY_BOOLEAN, true);
    MAOValue booleanCopy(boolean);
    booleanCopy.setValue<bool>(false);
    EXPECT_TRUE(boolean.getValue<bool>());
    EXPECT_FALSE(booleanCopy.getValue<bool>());

    MAOValue string(MAOPROPERTY_STRING, std::string("original"));
    MAOValue stringCopy(string);
    stringCopy.setValue<std::string>("copy");
    EXPECT_EQ(string.getValue<std::string>(), "original");
    EXPECT_EQ(stringCopy.getValue<std::string>(), "copy");
}

TEST(MAOValueTest, CopyConstructsIndependentPose) {
    cv::Mat pose = (cv::Mat_<float>(4, 4) <<
        1.0F, 2.0F, 3.0F, 4.0F,
        5.0F, 6.0F, 7.0F, 8.0F,
        9.0F, 10.0F, 11.0F, 12.0F,
        13.0F, 14.0F, 15.0F, 16.0F);
    MAOValue value(MAOPROPERTY_POSE, pose);
    MAOValue copy(value);

    cv::Mat& originalPose = value.getValue<cv::Mat>();
    cv::Mat& copiedPose = copy.getValue<cv::Mat>();
    ASSERT_EQ(copiedPose.rows, 4);
    ASSERT_EQ(copiedPose.cols, 4);
    EXPECT_EQ(copiedPose.type(), CV_32F);
    EXPECT_FLOAT_EQ(copiedPose.at<float>(2, 3), 12.0F);

    copiedPose.at<float>(2, 3) = 99.0F;
    EXPECT_FLOAT_EQ(originalPose.at<float>(2, 3), 12.0F);
    EXPECT_FLOAT_EQ(copiedPose.at<float>(2, 3), 99.0F);
}

TEST(MAOValueTest, AssignsValuesWithOriginalCopySemantics) {
    MAOValue integerSource(MAOPROPERTY_INT, 5);
    MAOValue integerTarget;
    integerTarget = integerSource;
    integerTarget.setValue<int>(6);
    EXPECT_EQ(integerSource.getValue<int>(), 5);
    EXPECT_EQ(integerTarget.getValue<int>(), 6);

    MAOValue floatSource(MAOPROPERTY_FLOAT, 1.5F);
    MAOValue floatTarget;
    floatTarget = floatSource;
    floatTarget.setValue<float>(2.5F);
    EXPECT_FLOAT_EQ(floatSource.getValue<float>(), 1.5F);
    EXPECT_FLOAT_EQ(floatTarget.getValue<float>(), 2.5F);

    MAOValue booleanSource(MAOPROPERTY_BOOLEAN, true);
    MAOValue booleanTarget;
    booleanTarget = booleanSource;
    booleanTarget.setValue<bool>(false);
    EXPECT_TRUE(booleanSource.getValue<bool>());
    EXPECT_FALSE(booleanTarget.getValue<bool>());

    MAOValue stringSource(MAOPROPERTY_STRING, std::string("source"));
    MAOValue stringTarget;
    stringTarget = stringSource;
    stringTarget.setValue<std::string>("target");
    EXPECT_EQ(stringSource.getValue<std::string>(), "source");
    EXPECT_EQ(stringTarget.getValue<std::string>(), "target");

    cv::Mat pose = cv::Mat::eye(4, 4, CV_32F);
    MAOValue poseSource(MAOPROPERTY_POSE, pose);
    MAOValue poseTarget;
    poseTarget = poseSource;
    poseTarget.getValue<cv::Mat>().at<float>(0, 0) = 8.0F;
    EXPECT_FLOAT_EQ(poseSource.getValue<cv::Mat>().at<float>(0, 0), 8.0F);
    EXPECT_FLOAT_EQ(poseTarget.getValue<cv::Mat>().at<float>(0, 0), 8.0F);
}

TEST(MAOValueTest, SelfAssignmentPreservesValue) {
    MAOValue value(MAOPROPERTY_STRING, std::string("unchanged"));
    value = value;
    EXPECT_EQ(value.getType(), MAOPROPERTY_STRING);
    EXPECT_EQ(value.getValue<std::string>(), "unchanged");
}

TEST(MAOValueTest, CopyingEmptyValueReportsUnsupportedType) {
    MAOValue empty;
    EXPECT_THROW({ MAOValue copy(empty); }, const char*);
}

TEST(MAOValueTest, PoseStringConversionIsEmpty) {
    cv::Mat identity = cv::Mat::eye(4, 4, CV_32F);
    MAOValue pose(MAOPROPERTY_POSE, identity);
    EXPECT_EQ(pose.toString(), "");
}
