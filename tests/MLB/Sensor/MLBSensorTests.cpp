#include <MLB/Sensor/MLBSensorActuator.h>
#include <MLB/Sensor/MLBSensorAlways.h>
#include <MLB/Sensor/MLBSensorDelay.h>
#include <MLB/Sensor/MLBSensorProperty.h>
#include <MLB/Sensor/MLBSensorRandom.h>
#include <gtest/gtest.h>

namespace {

class TestSensor : public MLBSensor {
public:
    using MLBSensor::MLBSensor;
    void evaluate() override {}
};

TEST(MLBSensorTest, ManagesStateAndActivity) {
    MAO parent("parent");
    TestSensor sensor("sensor", parent);

    EXPECT_EQ(sensor.getMLBType(), T_MLBSENSOR);
    EXPECT_FALSE(sensor.getState());
    EXPECT_TRUE(sensor.isActive());

    sensor.trigger();
    EXPECT_TRUE(sensor.getState());
    sensor.cleanup();
    EXPECT_FALSE(sensor.getState());
    sensor.setActive(false);
    EXPECT_FALSE(sensor.isActive());
}

TEST(MLBSensorActuatorTest, PreservesTriggeredStateDuringEvaluation) {
    MAO parent("parent");
    MLBSensorActuator sensor("sensor", parent);
    EXPECT_EQ(sensor.getMLBType(), T_MLBSENSORACTUATOR);

    sensor.trigger();
    sensor.evaluate();
    EXPECT_TRUE(sensor.getState());
}

TEST(MLBSensorAlwaysTest, EvaluatesTrue) {
    MAO parent("parent");
    MLBSensorAlways sensor("always", parent);

    EXPECT_EQ(sensor.getMLBType(), T_MLBSENSORALWAYS);
    sensor.evaluate();
    EXPECT_TRUE(sensor.getState());
}

TEST(MLBSensorDelayTest, TriggersAtConfiguredFrameInterval) {
    MAO parent("parent");
    MLBSensorDelay sensor("delay", parent, 2);

    EXPECT_EQ(sensor.getMLBType(), T_MLBSENSORDELAY);
    EXPECT_EQ(sensor.mPyGetDelayFrames(), 2);
    sensor.evaluate();
    EXPECT_FALSE(sensor.getState());
    sensor.evaluate();
    EXPECT_TRUE(sensor.getState());

    sensor.mPySetDelayFrames(3);
    EXPECT_EQ(sensor.mPyGetDelayFrames(), 3);
}

TEST(MLBSensorPropertyTest, EvaluatesEqualityIntervalAndPropertyComparison) {
    MAO parent("parent");
    MAOProperty property("value", MAOPROPERTY_INT, 5);
    MAOValue lower(MAOPROPERTY_INT, 3);
    MAOValue upper(MAOPROPERTY_INT, 7);
    MLBSensorProperty interval(
        "interval", parent, MAOPROPERTY_INTERVAL, property, lower, upper);
    interval.evaluate();
    EXPECT_EQ(interval.getMLBType(), T_MLBSENSORPROPERTY);
    EXPECT_TRUE(interval.getState());

    MAOProperty comparison("comparison", MAOPROPERTY_INT, 5);
    MLBSensorProperty equal(
        "equal", parent, MAOPROPERTY_EQUAL, property, comparison);
    equal.evaluate();
    EXPECT_TRUE(equal.getState());

    comparison.setValue<int>(6);
    equal.evaluate();
    EXPECT_FALSE(equal.getState());
}

TEST(MLBSensorPropertyTest, RejectsIntervalForBooleanProperty) {
    MAO parent("parent");
    MAOProperty property("value", MAOPROPERTY_BOOLEAN, true);
    MAOValue lower(MAOPROPERTY_BOOLEAN, false);
    MAOValue upper(MAOPROPERTY_BOOLEAN, true);

    EXPECT_THROW(
        MLBSensorProperty(
            "interval", parent, MAOPROPERTY_INTERVAL, property, lower, upper),
        std::string);
}

TEST(MLBSensorRandomTest, ValidatesInitialProbability) {
    MAO parent("parent");
    MLBSensorRandom sensor("random", parent, 0.25F);

    EXPECT_EQ(sensor.getMLBType(), T_MLBSENSORRANDOM);
    EXPECT_FLOAT_EQ(sensor.mPyGetProbability(), 0.25F);
    sensor.mPySetProbability(0.75F);
    EXPECT_FLOAT_EQ(sensor.mPyGetProbability(), 0.75F);

    EXPECT_THROW(MLBSensorRandom("low", parent, -0.1F), std::string);
    EXPECT_THROW(MLBSensorRandom("high", parent, 1.1F), std::string);
}

}  // namespace
