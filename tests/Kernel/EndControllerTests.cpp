#include <Kernel/EndController.h>
#include <gtest/gtest.h>

TEST(EndControllerTest, StopsTheApplication) {
    EndController* controller = EndController::getInstance();
    EXPECT_TRUE(controller->isRunning());
    controller->end();
    EXPECT_FALSE(controller->isRunning());
    EndController::destroy();
}
