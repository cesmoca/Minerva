#include <MAO/MAO.h>
#include <gtest/gtest.h>

#include <string>

TEST(MAOTest, PreservesNameAndBaseType) {
    MAO object("object");

    EXPECT_EQ(object.getName(), "object");
    EXPECT_EQ(object.getType(), T_MAO);
}

TEST(MAOTest, AddsAndRetrievesScalarProperties) {
    MAO object("object");

    object.addPropertyInt("count", 7);
    object.addPropertyFloat("speed", 2.5F);
    object.addPropertyString("label", "minerva");
    object.addPropertyBoolean("visible", true);

    EXPECT_TRUE(object.hasProperty("count"));
    EXPECT_EQ(object.getProperty("count").getValue<int>(), 7);
    EXPECT_FLOAT_EQ(object.getProperty("speed").getValue<float>(), 2.5F);
    EXPECT_EQ(object.getProperty("label").getValue<std::string>(), "minerva");
    EXPECT_TRUE(object.getProperty("visible").getValue<bool>());
}

TEST(MAOTest, AddsDefaultAndExplicitPoseProperties) {
    MAO object("object");
    object.addPropertyPose("default-pose");

    cv::Mat pose = cv::Mat::eye(4, 4, CV_32F);
    pose.at<float>(0, 3) = 4.0F;
    object.addPropertyPose("pose", pose);

    EXPECT_EQ(object.getProperty("default-pose").getType(), MAOPROPERTY_POSE);
    ASSERT_EQ(object.getProperty("default-pose").getValue<cv::Mat>().rows, 4);
    ASSERT_EQ(object.getProperty("default-pose").getValue<cv::Mat>().cols, 4);
    EXPECT_FLOAT_EQ(
        object.getProperty("pose").getValue<cv::Mat>().at<float>(0, 3),
        4.0F);
}

TEST(MAOTest, ReportsDuplicateAndMissingProperties) {
    MAO object("object");
    object.addPropertyInt("value", 1);

    EXPECT_THROW(object.addPropertyInt("value", 2), std::string);
    EXPECT_THROW(object.getProperty("missing"), std::string);
}
