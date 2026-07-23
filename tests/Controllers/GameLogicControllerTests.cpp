#include <Controllers/GameLogicController.h>
#include <gtest/gtest.h>

TEST(GameLogicControllerTest, PollsAndActivatesLinkedLogicBricks) {
    MPYWrapper::getInstance()->initPython();

    MAORenderable3DLine& parent =
        MAOFactory::getInstance()->addMAORenderable3DLine(
            "logic-parent", 1.0F, 10, 20, 30);
    MLBSensorAlways& sensor =
        MLBFactory::getInstance()->addMLBSensorAlways(
            "always", parent.getName());
    MLBControllerAND& controller =
        MLBFactory::getInstance()->addMLBControllerAND(
            "and", parent.getName());
    MLBActuatorVisibility& actuator =
        MLBFactory::getInstance()->addMLBActuatorVisibility(
            "show", parent.getName(), true);
    MLBFactory::getInstance()->addMLBLink(
        parent.getName(), sensor.getName(), controller.getName());
    MLBFactory::getInstance()->addMLBLink(
        parent.getName(), controller.getName(), actuator.getName());

    ASSERT_FALSE(parent.isVisible());
    GameLogicController logic;
    logic.pollLogic();

    EXPECT_TRUE(parent.isVisible());
    EXPECT_FALSE(sensor.getState());
    EXPECT_FALSE(controller.getState());
}
