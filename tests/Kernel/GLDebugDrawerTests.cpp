#include <Kernel/GLDebugDrawer.h>
#include <gtest/gtest.h>

TEST(GLDebugDrawerTest, StoresDebugModeAndAcceptsNoOpCallbacks) {
    GLDebugDrawer drawer;

    EXPECT_EQ(drawer.getDebugMode(), 0);

    drawer.setDebugMode(btIDebugDraw::DBG_DrawWireframe);

    EXPECT_EQ(drawer.getDebugMode(), btIDebugDraw::DBG_DrawWireframe);
    EXPECT_NO_THROW(drawer.draw3dText(btVector3(1.0F, 2.0F, 3.0F), "text"));
    EXPECT_NO_THROW(drawer.drawContactPoint(
        btVector3(1.0F, 2.0F, 3.0F),
        btVector3(0.0F, 1.0F, 0.0F),
        2.0F,
        10,
        btVector3(1.0F, 0.0F, 0.0F)));
    EXPECT_NO_THROW(drawer.drawLine(
        btVector3(0.0F, 0.0F, 0.0F),
        btVector3(1.0F, 1.0F, 1.0F),
        btVector3(1.0F, 1.0F, 0.0F)));
}
