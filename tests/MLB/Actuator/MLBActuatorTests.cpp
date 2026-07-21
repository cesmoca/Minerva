#include <MLB/Actuator/MLBActuatorAng.h>
#include <MLB/Actuator/MLBActuatorDistance.h>
#include <MLB/Actuator/MLBActuatorProperty.h>
#include <MLB/Actuator/MLBActuatorQuitApp.h>
#include <MLB/Actuator/MLBActuatorRandom.h>
#include <MLB/Actuator/MLBActuatorRelativePose.h>
#include <Kernel/EndController.h>
#include <gtest/gtest.h>

namespace {

class TestPositionator : public MAOPositionator3D {
public:
    using MAOPositionator3D::MAOPositionator3D;
    cv::Mat& getPosMatrix() override { return _posMatrix; }
};

class CountingActuator : public MLBActuator {
public:
    CountingActuator(const std::string& name, MAO& parent)
        : MLBActuator(name, parent) {}

    int count = 0;

private:
    void specificActuate() override { ++count; }
};

TEST(MLBActuatorTest, ActuatesAndTriggersLinkedSensor) {
    MAO parent("parent");
    CountingActuator actuator("actuator", parent);
    MLBSensorActuator sensor("sensor", parent);
    actuator.addMLBSensorActuator(sensor);

    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATOR);
    actuator.actuate();
    EXPECT_EQ(actuator.count, 1);
    EXPECT_TRUE(sensor.getState());
}

TEST(MLBActuatorAngTest, WritesAngleForPositionedIdentityPose) {
    TestPositionator parent("parent");
    parent.setPositioned(true);
    MAOProperty property("angle", MAOPROPERTY_FLOAT, -1.0F);
    MLBActuatorAng actuator("angle", parent, property, ANG_X);

    actuator.actuate();
    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORANG);
    EXPECT_FLOAT_EQ(property.getValue<float>(), 0.0F);

    MAOProperty invalid("invalid", MAOPROPERTY_BOOLEAN, false);
    EXPECT_THROW(
        MLBActuatorAng("invalid", parent, invalid, ANG_X), std::string);
}

TEST(MLBActuatorDistanceTest, WritesDistanceBetweenPositionedObjects) {
    TestPositionator parent("parent");
    TestPositionator other("other");
    parent.setPositioned(true);
    other.setPositioned(true);
    other.getPosMatrix().at<float>(3, 0) = 3.0F;
    other.getPosMatrix().at<float>(3, 1) = 4.0F;
    MAOProperty property("distance", MAOPROPERTY_FLOAT, 0.0F);
    MLBActuatorDistance actuator("distance", parent, other, property);

    actuator.actuate();
    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORDISTANCE);
    EXPECT_FLOAT_EQ(property.getValue<float>(), 5.0F);
}

TEST(MLBActuatorPropertyTest, AppliesLiteralAndPropertyOperations) {
    MAO parent("parent");
    MAOProperty target("target", MAOPROPERTY_INT, 5);
    MAOValue increment(MAOPROPERTY_INT, 3);
    MLBActuatorProperty add(
        "add", parent, target, increment, MAOPROPERTY_ADD);

    add.actuate();
    EXPECT_EQ(add.getMLBType(), T_MLBACTUATORPROPERTY);
    EXPECT_EQ(target.getValue<int>(), 8);
    EXPECT_EQ(&add.getProperty(), &target);

    MAOProperty source("source", MAOPROPERTY_INT, 12);
    MLBActuatorProperty assign(
        "assign", parent, target, source, MAOPROPERTY_ASSIGN);
    assign.actuate();
    EXPECT_EQ(target.getValue<int>(), 12);
}

TEST(MLBActuatorQuitAppTest, StopsApplication) {
    MAO parent("parent");
    MLBActuatorQuitApp actuator("quit", parent);
    EndController* controller = EndController::getInstance();
    ASSERT_TRUE(controller->isRunning());

    actuator.actuate();
    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORQUITAPP);
    EXPECT_FALSE(controller->isRunning());
    EndController::destroy();
}

TEST(MLBActuatorRandomTest, WritesRandomFloatValue) {
    MAO parent("parent");
    MAOProperty property("random", MAOPROPERTY_FLOAT, -1.0F);
    MLBActuatorRandom actuator("random", parent, property);

    actuator.actuate();
    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORRANDOM);
    EXPECT_GE(property.getValue<float>(), 0.0F);

    MAOProperty invalid("invalid", MAOPROPERTY_INT, 0);
    EXPECT_THROW(MLBActuatorRandom("invalid", parent, invalid), std::string);
}

TEST(MLBActuatorRelativePoseTest, WritesRelativePoseAndUpdatesInverseFlag) {
    TestPositionator parent("parent");
    TestPositionator reference("reference");
    parent.setPositioned(true);
    reference.setPositioned(true);
    cv::Mat initial = cv::Mat::eye(4, 4, CV_32F);
    MAOProperty property("relative", MAOPROPERTY_POSE, initial);
    MLBActuatorRelativePose actuator(
        "relative", parent, reference, property, false);

    actuator.actuate();
    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORRELATIVEPOSE);
    EXPECT_FLOAT_EQ(property.getValue<cv::Mat>().at<float>(0, 0), 1.0F);
    EXPECT_FALSE(actuator.mPyGetInverse());
    actuator.mPySetInverse(true);
    EXPECT_TRUE(actuator.mPyGetInverse());
}

}  // namespace
