#include <Controllers/InputEventController.h>
#include <gtest/gtest.h>

namespace {

class InputEventControllerTest : public testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO), 0) << SDL_GetError();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
        }
    }

    void TearDown() override {
        SDL_Quit();
    }

    static void pushKeyboardEvent(Uint8 type, SDLKey key) {
        SDL_Event event = {};
        event.type = type;
        event.key.type = type;
        event.key.keysym.sym = key;
        ASSERT_EQ(SDL_PushEvent(&event), 0) << SDL_GetError();
    }
};

TEST_F(InputEventControllerTest, RoutesMatchingKeyDownAndKeyUpEvents) {
    MAO parent("parent");
    SDL_EventType downType = SDL_KEYDOWN;
    SDL_EventType upType = SDL_KEYUP;
    SDLKey key = SDLK_a;
    MLBSensorKeyboard downSensor("down", parent, downType, key);
    MLBSensorKeyboard upSensor("up", parent, upType, key);
    InputEventController controller;
    controller.addMLBSensorKeyboard(downSensor);
    controller.addMLBSensorKeyboard(upSensor);

    pushKeyboardEvent(SDL_KEYDOWN, key);
    controller.pollEvents();
    downSensor.evaluate();
    EXPECT_TRUE(downSensor.getState());
    EXPECT_FALSE(upSensor.getState());

    pushKeyboardEvent(SDL_KEYUP, key);
    controller.pollEvents();
    downSensor.evaluate();
    EXPECT_FALSE(downSensor.getState());
    EXPECT_TRUE(upSensor.getState());
}

TEST_F(InputEventControllerTest, IgnoresUnmatchedKeyboardEvents) {
    MAO parent("parent");
    SDL_EventType type = SDL_KEYDOWN;
    SDLKey key = SDLK_a;
    MLBSensorKeyboard sensor("sensor", parent, type, key);
    InputEventController controller;
    controller.addMLBSensorKeyboard(sensor);

    pushKeyboardEvent(SDL_KEYDOWN, SDLK_b);
    controller.pollEvents();
    sensor.evaluate();

    EXPECT_FALSE(sensor.getState());
}

}  // namespace
