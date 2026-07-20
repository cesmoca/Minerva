#include <MAO/MAOProperty.h>
#include <gtest/gtest.h>

#include <string>

TEST(MAOPropertyTest, PreservesIntegerNameTypeValueAndFormatting) {
    MAOProperty property("lives", MAOPROPERTY_INT, 3);

    EXPECT_EQ(property.getName(), "lives");
    EXPECT_EQ(property.getType(), MAOPROPERTY_INT);
    EXPECT_EQ(property.getValue<int>(), 3);
    EXPECT_EQ(property.toString(), "3");

    property.setValue<int>(5);
    EXPECT_EQ(property.getName(), "lives");
    EXPECT_EQ(property.getValue<int>(), 5);
}

TEST(MAOPropertyTest, PreservesFloatBooleanAndStringProperties) {
    MAOProperty floating("speed", MAOPROPERTY_FLOAT, 1.25F);
    EXPECT_EQ(floating.getName(), "speed");
    EXPECT_EQ(floating.getType(), MAOPROPERTY_FLOAT);
    EXPECT_FLOAT_EQ(floating.getValue<float>(), 1.25F);

    MAOProperty boolean("visible", MAOPROPERTY_BOOLEAN, true);
    EXPECT_EQ(boolean.getName(), "visible");
    EXPECT_EQ(boolean.getType(), MAOPROPERTY_BOOLEAN);
    EXPECT_TRUE(boolean.getValue<bool>());

    MAOProperty string("label", MAOPROPERTY_STRING, std::string("Minerva"));
    EXPECT_EQ(string.getName(), "label");
    EXPECT_EQ(string.getType(), MAOPROPERTY_STRING);
    EXPECT_EQ(string.getValue<std::string>(), "Minerva");
}

TEST(MAOPropertyTest, PreservesPoseProperty) {
    cv::Mat pose = cv::Mat::eye(4, 4, CV_32F);
    pose.at<float>(1, 3) = 7.5F;
    MAOProperty property("pose", MAOPROPERTY_POSE, pose);

    EXPECT_EQ(property.getName(), "pose");
    EXPECT_EQ(property.getType(), MAOPROPERTY_POSE);
    ASSERT_EQ(property.getValue<cv::Mat>().rows, 4);
    ASSERT_EQ(property.getValue<cv::Mat>().cols, 4);
    EXPECT_FLOAT_EQ(property.getValue<cv::Mat>().at<float>(1, 3), 7.5F);
}

TEST(MAOPropertyTest, CopyConstructionPreservesNameAndIndependentValue) {
    MAOProperty original("title", MAOPROPERTY_STRING, std::string("original"));
    MAOProperty copy(original);

    EXPECT_EQ(copy.getName(), "title");
    EXPECT_EQ(copy.getType(), MAOPROPERTY_STRING);
    copy.setValue<std::string>("copy");
    EXPECT_EQ(original.getValue<std::string>(), "original");
    EXPECT_EQ(copy.getValue<std::string>(), "copy");
}

TEST(MAOPropertyTest, CopyConstructionDeepCopiesPose) {
    cv::Mat pose = cv::Mat::eye(4, 4, CV_32F);
    MAOProperty original("transform", MAOPROPERTY_POSE, pose);
    MAOProperty copy(original);

    copy.getValue<cv::Mat>().at<float>(0, 0) = 9.0F;

    EXPECT_EQ(copy.getName(), "transform");
    EXPECT_FLOAT_EQ(original.getValue<cv::Mat>().at<float>(0, 0), 1.0F);
    EXPECT_FLOAT_EQ(copy.getValue<cv::Mat>().at<float>(0, 0), 9.0F);
}

TEST(MPYPropertyTest, IntegerAliasForwardsNameValueAndMutation) {
    MAOProperty property("score", MAOPROPERTY_INT, 10);
    MPYPropertyInt wrapper(&property);

    EXPECT_EQ(wrapper.getName(), "score");
    EXPECT_EQ(wrapper.getValue(), 10);

    wrapper.setValue(25);
    EXPECT_EQ(wrapper.getValue(), 25);
    EXPECT_EQ(property.getValue<int>(), 25);

    property.setValue<int>(30);
    EXPECT_EQ(wrapper.getValue(), 30);
}

TEST(MPYPropertyTest, FloatBooleanAndStringAliasesForwardValues) {
    MAOProperty floatProperty("weight", MAOPROPERTY_FLOAT, 2.5F);
    MPYPropertyFloat floatWrapper(&floatProperty);
    EXPECT_EQ(floatWrapper.getName(), "weight");
    EXPECT_FLOAT_EQ(floatWrapper.getValue(), 2.5F);
    floatWrapper.setValue(4.75F);
    EXPECT_FLOAT_EQ(floatProperty.getValue<float>(), 4.75F);

    MAOProperty boolProperty("enabled", MAOPROPERTY_BOOLEAN, false);
    MPYPropertyBool boolWrapper(&boolProperty);
    EXPECT_EQ(boolWrapper.getName(), "enabled");
    EXPECT_FALSE(boolWrapper.getValue());
    boolWrapper.setValue(true);
    EXPECT_TRUE(boolProperty.getValue<bool>());

    MAOProperty stringProperty("message", MAOPROPERTY_STRING, std::string("before"));
    MPYPropertyStr stringWrapper(&stringProperty);
    EXPECT_EQ(stringWrapper.getName(), "message");
    EXPECT_EQ(stringWrapper.getValue(), "before");
    stringWrapper.setValue("after");
    EXPECT_EQ(stringProperty.getValue<std::string>(), "after");
}

TEST(MPYPropertyTest, PoseAliasForwardsMatrixReferenceAndMutation) {
    cv::Mat pose = cv::Mat::eye(4, 4, CV_32F);
    MAOProperty property("camera-pose", MAOPROPERTY_POSE, pose);
    MPYPropertyPose wrapper(&property);

    EXPECT_EQ(wrapper.getName(), "camera-pose");
    wrapper.getValue().at<float>(2, 3) = 6.0F;
    EXPECT_FLOAT_EQ(property.getValue<cv::Mat>().at<float>(2, 3), 6.0F);

    cv::Mat replacement = cv::Mat::zeros(4, 4, CV_32F);
    replacement.at<float>(3, 0) = 8.0F;
    wrapper.setValue(replacement);
    EXPECT_FLOAT_EQ(property.getValue<cv::Mat>().at<float>(3, 0), 8.0F);
}
