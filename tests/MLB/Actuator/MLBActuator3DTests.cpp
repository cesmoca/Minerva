#include <MLB/Actuator/MLBActuatorAnim.h>
#include <MLB/Actuator/MLBActuatorChangePose.h>
#include <MLB/Actuator/MLBActuatorPathAddPoint.h>
#include <MLB/Actuator/MLBActuatorPathRemovePoints.h>
#include <gtest/gtest.h>

namespace {

class TestRenderable3D : public MAORenderable3D {
public:
    using MAORenderable3D::MAORenderable3D;

    void generateCollisionShape(int) override {}

private:
    void _drawMAO() override {}
    void _drawMAONoTexture() override {}
};

TEST(MLBActuatorAnimTest, StoresAnimationConfigurationAndDispatchesActions) {
    MAORenderable3DModel model(
        "model", boost::filesystem::path("model.orej"));
    MLBActuatorAnim actuator("animation", model, PLAY, LOOP);

    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORANIM);
    EXPECT_EQ(actuator.mPyGetAnimAction(), PLAY);
    EXPECT_EQ(actuator.mPyGetAnimType(), LOOP);
    EXPECT_NO_THROW(actuator.actuate());

    actuator.mPySetAnimAction(PAUSE);
    actuator.mPySetAnimType(PINGPONG);
    EXPECT_EQ(actuator.mPyGetAnimAction(), PAUSE);
    EXPECT_EQ(actuator.mPyGetAnimType(), PINGPONG);
    EXPECT_NO_THROW(actuator.actuate());

    actuator.mPySetAnimAction(STOP);
    EXPECT_NO_THROW(actuator.actuate());
}

TEST(MLBActuatorChangePoseTest, AppliesTranslationToPositionedStaticObject) {
    TestRenderable3D renderable("renderable", 1.0F);
    renderable.setPositioned(true);
    MLBActuatorChangePose actuator(
        "change-pose", renderable, CHANGEPOSE_LOCAL,
        1.0F, 2.0F, 3.0F, CHANGEPOSE_LOCAL, 0.0F, 0.0F, 0.0F);

    actuator.actuate();

    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORCHANGEPOSE);
    EXPECT_FLOAT_EQ(renderable.getRelativeMatrix().at<float>(3, 0), 1.0F);
    EXPECT_FLOAT_EQ(renderable.getRelativeMatrix().at<float>(3, 1), 2.0F);
    EXPECT_FLOAT_EQ(renderable.getRelativeMatrix().at<float>(3, 2), 3.0F);
    EXPECT_EQ(actuator.mPyGetLoc(), (VectorFloat{1.0F, 2.0F, 3.0F}));
}

TEST(MLBActuatorChangePoseTest, RejectsInvalidPythonVectorSizes) {
    TestRenderable3D renderable("renderable", 1.0F);
    MLBActuatorChangePose actuator(
        "change-pose", renderable, CHANGEPOSE_LOCAL,
        0.0F, 0.0F, 0.0F, CHANGEPOSE_LOCAL, 0.0F, 0.0F, 0.0F);

    EXPECT_THROW(actuator.mPySetLoc(VectorFloat{1.0F, 2.0F}), std::string);
    EXPECT_THROW(actuator.mPySetRot(VectorFloat{1.0F, 2.0F}), std::string);
}

TEST(MLBActuatorPathTest, AddsAndRemovesPathPoints) {
    MAORenderable3DPath path("path", 1.0F, 10, 20, 30);
    MLBActuatorPathAddPoint add("add", path);
    MLBActuatorPathRemovePoints remove("remove", path);

    EXPECT_EQ(add.getMLBType(), T_MLBACTUATORPATHADDPOINT);
    EXPECT_EQ(remove.getMLBType(), T_MLBACTUATORPATHREMOVEPOINTS);
    EXPECT_NO_THROW(add.actuate());
    EXPECT_NO_THROW(remove.actuate());
}

}  // namespace
