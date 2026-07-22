#include <MLB/Actuator/MLBActuatorVisibility.h>
#include <MAO/MAORenderable3DModel.h>
#include <gtest/gtest.h>

namespace {

class TestRenderable2DImage : public MAORenderable2D {
public:
    TestRenderable2DImage(const std::string& name)
        : MAORenderable2D(name, 0, 0, 1, 1) {
        _type = T_MAORENDERABLE2DIMAGE;
    }

    void generateTexFromSDLSurface() override {}
};

TEST(MLBActuatorVisibilityTest, UpdatesTwoDimensionalVisibility) {
    TestRenderable2DImage image("image");
    MLBActuatorVisibility actuator("visibility", image, false);

    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORVISIBILITY);
    EXPECT_FALSE(actuator.mPyGetVisibility());
    actuator.actuate();
    EXPECT_FALSE(image.isVisible());

    actuator.mPySetVisibility(true);
    actuator.actuate();
    EXPECT_TRUE(image.isVisible());
}

TEST(MLBActuatorVisibilityTest, UpdatesThreeDimensionalVisibility) {
    MAORenderable3DModel model(
        "model", boost::filesystem::path("model.orej"));
    MLBActuatorVisibility actuator("visibility", model, true);

    actuator.actuate();

    EXPECT_TRUE(model.isVisible());
}

TEST(MLBActuatorVisibilityTest, RejectsUnsupportedParentType) {
    MAO parent("plain-mao");

    EXPECT_THROW(
        MLBActuatorVisibility("visibility", parent, true), std::string);
}

}  // namespace
