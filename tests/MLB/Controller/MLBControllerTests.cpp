#include <MLB/Controller/MLBControllerAND.h>
#include <MLB/Controller/MLBControllerNAND.h>
#include <MLB/Controller/MLBControllerNOR.h>
#include <MLB/Controller/MLBControllerOR.h>
#include <gtest/gtest.h>

namespace {

class TestSensor : public MLBSensor {
public:
    using MLBSensor::MLBSensor;
    void evaluate() override {}
};

class CountingActuator : public MLBActuator {
public:
    CountingActuator(const std::string& name, MAO& parent)
        : MLBActuator(name, parent) {}

    int count = 0;

private:
    void specificActuate() override { ++count; }
};

class TestController : public MLBController {
public:
    using MLBController::MLBController;
    void evaluate() override {}
};

TEST(MLBControllerTest, StoresInputsAndActivatesOutputs) {
    MAO parent("parent");
    TestController controller("controller", parent);
    TestSensor sensor("sensor", parent);
    CountingActuator actuator("actuator", parent);
    controller.addMLBSensor(sensor);
    controller.addMLBActuator(actuator);

    EXPECT_EQ(controller.getMLBType(), T_MLBCONTROLLER);
    EXPECT_FALSE(controller.getState());
    controller.activateActuators();
    EXPECT_EQ(actuator.count, 1);
    controller.cleanup();
    EXPECT_FALSE(controller.getState());
}

TEST(MLBControllerANDTest, RequiresEverySensor) {
    MAO parent("parent");
    TestSensor first("first", parent);
    TestSensor second("second", parent);
    MLBControllerAND controller("and", parent);
    controller.addMLBSensor(first);
    controller.addMLBSensor(second);

    first.trigger();
    controller.evaluate();
    EXPECT_EQ(controller.getMLBType(), T_MLBCONTROLLERAND);
    EXPECT_FALSE(controller.getState());
    second.trigger();
    controller.evaluate();
    EXPECT_TRUE(controller.getState());
}

TEST(MLBControllerNANDTest, NegatesAllSensors) {
    MAO parent("parent");
    TestSensor first("first", parent);
    TestSensor second("second", parent);
    MLBControllerNAND controller("nand", parent);
    controller.addMLBSensor(first);
    controller.addMLBSensor(second);

    first.trigger();
    controller.evaluate();
    EXPECT_EQ(controller.getMLBType(), T_MLBCONTROLLERNAND);
    EXPECT_TRUE(controller.getState());
    second.trigger();
    controller.evaluate();
    EXPECT_FALSE(controller.getState());
}

TEST(MLBControllerORTest, AcceptsAnySensor) {
    MAO parent("parent");
    TestSensor sensor("sensor", parent);
    MLBControllerOR controller("or", parent);
    controller.addMLBSensor(sensor);

    controller.evaluate();
    EXPECT_EQ(controller.getMLBType(), T_MLBCONTROLLEROR);
    EXPECT_FALSE(controller.getState());
    sensor.trigger();
    controller.evaluate();
    EXPECT_TRUE(controller.getState());
}

TEST(MLBControllerNORTest, NegatesAnySensor) {
    MAO parent("parent");
    TestSensor sensor("sensor", parent);
    MLBControllerNOR controller("nor", parent);
    controller.addMLBSensor(sensor);

    controller.evaluate();
    EXPECT_EQ(controller.getMLBType(), T_MLBCONTROLLERNOR);
    EXPECT_TRUE(controller.getState());
    sensor.trigger();
    controller.evaluate();
    EXPECT_FALSE(controller.getState());
}

}  // namespace
