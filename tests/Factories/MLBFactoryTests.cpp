#include <Factories/MLBFactory.h>
#include <gtest/gtest.h>

TEST(MLBFactoryTest, CreatesTypedBricksAndLinksTheLogicChain) {
    MAORenderable3DLine& parent =
        MAOFactory::getInstance()->addMAORenderable3DLine(
            "factory-parent", 1.0F, 10, 20, 30);
    MLBFactory factory;

    MLBSensorAlways& sensor =
        factory.addMLBSensorAlways("always", parent.getName());
    MLBControllerAND& controller =
        factory.addMLBControllerAND("and", parent.getName());
    MLBActuatorVisibility& actuator =
        factory.addMLBActuatorVisibility("show", parent.getName(), true);
    factory.addMLBLink(parent.getName(), sensor.getName(), controller.getName());
    factory.addMLBLink(
        parent.getName(), controller.getName(), actuator.getName());

    sensor.evaluate();
    controller.evaluate();
    controller.activateActuators();

    EXPECT_EQ(&factory.getMLBSensor(parent.getName(), "always"), &sensor);
    EXPECT_EQ(&factory.getMLBController(parent.getName(), "and"), &controller);
    EXPECT_EQ(&factory.getMLBActuator(parent.getName(), "show"), &actuator);
    EXPECT_TRUE(parent.isVisible());
    EXPECT_EQ(factory.getVectorMLBSensor().size(), 1U);
    EXPECT_EQ(factory.getVectorMLBController().size(), 1U);
    EXPECT_EQ(factory.getVectorMLBActuator().size(), 1U);
    EXPECT_ANY_THROW(
        factory.addMLBSensorAlways("always", parent.getName()));
}
