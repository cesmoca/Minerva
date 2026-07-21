#include <MLB/Sensor/MLBSensorKeyboard.h>
#include <gtest/gtest.h>

namespace {

TEST(MLBSensorKeyboardTest, PreservesKeyboardConfiguration) {
    MAO parent("parent");
    SDL_EventType type = SDL_KEYDOWN;
    SDLKey key = SDLK_SPACE;
    MLBSensorKeyboard sensor("keyboard", parent, type, key);

    EXPECT_EQ(sensor.getMLBType(), T_MLBSENSORKEYBOARD);
    EXPECT_EQ(sensor.getType(), SDL_KEYDOWN);
    EXPECT_EQ(sensor.getKey(), SDLK_SPACE);
}

TEST(MLBSensorKeyboardTest, KeyDownEvaluationTracksPressedState) {
    MAO parent("parent");
    SDL_EventType type = SDL_KEYDOWN;
    SDLKey key = SDLK_RETURN;
    MLBSensorKeyboard sensor("keyboard", parent, type, key);

    sensor.evaluate();
    EXPECT_FALSE(sensor.getState());

    sensor.setKeyDown(true);
    sensor.evaluate();
    EXPECT_TRUE(sensor.getState());

    sensor.setKeyDown(false);
    sensor.evaluate();
    EXPECT_FALSE(sensor.getState());
}

TEST(MLBSensorKeyboardTest, KeyUpEvaluationPreservesOneShotTrigger) {
    MAO parent("parent");
    SDL_EventType type = SDL_KEYUP;
    SDLKey key = SDLK_ESCAPE;
    MLBSensorKeyboard sensor("keyboard", parent, type, key);

    sensor.trigger();
    sensor.evaluate();
    EXPECT_TRUE(sensor.getState());

    sensor.cleanup();
    sensor.evaluate();
    EXPECT_FALSE(sensor.getState());
}

}  // namespace
