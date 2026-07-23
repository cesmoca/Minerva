#include <MLB/Actuator/MLBActuatorAddDynamicObject.h>
#include <gtest/gtest.h>

namespace {

class TestPositionator : public MAOPositionator3D {
public:
    using MAOPositionator3D::MAOPositionator3D;
    cv::Mat& getPosMatrix() override { return _posMatrix; }
};

TEST(MLBActuatorAddDynamicObjectTest, StoresInstantiationParameters) {
    TestPositionator parent("parent");
    MAORenderable3DModel model(
        "template", boost::filesystem::path("template.orej"), 1.0F);
    model.setMass(2.0F);
    btVector3 impulse(1.0F, 2.0F, 3.0F);

    MLBActuatorAddDynamicObject actuator(
        "spawn", parent, model, 25, nullptr, &impulse);

    EXPECT_EQ(actuator.getMLBType(), T_MLBACTUATORADDDYNAMICOBJECT);
    EXPECT_EQ(actuator.mPyGetTimeToExpire(), 25);
    EXPECT_EQ(actuator.mPyGetImpulse(), (VectorFloat{1.0F, 2.0F, 3.0F}));
    EXPECT_EQ(actuator.mPyGetOffset().size(), 16U);

    actuator.mPySetTimeToExpire(40);
    actuator.mPySetImpulse(VectorFloat{4.0F, 5.0F, 6.0F});
    EXPECT_EQ(actuator.mPyGetTimeToExpire(), 40);
    EXPECT_EQ(actuator.mPyGetImpulse(), (VectorFloat{4.0F, 5.0F, 6.0F}));
    EXPECT_ANY_THROW(actuator.mPySetImpulse(VectorFloat{1.0F, 2.0F}));
    EXPECT_ANY_THROW(actuator.mPySetOffset(VectorFloat{1.0F}));

    EXPECT_NO_THROW(actuator.actuate());
}

TEST(MLBActuatorAddDynamicObjectTest, RejectsStaticTemplates) {
    TestPositionator parent("parent");
    MAORenderable3DModel model(
        "template", boost::filesystem::path("template.orej"), 1.0F);

    EXPECT_ANY_THROW(
        MLBActuatorAddDynamicObject("spawn", parent, model));
}

}  // namespace
