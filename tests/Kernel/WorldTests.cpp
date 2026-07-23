#include <Kernel/World.h>
#include <gtest/gtest.h>

TEST(WorldTest, InitializesSubsystemsAndStoresWorldState) {
    char audioDriver[] = "SDL_AUDIODRIVER=dummy";
    ASSERT_EQ(SDL_putenv(audioDriver), 0);

    World world;
    world.initWorld(320, 240);

    EXPECT_EQ(world.getWidth(), 320);
    EXPECT_EQ(world.getHeight(), 240);
    EXPECT_EQ(world.getAppName(), "Minerva's Application");
    EXPECT_EQ(world.getScreen().w, 640);
    EXPECT_EQ(world.getScreen().h, 480);

    world.setAppName("Bundle K");
    EXPECT_EQ(world.getAppName(), "Bundle K");
}
